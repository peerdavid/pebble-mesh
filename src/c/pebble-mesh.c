#include <pebble.h>
#include "config.h"
#include "weather.h"
#include "steps.h"
#include "battery.h"



// A pointer to the main window and layers
static Window *s_main_window;
static Layer *s_time_layer;
static Layer *s_date_layer;
static Layer *s_frame_layer;
static Layer *s_animation_layer;

// Pointer for the animation AppTimer
#define GRID_SIZE 8
#define CROSS_SIZE 0

#define VERY_FIRST_ANIMATION_FRAME 500
#define DECREASE_PER_FRAME 10
#define ANIMATION_RATE_MS 20

#define NUM_ANIMATION_FRAMES 500

#define BORDER_THICKNESS 3

// Animation
static AppTimer *s_animation_timer = NULL;
static int current_animation_frame = 0; // Ranges from NUM_ANIMATION_FRAMES down to 0

// Buffer to hold the time string (e.g., "12:34" or "23:59")
static char s_time_buffer[9];
static char s_date_buffer[8];

// Forward declarations
static void update_time();
static void battery_handler(BatteryChargeState state);
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
static void animation_timer_callback(void *data);
static void try_start_animation_timer();
static void try_stop_animation_timer();
static void draw_frame(Layer *layer, GContext *ctx);
static void draw_animation(Layer *layer, GContext *ctx);
static void draw_time(Layer *layer, GContext *ctx);
static void draw_date(Layer *layer, GContext *ctx);
static void inbox_received_callback(DictionaryIterator *iterator, void *context);
static void delayed_weather_request(void *data);
static void init_info_layers(GRect bounds);
static void update_all_info_layers();
static void draw_info_for_type(InfoType info_type, InfoLayer* info_layer);
static void clear_info_layer(InfoLayer* info_layer);


// Function to update all colors based on current theme
static void update_colors() {
  window_set_background_color(s_main_window, get_background_color());

  layer_mark_dirty(s_time_layer);
  layer_mark_dirty(s_date_layer);

  // Recreate bitmaps for new theme
  if (s_weather_icon_bitmap) {
    gbitmap_destroy(s_weather_icon_bitmap);
  }
  uint32_t weather_resource_id = get_weather_image_resource(s_current_weather_code);
  s_weather_icon_bitmap = gbitmap_create_with_resource(weather_resource_id);
  
  load_step_icon();

  if (s_battery_icon_bitmap) {
    gbitmap_destroy(s_battery_icon_bitmap);
  }
  uint32_t battery_resource_id = is_light_theme() ? RESOURCE_ID_IMAGE_BATTERY_LIGHT : RESOURCE_ID_IMAGE_BATTERY_DARK;
  s_battery_icon_bitmap = gbitmap_create_with_resource(battery_resource_id);

  // Update all info layers to refresh the display
  update_all_info_layers();

  // Force redraw
  layer_mark_dirty(s_frame_layer);
  layer_mark_dirty(s_animation_layer);
}

// --- Weather Functions ---
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Message received");
  
  bool weather_data_updated = false;

  // Read temperature unit first to know what symbol to use
  Tuple *temp_unit_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE_UNIT);
  if (temp_unit_tuple) {
    int new_unit = (int)temp_unit_tuple->value->int32;
    if (new_unit != s_temperature_unit) {
      s_temperature_unit = new_unit;
      save_temperature_unit_to_storage();
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Temperature unit changed to: %d", s_temperature_unit);
      weather_data_updated = true;
    }
  }

  // Read temperature
  Tuple *temperature_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_TEMPERATURE);
  if (temperature_tuple) {
    const char* unit_symbol = s_temperature_unit == 1 ? "°F" : "°C";
    snprintf(s_temperature_buffer, sizeof(s_temperature_buffer), "%s%s", temperature_tuple->value->cstring, unit_symbol);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Temperature: %s", s_temperature_buffer);
    weather_data_updated = true;
  }

  // Read location
  Tuple *location_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_LOCATION);
  if (location_tuple) {
    snprintf(s_location_buffer, sizeof(s_location_buffer), "%s", location_tuple->value->cstring);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Location: %s", s_location_buffer);
    weather_data_updated = true;
  }

  // Read weather condition and update icon
  Tuple *condition_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_CONDITION);
  if (condition_tuple) {
    s_current_weather_code = (int)condition_tuple->value->int32;
    // Recreate weather bitmap with new weather code
    if (s_weather_icon_bitmap) {
      gbitmap_destroy(s_weather_icon_bitmap);
    }
    uint32_t resource_id = get_weather_image_resource(s_current_weather_code);
    s_weather_icon_bitmap = gbitmap_create_with_resource(resource_id);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Weather condition: %d", s_current_weather_code);
    weather_data_updated = true;
  }

  // Read is_day
  Tuple *is_day_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_IS_DAY);
  if (is_day_tuple) {
    s_is_day = (int)is_day_tuple->value->int32;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Is day: %d", s_is_day);
    weather_data_updated = true;
  }

  // Read color theme
  Tuple *theme_tuple = dict_find(iterator, MESSAGE_KEY_COLOR_THEME);
  if (theme_tuple) {
    int new_theme = (int)theme_tuple->value->int32;
    if (new_theme != s_color_theme) {
      s_color_theme = new_theme;
      save_theme_to_storage(); // Save the new theme preference
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Color theme changed to: %d", s_color_theme);
      update_colors();
    }
  }

  // Read step goal
  Tuple *step_goal_tuple = dict_find(iterator, MESSAGE_KEY_STEP_GOAL);
  if (step_goal_tuple) {
    int new_step_goal = (int)step_goal_tuple->value->int32;
    if (new_step_goal > 0 && new_step_goal != s_step_goal) {
      s_step_goal = new_step_goal;
      save_step_goal_to_storage(); // Save the new step goal preference
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Step goal changed to: %d", s_step_goal);
      // Redraw steps info layer to update the progress bar
      update_all_info_layers();
    }
  }
  
  // Save weather data to persistent storage if any was updated
  if (weather_data_updated) {
    save_weather_to_storage();
    // Redraw all info layers to show updated weather data
    update_all_info_layers();
  }

  // Read enable animations
  Tuple *enable_animations_tuple = dict_find(iterator, MESSAGE_KEY_ENABLE_ANIMATIONS);
  if (enable_animations_tuple) {
    int new_enable_animations = (int)enable_animations_tuple->value->int32;
    if (new_enable_animations != s_enable_animations) {
      s_enable_animations = new_enable_animations;
      save_enable_animations_to_storage();
      try_start_animation_timer();
    }
  }
}

// --- Update Time Function ---

static void update_time() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Update time");
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Use strftime to get the time in the user's 12h or 24h format
  if (clock_is_24h_style()) {
    strftime(s_time_buffer, sizeof(s_time_buffer), "%H:%M", tick_time);
  } else {
    strftime(s_time_buffer, sizeof(s_time_buffer), "%I:%M", tick_time);
  }

  layer_mark_dirty(s_time_layer);

  strftime(s_date_buffer, sizeof(s_date_buffer), " %a %d", tick_time);
  layer_mark_dirty(s_date_layer);

  update_step_count();
  
  // Update info layers and colors in case the theme changed (dynamic theme)
  update_colors();
}

// --- Battery Handler ---
static void battery_handler(BatteryChargeState state) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Battery handler");
  battery_level = state.charge_percent;
  snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%%", battery_level);
  
  // Update info layers to reflect new battery level
  update_all_info_layers();
}

/**
 * @brief Stops the animation timer and resets the running flag.
 */
static void try_stop_animation_timer() {
  if (s_animation_timer) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Stop animation timer");
    app_timer_cancel(s_animation_timer);
    s_animation_timer = NULL;
  }

  current_animation_frame = 0;
}

/**
 * @brief Starts the animation timer if it's not already running.
 */
static void try_start_animation_timer() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Start animation timer");

  if(current_animation_frame == 0){
    current_animation_frame = s_enable_animations == 0 ? 0 : NUM_ANIMATION_FRAMES;
  }

  if(current_animation_frame > 0){
    s_animation_timer = app_timer_register(VERY_FIRST_ANIMATION_FRAME, animation_timer_callback, NULL);
  } else {
    layer_mark_dirty(s_animation_layer);
  }
}

/**
 * @brief AppTimer callback that triggers the frame redraw and reschedules itself.
 */
static void animation_timer_callback(void *data) {
  layer_mark_dirty(s_animation_layer);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Current animation frame: %d", current_animation_frame);

  // Reschedule the timer for the next frame, creating a continuous loop
  if(current_animation_frame > 0){
    s_animation_timer = app_timer_register(ANIMATION_RATE_MS, animation_timer_callback, NULL);
  }
}


// --- Frame Layer Drawing Update Procedure (Modified for Line Fly-In) ---
static void draw_frame(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GColor frame_color = get_text_color(); // Use theme-appropriate color

  // In case we have light theme, we don't draw a frame
  if(BORDER_THICKNESS > 0 && is_dark_theme()) {
    graphics_context_set_stroke_color(ctx, frame_color);
    graphics_context_set_stroke_width(ctx, BORDER_THICKNESS);
    graphics_draw_rect(ctx, GRect(1, 1, bounds.size.w - 2, bounds.size.h - 2));
  }

  // IN case we have light theme, we draw a gray rectangle in the middle
  if(is_light_theme()) {
    graphics_context_set_fill_color(ctx, GColorLightGray);
    
    // We draw the rectange exactly from the upper to the lower animated line in  the same length
    const int max_line_length = bounds.size.w * 0.8;
    const int line_x_start_full = (bounds.size.w - max_line_length) / 2;
    const int time_y = bounds.size.h / 2;
    const int line_y_offset = 30;
    const int height = (line_y_offset + 2) * 2;
    
    graphics_fill_rect(ctx, GRect(line_x_start_full, time_y - line_y_offset + 4, max_line_length, height-11), 0, GCornerNone);
  }

  // Dots
  graphics_context_set_stroke_color(ctx, frame_color);
  graphics_context_set_stroke_width(ctx, 1);
  for (int y = 0; y < bounds.size.h; y += GRID_SIZE) {
    for (int x = 0; x < bounds.size.w; x += GRID_SIZE) {
      // 1. Draw the horizontal part of the cross
      // Starts at x - CROSS_SIZE and ends at x + CROSS_SIZE, centered on y
      graphics_draw_line(
          ctx,
          GPoint(x - CROSS_SIZE, y),
          GPoint(x + CROSS_SIZE, y));

      // 2. Draw the vertical part of the cross
      // Starts at y - CROSS_SIZE and ends at y + CROSS_SIZE, centered on x
      graphics_draw_line(
          ctx,
          GPoint(x, y - CROSS_SIZE),
          GPoint(x, y + CROSS_SIZE));
    }
  }
}

static void draw_animation(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GColor color = get_text_color();

  const int max_line_length = bounds.size.w * 0.8;
  const int line_x_start_full = (bounds.size.w - max_line_length) / 2;
  const int line_x_end_full = line_x_start_full + max_line_length;
  const int time_y = bounds.size.h / 2;
  const int line_y_offset = 30;
  const int segment_length = 12; // Length of each typed segment
  const int segment_gap = 2; // Gap between segments

  current_animation_frame -= DECREASE_PER_FRAME;
  current_animation_frame = current_animation_frame < 0 ? 0 : current_animation_frame;

  float animation_factor = 1.0f - ((float)current_animation_frame / NUM_ANIMATION_FRAMES);
  
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_stroke_width(ctx, BORDER_THICKNESS);
  
  // Calculate total number of segments needed for the full line
  int total_segments = (max_line_length + segment_length + segment_gap - 1) / (segment_length + segment_gap);
  
  // Typewriter effect: draw lines in discrete segments
  // First half of animation: upper line, second half: lower line
  if (animation_factor < 0.5f) {
    // Upper line typing
    float upper_progress = animation_factor * 2.0f; // 0.0 to 1.0
    int segments_to_draw = (int)(total_segments * upper_progress);
    
    // Draw complete segments only
    for (int i = 0; i < segments_to_draw; i++) {
      int segment_start = i * (segment_length + segment_gap);
      int segment_end = segment_start + segment_length;
      
      // Don't exceed line length
      if (segment_start >= max_line_length) break;
      if (segment_end > max_line_length) segment_end = max_line_length;
      
      graphics_draw_line(ctx,
          GPoint(line_x_start_full + segment_start, time_y - line_y_offset),
          GPoint(line_x_start_full + segment_end, time_y - line_y_offset));
    }
    
    // Draw blinking cursor at end of last segment
    if (segments_to_draw < total_segments) {
      int cursor_x = segments_to_draw * (segment_length + segment_gap);
      if (cursor_x < max_line_length && (current_animation_frame % 6) < 3) {
        graphics_context_set_stroke_width(ctx, 2);
        graphics_draw_line(ctx,
            GPoint(line_x_start_full + cursor_x, time_y - line_y_offset - 3),
            GPoint(line_x_start_full + cursor_x, time_y - line_y_offset + 3));
        graphics_context_set_stroke_width(ctx, BORDER_THICKNESS);
      }
    }
  } else {
    // Upper line complete - draw it fully
    graphics_draw_line(ctx,
        GPoint(line_x_start_full, time_y - line_y_offset),
        GPoint(line_x_end_full, time_y - line_y_offset));
    
    // Lower line typing
    float lower_progress = (animation_factor - 0.5f) * 2.0f; // 0.0 to 1.0
    
    if (lower_progress >= 0.99f) {
      // Lower line complete - draw it fully as a solid line
      graphics_draw_line(ctx,
          GPoint(line_x_start_full, time_y + line_y_offset),
          GPoint(line_x_end_full, time_y + line_y_offset));
    } else {
      // Lower line still typing - draw in segments
      int segments_to_draw = (int)(total_segments * lower_progress);
      
      // Draw complete segments only
      for (int i = 0; i < segments_to_draw; i++) {
        int segment_start = i * (segment_length + segment_gap);
        int segment_end = segment_start + segment_length;
        
        // Don't exceed line length
        if (segment_start >= max_line_length) break;
        if (segment_end > max_line_length) segment_end = max_line_length;
        
        graphics_draw_line(ctx,
            GPoint(line_x_start_full + segment_start, time_y + line_y_offset),
            GPoint(line_x_start_full + segment_end, time_y + line_y_offset));
      }
      
      // Draw blinking cursor at end of last segment
      if (segments_to_draw < total_segments) {
        int cursor_x = segments_to_draw * (segment_length + segment_gap);
        if (cursor_x < max_line_length && (current_animation_frame % 6) < 3) {
          graphics_context_set_stroke_width(ctx, 2);
          graphics_draw_line(ctx,
              GPoint(line_x_start_full + cursor_x, time_y + line_y_offset - 3),
              GPoint(line_x_start_full + cursor_x, time_y + line_y_offset + 3));
          graphics_context_set_stroke_width(ctx, BORDER_THICKNESS);
        }
      }
    }
  }
}


// --- Draw Time with Outline ---
static void draw_time(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GFont font = fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS);
  
  // In light mode, draw white outline around black text
  if (is_light_theme()) {
    // Draw white outline by drawing the text 4 times with 1-pixel offsets
    graphics_context_set_text_color(ctx, GColorWhite);
    
    // Left
    graphics_draw_text(ctx, s_time_buffer, font,
                      GRect(bounds.origin.x - 1, bounds.origin.y, bounds.size.w, bounds.size.h),
                      GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    // Right
    graphics_draw_text(ctx, s_time_buffer, font,
                      GRect(bounds.origin.x + 1, bounds.origin.y, bounds.size.w, bounds.size.h),
                      GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    // Up
    graphics_draw_text(ctx, s_time_buffer, font,
                      GRect(bounds.origin.x, bounds.origin.y - 1, bounds.size.w, bounds.size.h),
                      GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    // Down
    graphics_draw_text(ctx, s_time_buffer, font,
                      GRect(bounds.origin.x, bounds.origin.y + 1, bounds.size.w, bounds.size.h),
                      GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    
    // Draw black text in center
    graphics_context_set_text_color(ctx, GColorBlack);
    graphics_draw_text(ctx, s_time_buffer, font,
                      bounds,
                      GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  } else {
    // Dark mode: just draw white text without outline
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx, s_time_buffer, font,
                      bounds,
                      GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  }
}

// --- Draw Date with Outline ---
static void draw_date(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  
  // In light mode, draw white outline around black text
  if (is_light_theme()) {
    // Draw white outline by drawing the text 4 times with 1-pixel offsets
    graphics_context_set_text_color(ctx, GColorWhite);
    
    // Left
    graphics_draw_text(ctx, s_date_buffer, font,
                      GRect(bounds.origin.x - 1, bounds.origin.y, bounds.size.w, bounds.size.h),
                      GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    // Right
    graphics_draw_text(ctx, s_date_buffer, font,
                      GRect(bounds.origin.x + 1, bounds.origin.y, bounds.size.w, bounds.size.h),
                      GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    // Up
    graphics_draw_text(ctx, s_date_buffer, font,
                      GRect(bounds.origin.x, bounds.origin.y - 1, bounds.size.w, bounds.size.h),
                      GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    // Down
    graphics_draw_text(ctx, s_date_buffer, font,
                      GRect(bounds.origin.x, bounds.origin.y + 1, bounds.size.w, bounds.size.h),
                      GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    
    // Draw black text in center
    graphics_context_set_text_color(ctx, GColorBlack);
    graphics_draw_text(ctx, s_date_buffer, font,
                      bounds,
                      GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  } else {
    // Dark mode: just draw white text without outline
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx, s_date_buffer, font,
                      bounds,
                      GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  }
}


// --- Tick Handler ---
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  try_start_animation_timer();
  update_time();
}


// Its simply a box with a border width 3 in the text color
static void draw_colored_box_info(InfoLayer* info_layer) {
  Layer* layer = info_layer->layer;
  // Create a filled rectangle layer
  info_layer->bitmap_layer_1 = bitmap_layer_create(GRect(1, 1, info_layer->bounds.size.w-2, info_layer->bounds.size.h-2));
  bitmap_layer_set_background_color(info_layer->bitmap_layer_1, get_text_color());
  // Add to the info layer
  layer_add_child(layer, bitmap_layer_get_layer(info_layer->bitmap_layer_1));
}

// Generic function to draw info based on type
static void draw_info_for_type(InfoType info_type, InfoLayer* info_layer) {
  switch (info_type) {
    case INFO_TYPE_WEATHER:
      draw_weather_info(info_layer);
      break;
    case INFO_TYPE_TEMPERATURE:
      draw_temperature_info(info_layer);
      break;
    case INFO_TYPE_STEPS:
      draw_steps_info(info_layer);
      break;
    case INFO_TYPE_BATTERY:
      draw_battery_info(info_layer);
      break;
    case INFO_TYPE_COLORED_BOX:
      draw_colored_box_info(info_layer);
    case INFO_TYPE_NONE:
      // Do nothing for empty layers
      break;
  }
}

// Clear all child layers from an info layer
static void clear_info_layer(InfoLayer* info_layer) {
  if (info_layer->text_layer1) {
    text_layer_destroy(info_layer->text_layer1);
    info_layer->text_layer1 = NULL;
  }
  if (info_layer->text_layer2) {
    text_layer_destroy(info_layer->text_layer2);
    info_layer->text_layer2 = NULL;
  }
  if (info_layer->bitmap_layer_1) {
    bitmap_layer_destroy(info_layer->bitmap_layer_1);
    info_layer->bitmap_layer_1 = NULL;
  }
  if (info_layer->bitmap_layer_2) {
    bitmap_layer_destroy(info_layer->bitmap_layer_2);
    info_layer->bitmap_layer_2 = NULL;
  }
  if (info_layer->bitmap_layer_3) {
    bitmap_layer_destroy(info_layer->bitmap_layer_3);
    info_layer->bitmap_layer_3 = NULL;
  }
}

// Update all info layers according to current assignments
static void update_all_info_layers() {
  for (int i = 0; i < NUM_INFO_LAYERS; i++) {
    // Clear existing content first
    clear_info_layer(&s_info_layers[i]);
    // Redraw with current assignment
    draw_info_for_type(s_layer_assignments[i], &s_info_layers[i]);
  }
}

// Timer callback to request weather after UI is loaded
static void delayed_weather_request(void *data) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Requesting weather update (delayed)");
  request_weather_update();
}

// Initialize the 4 info layers with proper positioning
static void init_info_layers(GRect bounds) {

  int theme_offset = is_dark_theme() ? 2 : 0;
  const int info_layer_width = bounds.size.w / 2 - BORDER_THICKNESS - 2 - theme_offset;
  const int info_layer_height = 44;

  int margin_w = 6 + theme_offset;
  const int margin_h = 4;

  // Upper left
  s_info_layers[LAYER_UPPER_LEFT].bounds = GRect(margin_w, margin_h, info_layer_width, info_layer_height);
  s_info_layers[LAYER_UPPER_LEFT].position = LAYER_UPPER_LEFT;
  s_info_layers[LAYER_UPPER_LEFT].layer = layer_create(s_info_layers[LAYER_UPPER_LEFT].bounds);
  s_info_layers[LAYER_UPPER_LEFT].text_layer1 = NULL;
  s_info_layers[LAYER_UPPER_LEFT].text_layer2 = NULL;
  s_info_layers[LAYER_UPPER_LEFT].bitmap_layer_1 = NULL;
  s_info_layers[LAYER_UPPER_LEFT].bitmap_layer_2 = NULL;
  s_info_layers[LAYER_UPPER_LEFT].bitmap_layer_3 = NULL;
  
  // Upper right 
  s_info_layers[LAYER_UPPER_RIGHT].bounds = GRect(bounds.size.w - info_layer_width - margin_w, margin_h, info_layer_width, info_layer_height);
  s_info_layers[LAYER_UPPER_RIGHT].position = LAYER_UPPER_RIGHT;
  s_info_layers[LAYER_UPPER_RIGHT].layer = layer_create(s_info_layers[LAYER_UPPER_RIGHT].bounds);
  s_info_layers[LAYER_UPPER_RIGHT].text_layer1 = NULL;
  s_info_layers[LAYER_UPPER_RIGHT].text_layer2 = NULL;
  s_info_layers[LAYER_UPPER_RIGHT].bitmap_layer_1 = NULL;
  s_info_layers[LAYER_UPPER_RIGHT].bitmap_layer_2 = NULL;
  s_info_layers[LAYER_UPPER_RIGHT].bitmap_layer_3 = NULL;
  
  // Lower left
  s_info_layers[LAYER_LOWER_LEFT].bounds = GRect(margin_w, bounds.size.h - info_layer_height - margin_h, info_layer_width, info_layer_height);
  s_info_layers[LAYER_LOWER_LEFT].position = LAYER_LOWER_LEFT;
  s_info_layers[LAYER_LOWER_LEFT].layer = layer_create(s_info_layers[LAYER_LOWER_LEFT].bounds);
  s_info_layers[LAYER_LOWER_LEFT].text_layer1 = NULL;
  s_info_layers[LAYER_LOWER_LEFT].text_layer2 = NULL;
  s_info_layers[LAYER_LOWER_LEFT].bitmap_layer_1 = NULL;
  s_info_layers[LAYER_LOWER_LEFT].bitmap_layer_2 = NULL;
  s_info_layers[LAYER_LOWER_LEFT].bitmap_layer_3 = NULL;
  
  // Lower right
  s_info_layers[LAYER_LOWER_RIGHT].bounds = GRect(bounds.size.w - info_layer_width - margin_w, bounds.size.h - info_layer_height - margin_h, info_layer_width, info_layer_height);
  s_info_layers[LAYER_LOWER_RIGHT].position = LAYER_LOWER_RIGHT;
  s_info_layers[LAYER_LOWER_RIGHT].layer = layer_create(s_info_layers[LAYER_LOWER_RIGHT].bounds);
  s_info_layers[LAYER_LOWER_RIGHT].text_layer1 = NULL;
  s_info_layers[LAYER_LOWER_RIGHT].text_layer2 = NULL;
  s_info_layers[LAYER_LOWER_RIGHT].bitmap_layer_1 = NULL;
  s_info_layers[LAYER_LOWER_RIGHT].bitmap_layer_2 = NULL;
  s_info_layers[LAYER_LOWER_RIGHT].bitmap_layer_3 = NULL;
}


// --- Window Load/Unload Handlers ---
static void main_window_appear(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Window appear");

  // Start animation and update UI immediately
  try_start_animation_timer();
  battery_handler(battery_state_service_peek());
  update_time(); // Ensure time is displayed immediately

  // Request weather update after a short delay to prevent blocking UI
  app_timer_register(100, delayed_weather_request, NULL);
}

static void main_window_load(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Window load");

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create frame layer
  s_frame_layer = layer_create(bounds);
  layer_set_update_proc(s_frame_layer, draw_frame);
  layer_add_child(window_layer, s_frame_layer);

  // Create animation layer
  s_animation_layer = layer_create(bounds);
  layer_set_update_proc(s_animation_layer, draw_animation);
  layer_add_child(window_layer, s_animation_layer);

  // Create Time Layer
  const int time_y_pos = bounds.size.h / 2 - 20 - 14;
  s_time_layer = layer_create(
      GRect(0, time_y_pos, bounds.size.w, bounds.size.h));
  layer_set_update_proc(s_time_layer, draw_time);
  layer_add_child(window_layer, s_time_layer);

  // Create the Date Layer (Center below time)
  s_date_layer = layer_create(
      GRect(0, time_y_pos + 38, bounds.size.w, 24));
  layer_set_update_proc(s_date_layer, draw_date);
  layer_add_child(window_layer, s_date_layer);

  // Initialize the 4 info layers
  init_info_layers(bounds);
  
  // Add the info layers to the window
  for (int i = 0; i < NUM_INFO_LAYERS; i++) {
    layer_add_child(window_layer, s_info_layers[i].layer);
  }

  // Initialize bitmaps for use in info drawing functions
  uint32_t default_icon = get_weather_image_resource(s_current_weather_code);
  s_weather_icon_bitmap = gbitmap_create_with_resource(default_icon);
  
  uint32_t step_icon = is_light_theme() ? RESOURCE_ID_IMAGE_STEP_LIGHT : RESOURCE_ID_IMAGE_STEP_DARK;
  s_step_icon_bitmap = gbitmap_create_with_resource(step_icon);

  uint32_t battery_icon = is_light_theme() ? RESOURCE_ID_IMAGE_BATTERY_LIGHT : RESOURCE_ID_IMAGE_BATTERY_DARK;
  s_battery_icon_bitmap = gbitmap_create_with_resource(battery_icon);

  // Initialize the display with current layer assignments
  update_all_info_layers();

  // Make sure the initial time is displayed
  update_time();
}

static void main_window_unload(Window *window) {
  // Destroy the main layers
  layer_destroy(s_time_layer);
  layer_destroy(s_date_layer);
  layer_destroy(s_frame_layer);
  layer_destroy(s_animation_layer);

  // Destroy info layers (this will also destroy all their children via clear_info_layer)
  for (int i = 0; i < NUM_INFO_LAYERS; i++) {
    if (s_info_layers[i].layer) {
      clear_info_layer(&s_info_layers[i]);
      layer_destroy(s_info_layers[i].layer);
    }
  }

  // Destroy bitmaps
  if (s_step_icon_bitmap) {
    gbitmap_destroy(s_step_icon_bitmap);
  }
  if (s_weather_icon_bitmap) {
    gbitmap_destroy(s_weather_icon_bitmap);
  }
  if (s_battery_icon_bitmap) {
    gbitmap_destroy(s_battery_icon_bitmap);
  }
}

// --- Initialization and Deinitialization ---

static void init() {
  srand(time(NULL));

  // Load saved theme preference before creating UI
  load_theme_from_storage();
  
  // Load saved step goal preference
  load_step_goal_from_storage();
  
  // Load saved temperature unit preference
  load_temperature_unit_from_storage();

  // Load animation
  load_enable_animations_from_storage();
  
  // Load saved weather data from storage
  load_weather_from_storage();

  s_main_window = window_create();
  window_set_background_color(s_main_window, get_background_color());

  window_set_window_handlers(s_main_window, (WindowHandlers){
                                                .load = main_window_load,
                                                .unload = main_window_unload,
                                                .appear = main_window_appear});

  // Initialize App Message
  app_message_register_inbox_received(inbox_received_callback);
  app_message_open(128, 128); // Increase buffer size for weather data

  // Subscribe to MINUTE_UNIT (for time update/minute-start trigger) and SECOND_UNIT (for stop trigger)
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Subscribe to battery state updates
  battery_state_service_subscribe(battery_handler);

  window_stack_push(s_main_window, true);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Finished init");
}

static void deinit() {
  try_stop_animation_timer();
  window_destroy(s_main_window);
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
}

// --- Main Program Loop ---

int main(void)
{
  init();
  app_event_loop();
  deinit();
}