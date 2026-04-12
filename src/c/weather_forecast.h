
#ifndef WEATHER_FORECAST_H
#define WEATHER_FORECAST_H

#include <pebble.h>

// Weather detail bar height extends from top of screen to the upper horizontal line
#if defined(PBL_PLATFORM_EMERY)
  #define WEATHER_FORECAST_BAR_HEIGHT 77  // bounds.size.h/2 - 38 on emery (228/2 - 38 = 76)
#else
  #define WEATHER_FORECAST_BAR_HEIGHT 55  // bounds.size.h/2 - 30 on standard (168/2 - 30 = 54)
#endif

#define WEATHER_FORECAST_ANIM_DURATION_MS 1000

#define NUM_FORECAST_SLOTS 3
#define NUM_HOURLY_POINTS 24

// Forecast data for now, +1d, +2d
typedef struct {
  int temperature;
  int condition_code;
} ForecastSlot;

extern ForecastSlot s_forecast[NUM_FORECAST_SLOTS];

// Hourly data for bottom bar graph (12 hours)
extern int s_hourly_temps[NUM_HOURLY_POINTS];
extern int s_hourly_precip[NUM_HOURLY_POINTS];
extern bool s_hourly_data_available;

// Initialize the weather detail layers and add them to the window
void weather_forecast_init(Layer *window_layer, GRect bounds);

// Destroy the weather detail layers
void weather_forecast_deinit();

// Toggle the weather detail screen (called on flick event)
void weather_forecast_show();

// Check if weather detail screen is currently visible
bool weather_forecast_is_visible();

// Update forecast icons after data changes
void weather_forecast_update_icons();

// Parse comma-separated hourly data strings
void weather_forecast_parse_hourly_temps(const char *csv);
void weather_forecast_parse_hourly_precip(const char *csv);

// Storage
void weather_forecast_save_data();
void weather_forecast_load_data();

#endif // WEATHER_FORECAST_H
