#ifndef BATTERY_H
#define BATTERY_H

#include <pebble.h>
#include "config.h"
#include "utils.h"

extern GDrawCommandImage *s_battery_icon;
extern char s_battery_buffer[5];
extern int battery_level;

void draw_battery_info(InfoLayer* info_layer);
void load_battery_icon();

#endif // BATTERY_H
