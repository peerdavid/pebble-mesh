#ifndef CALENDAR_H
#define CALENDAR_H

#include <pebble.h>
#include "config.h"

extern GBitmap *s_calendar_icon_bitmap;
extern char s_day_buffer[3];

void draw_calendar_info(InfoLayer* info_layer);
void load_calendar_icon();
void update_day();

#endif // CALENDAR_H
