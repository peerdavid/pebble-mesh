#include <pebble.h>
#include "config.h"

int s_color_theme = 0; // 0 = dark, 1 = light, 2 = auto day/night, 3 = auto quiet time
int s_step_goal = 10000;
int s_temperature_unit = 0;  // 0 = celsius, 1 = fahrenheit
int s_is_day = 1; // 1 = day, 0 = night
int s_enable_animations = 0; // 1 = enabled, 0 = disabled
int s_disconnect_position = 0; // 0 = disabled, 1-4 = UL/UR/LL/LR
int s_enable_notifications = 1; // 1 = enabled, 0 = disabled (default enabled)
int s_notification_duration = 0; // 0 = 5s, 1 = 10s, 2 = forever, 3 = disabled

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
    APP_LOG(APP_LOG_LEVEL_DEBUG, "No theme preference found, using default light theme");
  }
}

bool is_dark_theme(){
  if(s_color_theme == 0) {
    return true;
  }
  else if(s_color_theme == 2) {
    return s_is_day == 0;
  }
  else if(s_color_theme == 3) {
    return quiet_time_is_active();
  }

  return false;
}

bool is_light_theme(){
  return !is_dark_theme();
}


// Function to get colors based on theme
GColor get_background_color() {
  return is_light_theme() ? GColorWhite : GColorBlack;
}

GColor get_text_color() {
  return is_light_theme() ? GColorBlack : GColorWhite;
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
    APP_LOG(APP_LOG_LEVEL_DEBUG, "No step goal preference found, using default 10000");
  }
}

void save_temperature_unit_to_storage() {
  persist_write_int(PERSIST_KEY_TEMPERATURE_UNIT, s_temperature_unit);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Saved temperature unit to storage: %d", s_temperature_unit);
}

void load_temperature_unit_from_storage() {
  if (persist_exists(PERSIST_KEY_TEMPERATURE_UNIT)) {
    s_temperature_unit = persist_read_int(PERSIST_KEY_TEMPERATURE_UNIT);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded temperature unit from storage: %d", s_temperature_unit);
  } else {
    // Default to celsius if no preference exists
    APP_LOG(APP_LOG_LEVEL_DEBUG, "No temperature unit preference found, using default celsius");
  }
}

void save_enable_animations_to_storage() {
  persist_write_int(PERSIST_KEY_ENABLE_ANIMATIONS, s_enable_animations);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Saved enable animations to storage: %d", s_enable_animations);
}

void load_enable_animations_from_storage() {
  if (persist_exists(PERSIST_KEY_ENABLE_ANIMATIONS)) {
    s_enable_animations = persist_read_int(PERSIST_KEY_ENABLE_ANIMATIONS);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded enable animations from storage: %d", s_enable_animations);
  } else {
    // Default to enabled if no preference exists
    APP_LOG(APP_LOG_LEVEL_DEBUG, "No enable animations preference found, using default enabled");
  }
}

void save_layout_to_storage() {
  persist_write_int(PERSIST_KEY_LAYOUT_UPPER_LEFT, s_layer_assignments[LAYER_UPPER_LEFT]);
  persist_write_int(PERSIST_KEY_LAYOUT_UPPER_RIGHT, s_layer_assignments[LAYER_UPPER_RIGHT]);
  persist_write_int(PERSIST_KEY_LAYOUT_LOWER_LEFT, s_layer_assignments[LAYER_LOWER_LEFT]);
  persist_write_int(PERSIST_KEY_LAYOUT_LOWER_RIGHT, s_layer_assignments[LAYER_LOWER_RIGHT]);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Saved layout to storage");
}

void load_layout_from_storage() {
  if (persist_exists(PERSIST_KEY_LAYOUT_UPPER_LEFT)) {
    s_layer_assignments[LAYER_UPPER_LEFT] = persist_read_int(PERSIST_KEY_LAYOUT_UPPER_LEFT);
    s_layer_assignments[LAYER_UPPER_RIGHT] = persist_read_int(PERSIST_KEY_LAYOUT_UPPER_RIGHT);
    s_layer_assignments[LAYER_LOWER_LEFT] = persist_read_int(PERSIST_KEY_LAYOUT_LOWER_LEFT);
    s_layer_assignments[LAYER_LOWER_RIGHT] = persist_read_int(PERSIST_KEY_LAYOUT_LOWER_RIGHT);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded layout from storage");
  }
}

void save_disconnect_position_to_storage() {
  persist_write_int(PERSIST_KEY_DISCONNECT_POSITION, s_disconnect_position);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Saved disconnect position to storage: %d", s_disconnect_position);
}

void load_disconnect_position_from_storage() {
  if (persist_exists(PERSIST_KEY_DISCONNECT_POSITION)) {
    s_disconnect_position = persist_read_int(PERSIST_KEY_DISCONNECT_POSITION);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded disconnect position from storage: %d", s_disconnect_position);
  } else {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "No disconnect position preference found, using default disabled");
  }
}

void save_enable_notifications_to_storage() {
  persist_write_int(PERSIST_KEY_ENABLE_NOTIFICATIONS, s_enable_notifications);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Saved enable notifications to storage: %d", s_enable_notifications);
}

void load_enable_notifications_from_storage() {
  if (persist_exists(PERSIST_KEY_ENABLE_NOTIFICATIONS)) {
    s_enable_notifications = persist_read_int(PERSIST_KEY_ENABLE_NOTIFICATIONS);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded enable notifications from storage: %d", s_enable_notifications);
  } else {
    s_enable_notifications = 1; // Default to enabled
    APP_LOG(APP_LOG_LEVEL_DEBUG, "No enable notifications preference found, using default enabled");
  }
}

void save_notification_duration_to_storage() {
  persist_write_int(PERSIST_KEY_NOTIFICATION_DURATION, s_notification_duration);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Saved notification duration to storage: %d", s_notification_duration);
}

void load_notification_duration_from_storage() {
  if (persist_exists(PERSIST_KEY_NOTIFICATION_DURATION)) {
    s_notification_duration = persist_read_int(PERSIST_KEY_NOTIFICATION_DURATION);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded notification duration from storage: %d", s_notification_duration);
  } else {
    s_notification_duration = 0; // Default to 5s
    APP_LOG(APP_LOG_LEVEL_DEBUG, "No notification duration preference found, using default 10s");
  }
}