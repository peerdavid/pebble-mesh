#include "weather.h"
#include "utils.h"


/*
 * Definitions
 */
int s_current_weather_code = -1; // -1 indicates no weather data yet
GDrawCommandImage *s_weather_icon = NULL;
char s_temperature_buffer[8];
char s_location_buffer[20];

 /*
  * Draw Functions
  */
static void weather_icon_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint origin = GPoint((bounds.size.w - gdraw_command_image_get_bounds_size(s_weather_icon).w) / 2,
                         (bounds.size.h - gdraw_command_image_get_bounds_size(s_weather_icon).h) / 2);
  gdraw_command_image_draw(ctx, s_weather_icon, origin);
}

void draw_weather_info(InfoLayer* info_layer) {
  Layer* layer = info_layer->layer;
  GRect bounds = layer_get_bounds(layer);
  int y_pos = bounds.size.h / 2 - WEATHER_ICON_SIZE / 2;
  int x_pos = (bounds.size.w / 2) - WEATHER_ICON_SIZE / 2;

  // Create weather icon layer via PDC draw command
  info_layer->custom_layer = layer_create(GRect(x_pos, y_pos, WEATHER_ICON_SIZE, WEATHER_ICON_SIZE));
  layer_set_update_proc(info_layer->custom_layer, weather_icon_layer_update_proc);

  // Add to the info layer
  layer_add_child(layer, info_layer->custom_layer);
}

void draw_temperature_info(InfoLayer* info_layer) {
  GRect bounds = info_layer->bounds;
  Layer* layer = info_layer->layer;
  int y_center = bounds.size.h / 2 - 10;
  
  // Create temperature text layer
#if defined(PBL_PLATFORM_EMERY)
  GRect temp_frame = GRect(0, y_center-16, bounds.size.w, 28);
#else
  GRect temp_frame = GRect(0, y_center-14, bounds.size.w, 24);
#endif
  info_layer->text_layer1 = text_layer_create(temp_frame);
  text_layer_set_background_color(info_layer->text_layer1, GColorClear);
  text_layer_set_text_color(info_layer->text_layer1, get_text_color());
  text_layer_set_text(info_layer->text_layer1, s_temperature_buffer);

#if defined(PBL_PLATFORM_EMERY)
  text_layer_set_font(info_layer->text_layer1, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
#else
  text_layer_set_font(info_layer->text_layer1, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
#endif

  // Create location text layer
#if defined(PBL_PLATFORM_EMERY)
  GRect location_frame = GRect(0, y_center+12, bounds.size.w, 24);
#else
  GRect location_frame = GRect(0, y_center+8, bounds.size.w, 18);
#endif
  info_layer->text_layer2 = text_layer_create(location_frame);
  text_layer_set_background_color(info_layer->text_layer2, GColorClear);
  text_layer_set_text_color(info_layer->text_layer2, get_text_color());
  text_layer_set_text(info_layer->text_layer2, s_location_buffer);
#if defined(PBL_PLATFORM_EMERY)
  text_layer_set_font(info_layer->text_layer2, fonts_get_system_font(FONT_KEY_GOTHIC_18));
#else
  text_layer_set_font(info_layer->text_layer2, fonts_get_system_font(FONT_KEY_GOTHIC_14));
#endif
  
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
    return RESOURCE_ID_IMAGE_QUESTION; // Error/No data
  }

  // Based on WMO Weather interpretation codes
  if (weather_code == 0) {
    if (s_is_day == 1) {
      return RESOURCE_ID_IMAGE_SUNNY; // Clear sky day
    } else {
      return RESOURCE_ID_IMAGE_CLEAR_NIGHT; // Clear sky night
    }
  } else if (weather_code <= 3) { // FAIL
    if (s_is_day == 1) {
      return RESOURCE_ID_IMAGE_PARTLY_CLOUDY; // Mostly cloudy day
    } else {
      return RESOURCE_ID_IMAGE_PARTLY_CLOUDY_NIGHT; // Mostly cloudy night
    }
  } else if (weather_code <= 48) {
    return RESOURCE_ID_IMAGE_CLOUDY; // Fog/deposits
  } else if (weather_code <= 57) {
    return RESOURCE_ID_IMAGE_LIGHT_RAIN; // Drizzle
  } else if (weather_code <= 67) {
    return RESOURCE_ID_IMAGE_HEAVY_RAIN; // Rain
  } else if (weather_code <= 77) {
    return RESOURCE_ID_IMAGE_LIGHT_SNOW; // Snow
  } else if (weather_code <= 82) {
    return RESOURCE_ID_IMAGE_LIGHT_RAIN; // Rain showers
  } else if (weather_code <= 86) {
    return RESOURCE_ID_IMAGE_HEAVY_SNOW; // Snow showers
  } else if (weather_code <= 99) {
    return RESOURCE_ID_IMAGE_THUNDERSTORM; // Thunderstorm
  }

  return RESOURCE_ID_IMAGE_QUESTION; // Unknown
}

void load_weather_icon() {
  uint32_t resource_id = get_weather_image_resource(s_current_weather_code);
  load_pdc_icon(&s_weather_icon, resource_id, ORIG_WEATHER_ICON_SIZE, WEATHER_ICON_SIZE);
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
    snprintf(s_temperature_buffer, sizeof(s_temperature_buffer), "---%s", unit_symbol);
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