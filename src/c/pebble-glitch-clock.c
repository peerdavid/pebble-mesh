#include <pebble.h>

// A pointer to the main window and layers
static Window *s_main_window;
static TextLayer *s_time_layer;
static Layer *s_glitch_layer;
static Layer *s_grid_layer;

// Pointer for the animation AppTimer
#define GRID_SIZE 15
static AppTimer *s_animation_timer = NULL;
#define ANIMATION_RATE_MS 5
#define NUM_ANIMATION_FRAMES 50
#define NUM_GLITCHES 180
#define NUM_GLITCHES_IF_ANIMATION_STOPPED 60
#define BORDER_THICKNESS 6
static int current_animation_frame = 0;

static int glitch_even[NUM_GLITCHES][4];
static int glitch_odd[NUM_GLITCHES][4];


// Buffer to hold the time string (e.g., "12:34" or "23:59")
static char s_time_buffer[9];

// Flag to track the state of the animation
static bool s_is_animation_running = false; 


// Forward declarations
static void update_time();
static void glitch_update_proc(Layer *layer, GContext *ctx);
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
static void animation_timer_callback(void *data);
static void try_start_animation_timer();
static void try_stop_animation_timer();
static void generate_two_glitches();
static void draw_grid(Layer *layer, GContext *ctx);


// --- Update Time Function ---

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  clock_copy_time_string(s_time_buffer, sizeof(s_time_buffer));
  text_layer_set_text(s_time_layer, s_time_buffer);
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
  current_animation_frame = 0;
  generate_two_glitches();
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
    
    current_animation_frame++;
    if(current_animation_frame >= NUM_ANIMATION_FRAMES) {
      // Animation duration completed, stop the animation
      // but still draw the frame otherwise its not shown.
      try_stop_animation_timer();
    }
    
    // Mark the layer as dirty to trigger a redraw
    layer_mark_dirty(s_glitch_layer);
    
    // Reschedule the timer for the next frame, creating a continuous loop
    s_animation_timer = app_timer_register(ANIMATION_RATE_MS, animation_timer_callback, NULL);
}


static void generate_glitch_for_pointer_to_array(int (*glitch_array)[4]) {
  GRect bounds = layer_get_bounds(s_glitch_layer);
  for (int i = 0; i < NUM_GLITCHES; i++) {
    glitch_array[i][0] = rand() % bounds.size.w; // x
    glitch_array[i][1] = rand() % bounds.size.h; // y
    glitch_array[i][2] = rand() % (bounds.size.w / 5) + 10; // width
    glitch_array[i][3] = 1; // height
  }
}

static void generate_two_glitches() {
  GRect bounds = layer_get_bounds(s_glitch_layer);
  generate_glitch_for_pointer_to_array(glitch_even);
  generate_glitch_for_pointer_to_array(glitch_odd);
}

// --- Glitch Layer Drawing Update Procedure (Unchanged) ---
static void glitch_update_proc(Layer *layer, GContext *ctx) {  
  GRect bounds = layer_get_bounds(layer);

  // 1. Draw the white border frame
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, BORDER_THICKNESS);
  graphics_draw_rect(ctx, GRect(1, 1, bounds.size.w - 2, bounds.size.h - 2));


  // Draw glitches
  int (*glitch_array)[4] = (rand() % 2 == 0) ? glitch_even : glitch_odd;

  // Draw the glitches from the selected array
  int num_glitches_to_draw = s_is_animation_running ? NUM_GLITCHES : NUM_GLITCHES_IF_ANIMATION_STOPPED;
  for (int i = 0; i < num_glitches_to_draw; i++) {
    // Randomly choose color for more variation (white or black to obscure text)
    if (i % 2 == 0) {
      graphics_context_set_fill_color(ctx, GColorWhite);
    } else {
      // Use the background color (Black) to create a cut-off effect on the text
      graphics_context_set_fill_color(ctx, GColorBlack);
    }

    // Draw the static line/box
    graphics_fill_rect(ctx, GRect(glitch_array[i][0], glitch_array[i][1], glitch_array[i][2], glitch_array[i][3]), 0, GCornerNone);
  }
}

static void draw_grid(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 1);
  
  // Draw horizontal lines (every 10 pixels)
  for (int y = 0; y < bounds.size.h; y += GRID_SIZE) {
    if (y < 70 || y > bounds.size.h - 70) {
        graphics_draw_line(ctx, GPoint(0, y), GPoint(bounds.size.w, y));
        continue;
    }
    graphics_draw_line(ctx, GPoint(0, y), GPoint(17, y));
    graphics_draw_line(ctx, GPoint(bounds.size.w - 17, y), GPoint(bounds.size.w, y));
  }
  
  // Draw vertical lines (every 10 pixels)
  for (int x = 0; x < bounds.size.w; x += GRID_SIZE) {
    if(x < 20 || x > bounds.size.w - 20) {
        graphics_draw_line(ctx, GPoint(x, 0), GPoint(x, bounds.size.h));
        continue;
    }
    graphics_draw_line(ctx, GPoint(x, 0), GPoint(x, bounds.size.h / 2 - 20));
    graphics_draw_line(ctx, GPoint(x, bounds.size.h / 2 + 20), GPoint(x, bounds.size.h));
  }
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
}

static void main_window_load(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Window load");
  GRect bounds = layer_get_bounds(window_get_root_layer(window));

  // 1. Create grid layer
  s_grid_layer = layer_create(bounds);
  // Draw the grid in its update procedure
  layer_set_update_proc(s_grid_layer, draw_grid);
  layer_add_child(window_get_root_layer(window), s_grid_layer);

  // 1. Create TextLayer for the time
  s_time_layer = text_layer_create(
      GRect(0, bounds.size.h / 2 - 21 - 6, bounds.size.w, bounds.size.h));

  // Transparent background for the text layer
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "00:00"); 
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));

  // 2. Create Glitch Layer on top of the Text Layer
  s_glitch_layer = layer_create(bounds);
  layer_set_update_proc(s_glitch_layer, glitch_update_proc);
  layer_add_child(window_get_root_layer(window), s_glitch_layer);

  // Make sure the initial time is displayed
  update_time();
}

static void main_window_unload(Window *window) {
  // Destroy the Layers
  text_layer_destroy(s_time_layer);
  layer_destroy(s_glitch_layer);
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
    window_stack_push(s_main_window, true);

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Finished init");
}


static void deinit() {
  try_stop_animation_timer();
  window_destroy(s_main_window);
  tick_timer_service_unsubscribe();
}

// --- Main Program Loop ---

int main(void) {
  init();
  app_event_loop();
  deinit();
}