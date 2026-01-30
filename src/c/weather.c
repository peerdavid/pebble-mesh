#include "weather.h"


/*
 * Definitions
 */
int s_current_weather_code = -1; // -1 indicates no weather data yet
GBitmap *s_weather_icon_bitmap;
char s_temperature_buffer[8];
char s_location_buffer[20];

 /*
  * Draw Functions
  */
void draw_weather_info(InfoLayer* info_layer) {
  Layer* layer = info_layer->layer;
  GRect bounds = layer_get_bounds(layer);
  int y_pos = bounds.size.h / 2 - 16;
  int x_pos = (bounds.size.w / 2) - 16;
  
  // Create a bitmap layer for the weather icon
  info_layer->bitmap_layer_1 = bitmap_layer_create(GRect(x_pos, y_pos, 32, 32));
  bitmap_layer_set_bitmap(info_layer->bitmap_layer_1, s_weather_icon_bitmap);
  bitmap_layer_set_compositing_mode(info_layer->bitmap_layer_1, GCompOpSet);
  
  // Add to the info layer
  layer_add_child(layer, bitmap_layer_get_layer(info_layer->bitmap_layer_1));
}

void draw_temperature_info(InfoLayer* info_layer) {
  GRect bounds = info_layer->bounds;
  Layer* layer = info_layer->layer;
  int y_center = bounds.size.h / 2 - 10;
  
  // Create temperature text layer
  GRect temp_frame = GRect(0, y_center-14, bounds.size.w, 24);
  info_layer->text_layer1 = text_layer_create(temp_frame);
  text_layer_set_background_color(info_layer->text_layer1, GColorClear);
  text_layer_set_text_color(info_layer->text_layer1, get_text_color());
  text_layer_set_text(info_layer->text_layer1, s_temperature_buffer);
  text_layer_set_font(info_layer->text_layer1, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  
  // Create location text layer
  GRect location_frame = GRect(0, y_center+8, bounds.size.w, 18);
  info_layer->text_layer2 = text_layer_create(location_frame);
  text_layer_set_background_color(info_layer->text_layer2, GColorClear);
  text_layer_set_text_color(info_layer->text_layer2, get_text_color());
  text_layer_set_text(info_layer->text_layer2, s_location_buffer);
  text_layer_set_font(info_layer->text_layer2, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  
  // Set alignment based on layer alignment
  text_layer_set_text_alignment(info_layer->text_layer1, GTextAlignmentCenter);
  text_layer_set_text_alignment(info_layer->text_layer2, GTextAlignmentCenter);

  // Add to the info layer
  layer_add_child(layer, text_layer_get_layer(info_layer->text_layer1));
  layer_add_child(layer, text_layer_get_layer(info_layer->text_layer2));
}

uint32_t get_weather_image_resource(int weather_code) {
  // Handle error case with question mark icon
  if (weather_code == -1) {
    return s_color_theme == 1 ? RESOURCE_ID_IMAGE_QUESTION_LIGHT : RESOURCE_ID_IMAGE_QUESTION_DARK; // Error/No data
  }

  // Based on WMO Weather interpretation codes
  if (weather_code == 0) {
    if (s_is_day == 1) {
      return s_color_theme == 1 ? RESOURCE_ID_IMAGE_SUNNY_LIGHT : RESOURCE_ID_IMAGE_SUNNY_DARK; // Clear sky day
    } else {
      return s_color_theme == 1 ? RESOURCE_ID_IMAGE_CLEAR_NIGHT_LIGHT : RESOURCE_ID_IMAGE_CLEAR_NIGHT_DARK; // Clear sky night
    }
  } else if (weather_code <= 3) {
    if (s_is_day == 1) {
      return s_color_theme == 1 ? RESOURCE_ID_IMAGE_PARTLY_CLOUDY_LIGHT : RESOURCE_ID_IMAGE_PARTLY_CLOUDY_DARK; // Mostly cloudy day
    } else {
      return s_color_theme == 1 ? RESOURCE_ID_IMAGE_PARTLY_CLOUDY_NIGHT_LIGHT : RESOURCE_ID_IMAGE_PARTLY_CLOUDY_NIGHT_DARK; // Mostly cloudy night
    }
  } else if (weather_code <= 48) {
    if (s_is_day == 1) {
      return s_color_theme == 1 ? RESOURCE_ID_IMAGE_PARTLY_CLOUDY_LIGHT : RESOURCE_ID_IMAGE_PARTLY_CLOUDY_DARK; // Mostly cloudy day
    } else {
      return s_color_theme == 1 ? RESOURCE_ID_IMAGE_PARTLY_CLOUDY_NIGHT_LIGHT : RESOURCE_ID_IMAGE_PARTLY_CLOUDY_NIGHT_DARK; // Mostly cloudy night
    }
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
  }
  
  return s_color_theme == 1 ? RESOURCE_ID_IMAGE_QUESTION_LIGHT : RESOURCE_ID_IMAGE_QUESTION_DARK; // Unknown
}

/*
* App Connection
 */
void request_weather_update() {
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

/*
 * Storage of weather 
 */
void save_weather_to_storage() {
  persist_write_int(PERSIST_KEY_WEATHER_CODE, s_current_weather_code);
  persist_write_string(PERSIST_KEY_TEMPERATURE, s_temperature_buffer);
  persist_write_string(PERSIST_KEY_LOCATION, s_location_buffer);
  persist_write_int(PERSIST_KEY_IS_DAY, s_is_day);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Saved weather to storage: code=%d, temp=%s, location=%s", 
          s_current_weather_code, s_temperature_buffer, s_location_buffer);
}

void load_weather_from_storage() {
  // Load weather code
  if (persist_exists(PERSIST_KEY_WEATHER_CODE)) {
    s_current_weather_code = persist_read_int(PERSIST_KEY_WEATHER_CODE);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded weather code from storage: %d", s_current_weather_code);
  }
  
  // Load temperature
  if (persist_exists(PERSIST_KEY_TEMPERATURE)) {
    persist_read_string(PERSIST_KEY_TEMPERATURE, s_temperature_buffer, sizeof(s_temperature_buffer));
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded temperature from storage: %s", s_temperature_buffer);
  } else {
    // Use the current temperature unit for the default display
    const char* unit_symbol = s_temperature_unit == 1 ? "°F" : "°C";
    snprintf(s_temperature_buffer, sizeof(s_temperature_buffer), "--%s", unit_symbol);
  }
  
  // Load location
  if (persist_exists(PERSIST_KEY_LOCATION)) {
    persist_read_string(PERSIST_KEY_LOCATION, s_location_buffer, sizeof(s_location_buffer));
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded location from storage: %s", s_location_buffer);
  } else {
    snprintf(s_location_buffer, sizeof(s_location_buffer), "---");
  }

  // Load is_day
  if (persist_exists(PERSIST_KEY_IS_DAY)) {
    s_is_day = persist_read_int(PERSIST_KEY_IS_DAY);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded is_day from storage: %d", s_is_day);
  } else {
    s_is_day = 1; // Default to day
  }
}