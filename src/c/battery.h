#ifndef BATTERY_H
#define BATTERY_H

#include <pebble.h>
#include "config.h"
#include "utils.h"

#define ORIG_BATTERY_ICON_SIZE 25
#if defined(PBL_PLATFORM_EMERY)
  #define BATTERY_ICON_SIZE 44
#else
  #define BATTERY_ICON_SIZE 32
#endif

extern GDrawCommandImage *s_battery_icon;
extern char s_battery_buffer[5];
extern int battery_level;

void draw_battery_info(InfoLayer* info_layer);
void load_battery_icon();

#endif // BATTERY_H
