#include <pebble.h>
#include "config.h"
#include "weather.h"
#include "steps.h"
#include "battery.h"



// A pointer to the main window and layers
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static Layer *s_frame_layer;
static Layer *s_animation_layer;

// Pointer for the animation AppTimer
#define GRID_SIZE 8
#define CROSS_SIZE 0

#define VERY_FIRST_ANIMATION_FRAME 500
#define ANIMATION_RATE_MS 20

#define NUM_ANIMATION_FRAMES 500
#define ANIMATION_DECREASE_STEP 15
#define SLOW_DOWN_ANIMATION_FRAME 130

#define BORDER_THICKNESS 3

// Animation
static AppTimer *s_animation_timer = NULL;
static int current_animation_frame = 0; // Ranges from NUM_ANIMATION_FRAMES down to 0
static int current_decrease_rate = 1;

// Buffer to hold the time string (e.g., "12:34" or "23:59")
static char s_time_buffer[9];
static char s_date_buffer[8];

// Flag to track the state of the animation
static bool s_is_animation_running = false;

// Forward declarations
static void update_time();
static void battery_handler(BatteryChargeState state);
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
static void animation_timer_callback(void *data);
static void try_start_animation_timer();
static void try_stop_animation_timer();
static void draw_frame(Layer *layer, GContext *ctx);
static void draw_animation(Layer *layer, GContext *ctx);
static void inbox_received_callback(DictionaryIterator *iterator, void *context);
static void delayed_weather_request(void *data);
static void init_info_layers(GRect bounds);
static void update_all_info_layers();
static void draw_info_for_type(InfoType info_type, InfoLayer* info_layer);
static void clear_info_layer(InfoLayer* info_layer);

// Function to update all colors based on current theme
static void update_colors() {
  window_set_background_color(s_main_window, get_background_color());

  text_layer_set_text_color(s_time_layer, get_text_color());
  text_layer_set_text_color(s_date_layer, get_text_color());

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
  uint32_t battery_resource_id = s_color_theme == 1 ? RESOURCE_ID_IMAGE_BATTERY_LIGHT : RESOURCE_ID_IMAGE_BATTERY_DARK;
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

  // Read temperature
  Tuple *temperature_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_TEMPERATURE);
  if (temperature_tuple) {
    snprintf(s_temperature_buffer, sizeof(s_temperature_buffer), "%s°C", temperature_tuple->value->cstring);
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

  text_layer_set_text(s_time_layer, s_time_buffer);

  strftime(s_date_buffer, sizeof(s_date_buffer), " %a %d", tick_time);
  text_layer_set_text(s_date_layer, s_date_buffer);

  update_step_count();
  
  // Update info layers to reflect new step count
  update_all_info_layers();
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
  if (!s_is_animation_running) {
    return;
  }

  if (s_animation_timer) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Stop animation timer");
    app_timer_cancel(s_animation_timer);
    s_animation_timer = NULL;
  }

  s_is_animation_running = false;
  current_animation_frame = 0;
}

/**
 * @brief Starts the animation timer if it's not already running.
 */
static void try_start_animation_timer() {
  if (s_is_animation_running) {
    return;
  }

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Start animation timer");
  current_animation_frame = NUM_ANIMATION_FRAMES;
  current_decrease_rate = ANIMATION_DECREASE_STEP;
  s_is_animation_running = true;

  s_animation_timer = app_timer_register(VERY_FIRST_ANIMATION_FRAME, animation_timer_callback, NULL);
}

/**
 * @brief AppTimer callback that triggers the frame redraw and reschedules itself.
 */
static void animation_timer_callback(void *data) {
  // If the animation was stopped between the last timer call and now, don't restart it
  if (!s_is_animation_running) {
    return;
  }

  current_animation_frame -= current_decrease_rate;
  if(current_animation_frame < 0) {
    try_stop_animation_timer();
    return; // try_stop_animation_timer will trigger the final redraw
  }

  if(current_animation_frame <= SLOW_DOWN_ANIMATION_FRAME && current_decrease_rate > 2) {
    current_decrease_rate -= 1;
  }

  // Mark the frame layer as dirty to trigger a redraw for the animation effect
  layer_mark_dirty(s_animation_layer);

  // Reschedule the timer for the next frame, creating a continuous loop
  s_animation_timer = app_timer_register(ANIMATION_RATE_MS, animation_timer_callback, NULL);
}


// --- Frame Layer Drawing Update Procedure (Modified for Line Fly-In) ---
static void draw_frame(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GColor frame_color = get_text_color(); // Use theme-appropriate color

  // In case we have light theme, we don't draw a frame
  if(BORDER_THICKNESS > 0 && s_color_theme == 0) {
    graphics_context_set_stroke_color(ctx, frame_color);
    graphics_context_set_stroke_width(ctx, BORDER_THICKNESS);
    graphics_draw_rect(ctx, GRect(1, 1, bounds.size.w - 2, bounds.size.h - 2));
  }

  // IN case we have light theme, we draw a gray rectangle in the middle
  if(s_color_theme == 1) {
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
  GColor color = get_text_color(); // Use theme-appropriate color

  // Horizontal lines above and below the time with 80% width
  const int max_line_length = bounds.size.w * 0.8;
  const int line_x_start_full = (bounds.size.w - max_line_length) / 2;
  const int time_y = bounds.size.h / 2;
  const int line_y_offset = 30; // Distance from the center of the time text

  // Calculate the current animated line length. It goes from 0 up to max_line_length.
  // current_animation_frame ranges from NUM_ANIMATION_FRAMES (max) down to 0.
  // The factor should go from 0 (when frame is max) up to 1 (when frame is 0).
  float animation_factor = 1.0f - ((float)current_animation_frame / NUM_ANIMATION_FRAMES);
  int current_line_length = (int)(max_line_length * animation_factor);

  // Safety check
  if (current_line_length < 0)
    current_line_length = 0;
  if (current_line_length > max_line_length)
    current_line_length = max_line_length;

  if (current_line_length < 1) {
    return;
  }

  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_stroke_width(ctx, BORDER_THICKNESS);

  // ANIMATE: Top line flies in from LEFT
  // Start X is fixed (line_x_start_full), End X is animated.
  graphics_draw_line(
      ctx,
      GPoint(line_x_start_full, time_y - line_y_offset),
      GPoint(line_x_start_full + current_line_length, time_y - line_y_offset));

  // ANIMATE: Bottom line flies in from RIGHT
  // End X is fixed (line_x_start_full + max_line_length), Start X is animated.
  graphics_draw_line(
      ctx,
      GPoint(line_x_start_full + max_line_length - current_line_length, time_y + line_y_offset),
      GPoint(line_x_start_full + max_line_length, time_y + line_y_offset));
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

  int theme_offset = s_color_theme == 0 ? 2 : 0;
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
  s_time_layer = text_layer_create(
      GRect(0, time_y_pos, bounds.size.w, bounds.size.h));

  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, get_text_color());
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  // Create the Date Layer (Center below time)
  s_date_layer = text_layer_create(
      GRect(0, time_y_pos + 38, bounds.size.w, 24)); // Positioned at upper left

  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, get_text_color());
  text_layer_set_text(s_date_layer, "Mon 01");
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  // Initialize the 4 info layers
  init_info_layers(bounds);
  
  // Add the info layers to the window
  for (int i = 0; i < NUM_INFO_LAYERS; i++) {
    layer_add_child(window_layer, s_info_layers[i].layer);
  }

  // Initialize bitmaps for use in info drawing functions
  uint32_t default_icon = get_weather_image_resource(s_current_weather_code);
  s_weather_icon_bitmap = gbitmap_create_with_resource(default_icon);
  
  uint32_t step_icon = s_color_theme == 1 ? RESOURCE_ID_IMAGE_STEP_LIGHT : RESOURCE_ID_IMAGE_STEP_DARK;
  s_step_icon_bitmap = gbitmap_create_with_resource(step_icon);

  uint32_t battery_icon = s_color_theme == 1 ? RESOURCE_ID_IMAGE_BATTERY_LIGHT : RESOURCE_ID_IMAGE_BATTERY_DARK;
  s_battery_icon_bitmap = gbitmap_create_with_resource(battery_icon);

  // Initialize the display with current layer assignments
  update_all_info_layers();

  // Make sure the initial time is displayed
  update_time();
}

static void main_window_unload(Window *window) {
  // Destroy the main layers
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
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
  app_message_open(64, 64); // Increase buffer size for weather data

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