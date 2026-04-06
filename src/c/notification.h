
#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <pebble.h>

// Weather detail bar height extends from top of screen to the upper horizontal line
#if defined(PBL_PLATFORM_EMERY)
  #define NOTIFICATION_BAR_HEIGHT 77  // bounds.size.h/2 - 38 on emery (228/2 - 38 = 76)
#else
  #define NOTIFICATION_BAR_HEIGHT 55  // bounds.size.h/2 - 30 on standard (168/2 - 30 = 54)
#endif

#define NOTIFICATION_FADE_FRAMES 10
#define NOTIFICATION_FADE_RATE_MS 40
#define NOTIFICATION_DISPLAY_MS 10000

#define NUM_FORECAST_SLOTS 3
#define NUM_HOURLY_POINTS 24

// Forecast data for +3h, +6h, +9h
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
void notification_init(Layer *window_layer, GRect bounds);

// Destroy the weather detail layers
void notification_deinit();

// Toggle the weather detail screen (called on flick event)
void notification_show();

// Check if weather detail screen is currently visible
bool notification_is_visible();

// Update forecast icons after data changes
void notification_update_forecast_icons();

// Parse comma-separated hourly data strings
void notification_parse_hourly_temps(const char *csv);
void notification_parse_hourly_precip(const char *csv);

#endif // NOTIFICATION_H
