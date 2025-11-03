#include <pebble.h>
#include "config.h"

int s_color_theme = 1;
int s_step_goal = 10000;

InfoLayer s_info_layers[NUM_INFO_LAYERS];
InfoType s_layer_assignments[NUM_INFO_LAYERS] = {
  INFO_TYPE_WEATHER,      // LAYER_UPPER_LEFT
  INFO_TYPE_TEMPERATURE,  // LAYER_UPPER_RIGHT  
  INFO_TYPE_STEPS,        // LAYER_LOWER_LEFT
  INFO_TYPE_BATTERY       // LAYER_LOWER_RIGHT
};


// Persistent storage functions
void save_theme_to_storage() {
  persist_write_int(PERSIST_KEY_COLOR_THEME, s_color_theme);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Saved theme to storage: %d", s_color_theme);
}

void load_theme_from_storage() {
  if (persist_exists(PERSIST_KEY_COLOR_THEME)) {
    s_color_theme = persist_read_int(PERSIST_KEY_COLOR_THEME);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded theme from storage: %d", s_color_theme);
  } else {
    // Default to light theme if no preference exists
    s_color_theme = 1;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "No theme preference found, using default light theme");
  }
}


// Function to get colors based on theme
GColor get_background_color() {
  return s_color_theme == 1 ? GColorWhite : GColorBlack;
}

GColor get_text_color() {
  return s_color_theme == 1 ? GColorBlack : GColorWhite;
}

void save_step_goal_to_storage() {
  persist_write_int(PERSIST_KEY_STEP_GOAL, s_step_goal);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Saved step goal to storage: %d", s_step_goal);
}

void load_step_goal_from_storage() {
  if (persist_exists(PERSIST_KEY_STEP_GOAL)) {
    s_step_goal = persist_read_int(PERSIST_KEY_STEP_GOAL);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded step goal from storage: %d", s_step_goal);
  } else {
    // Default to 10000 if no preference exists
    s_step_goal = 10000;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "No step goal preference found, using default 10000");
  }
}
