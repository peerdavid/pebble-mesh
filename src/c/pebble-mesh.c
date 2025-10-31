#include <pebble.h>

// A pointer to the main window and layers
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_battery_layer;
static TextLayer *s_steps_layer;
static TextLayer *s_temperature_layer;
static TextLayer *s_location_layer;
static BitmapLayer *s_weather_icon_layer;
static Layer *s_frame_layer;

static GBitmap *s_step_icon_bitmap;
static BitmapLayer *s_step_icon_layer;
static GBitmap *s_weather_icon_bitmap;

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
static int current_decrease_rate =  1;

// Buffer to hold the time string (e.g., "12:34" or "23:59")
static char s_time_buffer[9];
static char s_date_buffer[8];
static char s_battery_buffer[5]; 
static char s_step_buffer[20];
static char s_temperature_buffer[8];
static char s_location_buffer[20];

// Flag to track the state of the animation
static bool s_is_animation_running = false; 

// Color theme (0 = dark, 1 = light)
static int s_color_theme = 0;

// Current weather code (stored for theme changes)
static int s_current_weather_code = 0; 


// Forward declarations
static void update_time();
static void battery_handler(BatteryChargeState state);
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
static void animation_timer_callback(void *data);
static void try_start_animation_timer();
static void try_stop_animation_timer();
static void draw_frame(Layer *layer, GContext *ctx);
static void inbox_received_callback(DictionaryIterator *iterator, void *context);
static void request_weather_update();
static void update_weather_icon();
static void update_step_icon();
static void delayed_weather_request(void *data);

// Function to map weather codes to image resource IDs based on theme
static uint32_t get_weather_image_resource(int weather_code) {
  
  // Based on WMO Weather interpretation codes
  if (weather_code == 0) {
    return s_color_theme == 1 ? RESOURCE_ID_IMAGE_SUNNY_LIGHT : RESOURCE_ID_IMAGE_SUNNY_DARK; // Clear sky
  } else if (weather_code <= 3) {
    return s_color_theme == 1 ? RESOURCE_ID_IMAGE_PARTLY_CLOUDY_LIGHT : RESOURCE_ID_IMAGE_PARTLY_CLOUDY_DARK; // Partly cloudy
  } else if (weather_code <= 48) {
    return s_color_theme == 1 ? RESOURCE_ID_IMAGE_CLOUDY_LIGHT : RESOURCE_ID_IMAGE_CLOUDY_DARK; // Overcast
  } else if (weather_code <= 57) {
    return s_color_theme == 1 ? RESOURCE_ID_IMAGE_LIGHT_RAIN_LIGHT : RESOURCE_ID_IMAGE_LIGHT_RAIN_DARK; // Drizzle
  } else if (weather_code <= 67) {
    return s_color_theme == 1 ? RESOURCE_ID_IMAGE_HEAVY_RAIN_LIGHT : RESOURCE_ID_IMAGE_HEAVY_RAIN_DARK; // Rain
  } else if (weather_code <= 77) {
    return s_color_theme == 1 ? RESOURCE_ID_IMAGE_LIGHT_SNOW_LIGHT : RESOURCE_ID_IMAGE_LIGHT_SNOW_DARK; // Snow
  } else if (weather_code <= 82) {
    return s_color_theme == 1 ? RESOURCE_ID_IMAGE_LIGHT_RAIN_LIGHT : RESOURCE_ID_IMAGE_LIGHT_RAIN_DARK; // Rain showers
  } else if (weather_code <= 86) {
    return s_color_theme == 1 ? RESOURCE_ID_IMAGE_HEAVY_SNOW_LIGHT : RESOURCE_ID_IMAGE_HEAVY_SNOW_DARK; // Snow showers
  } else if (weather_code <= 99) {
    return s_color_theme == 1 ? RESOURCE_ID_IMAGE_THUNDERSTORM_LIGHT : RESOURCE_ID_IMAGE_THUNDERSTORM_DARK; // Thunderstorm
  } else {
    return s_color_theme == 1 ? RESOURCE_ID_IMAGE_GENERIC_WEATHER_LIGHT : RESOURCE_ID_IMAGE_GENERIC_WEATHER_DARK; // Unknown
  }
}

// Function to get colors based on theme
static GColor get_background_color() {
  return s_color_theme == 1 ? GColorWhite : GColorBlack;
}

static GColor get_text_color() {
  return s_color_theme == 1 ? GColorBlack : GColorWhite;
}

// Function to update all colors based on current theme
static void update_colors() {
  window_set_background_color(s_main_window, get_background_color());
  
  text_layer_set_text_color(s_time_layer, get_text_color());
  text_layer_set_text_color(s_date_layer, get_text_color());
  text_layer_set_text_color(s_temperature_layer, get_text_color());
  text_layer_set_text_color(s_location_layer, get_text_color());
  text_layer_set_text_color(s_battery_layer, get_text_color());
  text_layer_set_text_color(s_steps_layer, get_text_color());
  
  // Always use GCompOpSet for all bitmap layers
  bitmap_layer_set_compositing_mode(s_weather_icon_layer, GCompOpSet);
  bitmap_layer_set_compositing_mode(s_step_icon_layer, GCompOpSet);
  
  // Update weather icon for new theme
  update_weather_icon();
  update_step_icon();
  
  // Force redraw
  layer_mark_dirty(s_frame_layer);
}

// Function to update weather icon based on current weather code and theme
static void update_weather_icon() {
  uint32_t resource_id;
  
  if (s_current_weather_code > 0) {
    // Use actual weather condition
    resource_id = get_weather_image_resource(s_current_weather_code);
  } else {
    // Use default sunny icon based on theme
    resource_id = s_color_theme == 1 ? RESOURCE_ID_IMAGE_SUNNY_LIGHT : RESOURCE_ID_IMAGE_SUNNY_DARK;
  }
  
  // Destroy the old bitmap if it exists
  if (s_weather_icon_bitmap) {
    gbitmap_destroy(s_weather_icon_bitmap);
  }
  
  // Load the new weather icon bitmap
  s_weather_icon_bitmap = gbitmap_create_with_resource(resource_id);
  bitmap_layer_set_bitmap(s_weather_icon_layer, s_weather_icon_bitmap);
}

static void update_step_icon() {
  // Destroy the old bitmap if it exists
  if (s_step_icon_bitmap) {
    gbitmap_destroy(s_step_icon_bitmap);
  }

  // Load the new step icon bitmap based on theme
  s_step_icon_bitmap = gbitmap_create_with_resource(
    s_color_theme == 1 ? RESOURCE_ID_IMAGE_STEP_LIGHT : RESOURCE_ID_IMAGE_STEP_DARK
  );
  bitmap_layer_set_bitmap(s_step_icon_layer, s_step_icon_bitmap);
}


// --- Weather Functions ---

static void request_weather_update() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Requesting weather update");
  
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  
  if (iter == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to create message iterator");
    return;
  }
  
  dict_write_uint8(iter, MESSAGE_KEY_WEATHER_REQUEST, 1);
  app_message_outbox_send();
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Message received");
  
  // Read temperature
  Tuple *temperature_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_TEMPERATURE);
  if (temperature_tuple) {
    snprintf(s_temperature_buffer, sizeof(s_temperature_buffer), "%s°C", temperature_tuple->value->cstring);
    text_layer_set_text(s_temperature_layer, s_temperature_buffer);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Temperature: %s", s_temperature_buffer);
  }
  
  // Read location
  Tuple *location_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_LOCATION);
  if (location_tuple) {
    snprintf(s_location_buffer, sizeof(s_location_buffer), "%s", location_tuple->value->cstring);
    text_layer_set_text(s_location_layer, s_location_buffer);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Location: %s", s_location_buffer);
  }
  
  // Read weather condition and update icon
  Tuple *condition_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_CONDITION);
  if (condition_tuple) {
    s_current_weather_code = (int)condition_tuple->value->int32;
    update_weather_icon();
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Weather condition: %d", s_current_weather_code);
  }
  
  // Read color theme
  Tuple *theme_tuple = dict_find(iterator, MESSAGE_KEY_COLOR_THEME);
  if (theme_tuple) {
    s_color_theme = (int)theme_tuple->value->int32;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Color theme: %d", s_color_theme);
    update_colors();
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
      // Remove leading zero on 12-hour clock
      if (s_time_buffer[0] == '0') {
          memmove(s_time_buffer, &s_time_buffer[1], sizeof(s_time_buffer) - 1);
      }
  }

  text_layer_set_text(s_time_layer, s_time_buffer);

  strftime(s_date_buffer, sizeof(s_date_buffer), " %a %d", tick_time);
  text_layer_set_text(s_date_layer, s_date_buffer);

  int steps = (int) health_service_sum_today(HealthMetricStepCount);
  snprintf(s_step_buffer, sizeof(s_step_buffer), "%d", steps);
  text_layer_set_text(s_steps_layer, s_step_buffer);
}

// --- Battery Handler ---
static void battery_handler(BatteryChargeState state) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Battery handler");
  snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%%", state.charge_percent);
  text_layer_set_text(s_battery_layer, s_battery_buffer);
}

/**
 * @brief Stops the animation timer and resets the running flag.
 */
static void try_stop_animation_timer() {
    if(!s_is_animation_running) {
        return; 
    }

    if (s_animation_timer) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Stop animation timer");
        app_timer_cancel(s_animation_timer);
        s_animation_timer = NULL;
    }

    s_is_animation_running = false; 
    current_animation_frame = 0;
    
    // Ensure the final state is drawn (full length)
    layer_mark_dirty(s_frame_layer);
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
  layer_mark_dirty(s_frame_layer);
  
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
        GPoint(x + CROSS_SIZE, y)
      );
      
      // 2. Draw the vertical part of the cross
      // Starts at y - CROSS_SIZE and ends at y + CROSS_SIZE, centered on x
      graphics_draw_line(
        ctx, 
        GPoint(x, y - CROSS_SIZE), 
        GPoint(x, y + CROSS_SIZE)
      );
    }
  }

  // Horizontal lines above and below the time with 80% width
  const int max_line_length = bounds.size.w * 0.8;
  const int line_x_start_full = (bounds.size.w - max_line_length) / 2;
  const int time_y = bounds.size.h / 2;
  const int line_y_offset = 29; // Distance from the center of the time text
  
  // Calculate the current animated line length. It goes from 0 up to max_line_length.
  // current_animation_frame ranges from NUM_ANIMATION_FRAMES (max) down to 0.
  // The factor should go from 0 (when frame is max) up to 1 (when frame is 0).
  float animation_factor = 1.0f - ((float)current_animation_frame / NUM_ANIMATION_FRAMES);
  int current_line_length = (int)(max_line_length * animation_factor);
  
  // Safety check
  if (current_line_length < 0) current_line_length = 0;
  if (current_line_length > max_line_length) current_line_length = max_line_length;

  if (current_line_length < 5) {
    return;
  }

  graphics_context_set_stroke_color(ctx, frame_color);
  graphics_context_set_stroke_width(ctx, BORDER_THICKNESS);

  // ANIMATE: Top line flies in from LEFT
  // Start X is fixed (line_x_start_full), End X is animated.
  graphics_draw_line(
    ctx, 
    GPoint(line_x_start_full, time_y - line_y_offset - 3), 
    GPoint(line_x_start_full + current_line_length, time_y - line_y_offset - 3)
  );

  // ANIMATE: Bottom line flies in from RIGHT
  // End X is fixed (line_x_start_full + max_line_length), Start X is animated.
  graphics_draw_line(
    ctx, 
    GPoint(line_x_start_full + max_line_length - current_line_length, time_y + line_y_offset), 
    GPoint(line_x_start_full + max_line_length, time_y + line_y_offset)
  );
}


// --- Tick Handler ---
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  try_start_animation_timer();
  update_time();
}


// Timer callback to request weather after UI is loaded
static void delayed_weather_request(void *data) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Requesting weather update (delayed)");
    request_weather_update();
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

  // Create Time Layer
  const int time_y_pos = bounds.size.h / 2 - 21 - 14;
  s_time_layer = text_layer_create(
      GRect(0, time_y_pos, bounds.size.w, bounds.size.h));

  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "00:00"); 
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  // Create the Date Layer (Center below time)
  s_date_layer = text_layer_create(
      GRect(0, time_y_pos + 40, bounds.size.w, 24)); // Positioned at upper left

  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text(s_date_layer, "Mon 01");
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  
  // Create the Temperature Layer (Top Right)
  s_temperature_layer = text_layer_create(
      GRect(bounds.size.w - 68, 6, 60, 24)); // Positioned at top right, moved up

  text_layer_set_background_color(s_temperature_layer, GColorClear);
  text_layer_set_text_color(s_temperature_layer, GColorWhite);
  text_layer_set_text(s_temperature_layer, "--°C");
  text_layer_set_font(s_temperature_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_temperature_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(s_temperature_layer));
  
  // Create the Location Layer (Below temperature)
  s_location_layer = text_layer_create(
      GRect(bounds.size.w - 80, 24, 72, 24)); // Positioned below temperature

  text_layer_set_background_color(s_location_layer, GColorClear);
  text_layer_set_text_color(s_location_layer, GColorWhite);
  text_layer_set_text(s_location_layer, "---");
  text_layer_set_font(s_location_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_location_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(s_location_layer));
  
  // Create the Weather Icon Layer (Top Left)
  s_weather_icon_layer = bitmap_layer_create(
      GRect(0, 0, 50, 50)); // Positioned at top left

  // Load default weather icon based on current theme
  uint32_t default_icon = s_color_theme == 1 ? RESOURCE_ID_IMAGE_SUNNY_LIGHT : RESOURCE_ID_IMAGE_SUNNY_DARK;
  s_weather_icon_bitmap = gbitmap_create_with_resource(default_icon);
  bitmap_layer_set_bitmap(s_weather_icon_layer, s_weather_icon_bitmap);
  bitmap_layer_set_compositing_mode(s_weather_icon_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_weather_icon_layer));

  // Create the Battery Layer
  const int lower_y = bounds.size.h - 32; // Example Y position
  const int battery_width = 38;
  s_battery_layer = text_layer_create(
      GRect(bounds.size.w - battery_width - 20, lower_y, 50, 24)); // Positioned at upper right
      
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_text_color(s_battery_layer, GColorWhite);
  text_layer_set_text(s_battery_layer, "100%"); // Default text
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
  
  // Load icon
  s_step_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STEP_LIGHT);
  s_step_icon_layer = bitmap_layer_create(
      GRect(4, lower_y, 24, 24)
  );
  bitmap_layer_set_bitmap(s_step_icon_layer, s_step_icon_bitmap);
  bitmap_layer_set_compositing_mode(s_step_icon_layer, GCompOpSet); // Renders the icon cleanly
  layer_add_child(window_layer, bitmap_layer_get_layer(s_step_icon_layer));

  // Create Steps Layer (Lower Left) - Placeholder for future use
  s_steps_layer = text_layer_create(
      GRect(4 + 24, lower_y, 50, 24)); // Positioned at lower left
  text_layer_set_background_color(s_steps_layer, GColorClear);
  text_layer_set_text_color(s_steps_layer, GColorWhite);
  text_layer_set_text(s_steps_layer, "???"); // Placeholder text
  text_layer_set_font(s_steps_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_steps_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(s_steps_layer));

  // Make sure the initial time is displayed
  update_time();
  
  // Apply initial theme colors
  update_colors();
}

static void main_window_unload(Window *window) {
  // Destroy the Layers
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_steps_layer);
  text_layer_destroy(s_temperature_layer);
  text_layer_destroy(s_location_layer);
  bitmap_layer_destroy(s_weather_icon_layer);
  layer_destroy(s_frame_layer);
  bitmap_layer_destroy(s_step_icon_layer);
  gbitmap_destroy(s_step_icon_bitmap);
  
  // Destroy weather icon bitmap if it exists
  if (s_weather_icon_bitmap) {
    gbitmap_destroy(s_weather_icon_bitmap);
  }
}


// --- Initialization and Deinitialization ---

static void init() {
    srand(time(NULL)); 
    
    // Initialize weather data buffers with clean placeholders
    snprintf(s_temperature_buffer, sizeof(s_temperature_buffer), "--°C");
    snprintf(s_location_buffer, sizeof(s_location_buffer), "---");
    
    s_main_window = window_create();
    window_set_background_color(s_main_window, GColorBlack); 
    
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload,
        .appear = main_window_appear
    });
    
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

int main(void) {
  init();
  app_event_loop();
  deinit();
}