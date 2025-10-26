#include <pebble.h>

// A pointer to the main window and layers
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_battery_layer;
static TextLayer *s_steps_layer;
static Layer *s_glitch_layer;
static Layer *s_frame_layer;

static GBitmap *s_step_icon_bitmap;
static BitmapLayer *s_step_icon_layer;

// Pointer for the animation AppTimer
#define GRID_SIZE 8
#define CROSS_SIZE 0
#define INFO_DISTANCE 22
#define ANIMATION_RATE_MS 20
#define NUM_ANIMATION_FRAMES 500
#define ANIMATION_DECREASE_STEP 20
#define NUM_GLITCHES_IF_ANIMATION_STOPPED 0
#define BORDER_THICKNESS 7

// Animation
static AppTimer *s_animation_timer = NULL;
static int current_animation_frame = 0;


// Buffer to hold the time string (e.g., "12:34" or "23:59")
static char s_time_buffer[9];
static char s_date_buffer[8];
static char s_battery_buffer[5]; 
static char s_step_buffer[20];

// Flag to track the state of the animation
static bool s_is_animation_running = false; 


// Forward declarations
static void update_time();
// NEW: Forward declaration for battery handler
static void battery_handler(BatteryChargeState state);
static void glitch_update_proc(Layer *layer, GContext *ctx);
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
static void animation_timer_callback(void *data);
static void try_start_animation_timer();
static void try_stop_animation_timer();
static void draw_frame(Layer *layer, GContext *ctx);


// --- Update Time Function ---

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  clock_copy_time_string(s_time_buffer, sizeof(s_time_buffer));
  text_layer_set_text(s_time_layer, s_time_buffer);

  strftime(s_date_buffer, sizeof(s_date_buffer), " %a %d", tick_time);
  text_layer_set_text(s_date_layer, s_date_buffer);

  int steps = (int) health_service_sum_today(HealthMetricStepCount);
  snprintf(s_step_buffer, sizeof(s_step_buffer), "%d", steps);
  text_layer_set_text(s_steps_layer, s_step_buffer);
}

// --- Battery Handler ---
static void battery_handler(BatteryChargeState state) {
  // Write the battery percentage into the buffer
  snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%%", state.charge_percent);
  
  // Update the TextLayer
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
  s_is_animation_running = true;
  s_animation_timer = app_timer_register(ANIMATION_RATE_MS, animation_timer_callback, NULL);
}

/**
 * @brief AppTimer callback that triggers the glitch redraw and reschedules itself.
 */
static void animation_timer_callback(void *data) {
    // If the animation was stopped between the last timer call and now, don't restart it
    if (!s_is_animation_running) {
        return;
    }
    
    current_animation_frame -= ANIMATION_DECREASE_STEP;
    if(current_animation_frame <= 0) {
      // Animation duration completed, stop the animation
      // but still draw the frame otherwise its not shown.
      try_stop_animation_timer();
    }
    
    // Mark the layer as dirty to trigger a redraw
    layer_mark_dirty(s_glitch_layer);
    
    // Reschedule the timer for the next frame, creating a continuous loop
    s_animation_timer = app_timer_register(ANIMATION_RATE_MS, animation_timer_callback, NULL);
}


// --- Glitch Layer Drawing Update Procedure (Unchanged) ---
static void glitch_update_proc(Layer *layer, GContext *ctx) {  
  GRect bounds = layer_get_bounds(layer);
  // Draw the glitches from the selected array
  int num_glitches_to_draw = current_animation_frame;
  for (int i = 0; i < num_glitches_to_draw; i++) {
    // Randomly choose color for more variation (white or black to obscure text)
    if (i % 2 == 0) {
      graphics_context_set_fill_color(ctx, GColorWhite);
    } else {
      // Use the background color (Black) to create a cut-off effect on the text
      graphics_context_set_fill_color(ctx, GColorBlack);
    }

    // Draw a random dot somewhere in the bounds
    int x = rand() % bounds.size.w;
    int y = rand() % bounds.size.h;
    int w = (rand() % 20) + 3; // Width between 5 and 24
    int h = (rand() % 5) + 2;  // Height between 2 and 6
    graphics_fill_rect(ctx, GRect(x, y, w, h), 0, GCornerNone);
  }
}

static void draw_frame(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  
  // Frame
  if(BORDER_THICKNESS > 0) {
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_stroke_width(ctx, BORDER_THICKNESS);
    graphics_draw_rect(ctx, GRect(1, 1, bounds.size.w - 2, bounds.size.h - 2));
  }

  // Dots
  graphics_context_set_stroke_color(ctx, GColorWhite);
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
  int line_length = bounds.size.w * 0.8;
  int line_x_start = (bounds.size.w - line_length) / 2;
  int time_y = bounds.size.h / 2;
  int line_y_offset = 28; // Distance from the center of the time text
  
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(
    ctx, 
    GPoint(line_x_start, time_y - line_y_offset), 
    GPoint(line_x_start + line_length, time_y - line_y_offset)
  );
  graphics_draw_line(
    ctx, 
    GPoint(line_x_start, time_y + line_y_offset), 
    GPoint(line_x_start + line_length, time_y + line_y_offset)
  );
}


// --- Tick Handler ---
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  try_start_animation_timer();
  update_time();
}


// --- Window Load/Unload Handlers ---
static void main_window_appear(Window *window) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Window appear");
    try_start_animation_timer();
    
    // NEW: Get the current battery state and display it
    battery_handler(battery_state_service_peek());
}

static void main_window_load(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Window load");
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create grid layer
  s_frame_layer = layer_create(bounds);
  layer_set_update_proc(s_frame_layer, draw_frame);
  layer_add_child(window_layer, s_frame_layer);

  // Create Time Layer
  s_time_layer = text_layer_create(
      GRect(0, bounds.size.h / 2 - 21 - 6, bounds.size.w, bounds.size.h));

  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "00:00"); 
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  // Create the Date Layer (Upper Left)
  s_date_layer = text_layer_create(
      GRect(INFO_DISTANCE/2-4, INFO_DISTANCE-14, 54, 24)); // Positioned at upper left

  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text(s_date_layer, "Mon 01");
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  
  // Create the Battery Layer (Upper Right)
  s_battery_layer = text_layer_create(
      GRect(bounds.size.w - 62, INFO_DISTANCE-14, 50, 24)); // Positioned at upper right
      
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_text_color(s_battery_layer, GColorWhite);
  text_layer_set_text(s_battery_layer, "100%"); // Default text
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
  
  // Load icon
  const int icon_x = INFO_DISTANCE/2-4; // Example X position
  const int icon_y = bounds.size.h - INFO_DISTANCE - 14; // Example Y position

  s_step_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STEP);
  s_step_icon_layer = bitmap_layer_create(
      GRect(icon_x, icon_y, 24, 24)
  );
  bitmap_layer_set_bitmap(s_step_icon_layer, s_step_icon_bitmap);
  bitmap_layer_set_compositing_mode(s_step_icon_layer, GCompOpSet); // Renders the icon cleanly
  layer_add_child(window_layer, bitmap_layer_get_layer(s_step_icon_layer));

  // Create Steps Layer (Lower Left) - Placeholder for future use
  s_steps_layer = text_layer_create(
      GRect(icon_x + 24, icon_y, 50, 24)); // Positioned at lower left
  text_layer_set_background_color(s_steps_layer, GColorClear);
  text_layer_set_text_color(s_steps_layer, GColorWhite);
  text_layer_set_text(s_steps_layer, "???"); // Placeholder text
  text_layer_set_font(s_steps_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_steps_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(s_steps_layer));

  // Create Glitch Layer on top of the Text Layer
  s_glitch_layer = layer_create(bounds);
  layer_set_update_proc(s_glitch_layer, glitch_update_proc);
  layer_add_child(window_layer, s_glitch_layer);

  // Make sure the initial time is displayed
  update_time();
}

static void main_window_unload(Window *window) {
  // Destroy the Layers
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_steps_layer);
  layer_destroy(s_frame_layer);
  layer_destroy(s_glitch_layer);
  bitmap_layer_destroy(s_step_icon_layer);
  gbitmap_destroy(s_step_icon_bitmap);
}


// --- Initialization and Deinitialization ---

static void init() {
    srand(time(NULL)); 
    
    s_main_window = window_create();
    window_set_background_color(s_main_window, GColorBlack); 
    
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload,
        .appear = main_window_appear // NEW: Window appear handler
    });
    
    // Subscribe to MINUTE_UNIT (for time update/minute-start trigger) and SECOND_UNIT (for stop trigger)
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    
    // NEW: Subscribe to battery state updates
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