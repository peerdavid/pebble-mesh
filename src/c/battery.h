#ifndef BATTERY_H
#define BATTERY_H

#include <pebble.h>
#include "config.h"

// Current weather code (stored for theme changes)
int battery_level;

// Buffers for drawing
char s_battery_buffer[5];
GBitmap *s_battery_icon_bitmap;

// Function Declarations
void draw_battery_info(InfoLayer* info_layer);


#endif // BATTERY_H