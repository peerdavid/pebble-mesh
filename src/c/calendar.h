#ifndef CALENDAR_H
#define CALENDAR_H

#include <pebble.h>
#include "config.h"
#include "utils.h"

#define ORIG_CAL_ICON_SIZE 25
#if defined(PBL_PLATFORM_EMERY)
  #define CAL_ICON_SIZE 44
#else
  #define CAL_ICON_SIZE 32
#endif

extern GDrawCommandImage *s_calendar_icon;
extern char s_day_buffer[3];

void draw_calendar_info(InfoLayer* info_layer);
void load_calendar_icon();
void update_day();

#endif // CALENDAR_H
