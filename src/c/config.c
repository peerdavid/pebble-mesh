#include <pebble.h>
#include "config.h"

int s_color_theme = 0; // 0 = dark, 1 = light, 2 = auto day/night, 3 = auto quiet time
int s_step_goal = 10000;
int s_temperature_unit = 0;  // 0 = celsius, 1 = fahrenheit
int s_is_day = 1; // 1 = day, 0 = night
int s_enable_animations = 0; // 1 = enabled, 0 = disabled
int s_disconnect_position = 0; // 0 = disabled, 1-4 = UL/UR/LL/LR
int s_enable_weather_forecast = 1; // 1 = enabled, 0 = disabled (default enabled)
int s_weather_forecast_duration = 0; // 0 = 5s, 1 = 10s, 2 = forever
int s_weather_forecast_flick_mode = 2; // 0 = disabled, 1 = single flick, 2 = double flick
int s_enable_mesh = 1; // 1 = enabled, 0 = disabled
char s_date_format[16] = " %a %d"; // strftime format string for date display
int s_light_show_background = 1; // 1 = show gray box in light theme, 0 = hide
int s_dark_show_border = 1; // 1 = show border in dark theme, 0 = hide

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


void save_weather_forecast_duration_to_storage() {
  persist_write_int(PERSIST_KEY_WEATHER_FORECAST_DURATION, s_weather_forecast_duration);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Saved weather forecast duration to storage: %d", s_weather_forecast_duration);
}

void load_weather_forecast_duration_from_storage() {
  if (persist_exists(PERSIST_KEY_WEATHER_FORECAST_DURATION)) {
    s_weather_forecast_duration = persist_read_int(PERSIST_KEY_WEATHER_FORECAST_DURATION);
    // Migrate old "disabled" value 3 to the new flick mode setting
    if (s_weather_forecast_duration == 3) {
      s_weather_forecast_duration = 0;
      s_weather_forecast_flick_mode = 0;
      save_weather_forecast_duration_to_storage();
      save_weather_forecast_flick_mode_to_storage();
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded weather forecast duration from storage: %d", s_weather_forecast_duration);
  } else {
    s_weather_forecast_duration = 0; // Default to 5s
    APP_LOG(APP_LOG_LEVEL_DEBUG, "No weather forecast duration preference found, using default 5s");
  }
}

void save_weather_forecast_flick_mode_to_storage() {
  persist_write_int(PERSIST_KEY_WEATHER_FORECAST_FLICK_MODE, s_weather_forecast_flick_mode);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Saved weather forecast flick mode to storage: %d", s_weather_forecast_flick_mode);
}

void load_weather_forecast_flick_mode_from_storage() {
  if (persist_exists(PERSIST_KEY_WEATHER_FORECAST_FLICK_MODE)) {
    s_weather_forecast_flick_mode = persist_read_int(PERSIST_KEY_WEATHER_FORECAST_FLICK_MODE);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded weather forecast flick mode from storage: %d", s_weather_forecast_flick_mode);
  } else {
    s_weather_forecast_flick_mode = 2; // Default to double flick
    APP_LOG(APP_LOG_LEVEL_DEBUG, "No weather forecast flick mode preference found, using default double flick");
  }
}

void save_enable_mesh_to_storage() {
  persist_write_int(PERSIST_KEY_ENABLE_MESH, s_enable_mesh);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Saved enable mesh to storage: %d", s_enable_mesh);
}

void load_enable_mesh_from_storage() {
  if (persist_exists(PERSIST_KEY_ENABLE_MESH)) {
    s_enable_mesh = persist_read_int(PERSIST_KEY_ENABLE_MESH);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded enable mesh from storage: %d", s_enable_mesh);
  } else {
    s_enable_mesh = 1; // Default to enabled
    APP_LOG(APP_LOG_LEVEL_DEBUG, "No enable mesh preference found, using default enabled");
  }
}

void save_date_format_to_storage() {
  persist_write_string(PERSIST_KEY_DATE_FORMAT, s_date_format);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Saved date format to storage: %s", s_date_format);
}

void load_date_format_from_storage() {
  if (persist_exists(PERSIST_KEY_DATE_FORMAT)) {
    persist_read_string(PERSIST_KEY_DATE_FORMAT, s_date_format, sizeof(s_date_format));
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded date format from storage: %s", s_date_format);
  } else {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "No date format preference found, using default");
  }
}

void save_light_show_background_to_storage() {
  persist_write_int(PERSIST_KEY_LIGHT_SHOW_BACKGROUND, s_light_show_background);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Saved light show background to storage: %d", s_light_show_background);
}

void load_light_show_background_from_storage() {
  if (persist_exists(PERSIST_KEY_LIGHT_SHOW_BACKGROUND)) {
    s_light_show_background = persist_read_int(PERSIST_KEY_LIGHT_SHOW_BACKGROUND);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded light show background from storage: %d", s_light_show_background);
  } else {
    s_light_show_background = 1; // Default to enabled
    APP_LOG(APP_LOG_LEVEL_DEBUG, "No light show background preference found, using default enabled");
  }
}

void save_dark_show_border_to_storage() {
  persist_write_int(PERSIST_KEY_DARK_SHOW_BORDER, s_dark_show_border);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Saved dark show border to storage: %d", s_dark_show_border);
}

void load_dark_show_border_from_storage() {
  if (persist_exists(PERSIST_KEY_DARK_SHOW_BORDER)) {
    s_dark_show_border = persist_read_int(PERSIST_KEY_DARK_SHOW_BORDER);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded dark show border from storage: %d", s_dark_show_border);
  } else {
    s_dark_show_border = 1; // Default to enabled
    APP_LOG(APP_LOG_LEVEL_DEBUG, "No dark show border preference found, using default enabled");
  }
}