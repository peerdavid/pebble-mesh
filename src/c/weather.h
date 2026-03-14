#ifndef WEATHER_H
#define WEATHER_H

#include <pebble.h>
#include "config.h"


/*
 * Definitions
 */

#define ORIG_WEATHER_ICON_SIZE 50
#if defined(PBL_PLATFORM_EMERY)
  #define WEATHER_ICON_SIZE 50
#else
  #define WEATHER_ICON_SIZE 36
#endif

extern int s_current_weather_code;
extern GDrawCommandImage *s_weather_icon;
extern char s_temperature_buffer[8];
extern char s_location_buffer[20];

/*
 * Function Declarations
 */
void draw_weather_info(InfoLayer* info_layer);
void draw_temperature_info(InfoLayer* info_layer);
uint32_t get_weather_image_resource(int weather_code);
void load_weather_icon();
void request_weather_update();
void save_weather_to_storage();
void load_weather_from_storage();

#endif // WEATHER_H