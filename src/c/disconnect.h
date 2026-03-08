#ifndef DISCONNECT_H
#define DISCONNECT_H

#include <pebble.h>
#include "config.h"

#define ORIG_DISCONNECT_ICON_SIZE 25
#define DISCONNECT_ICON_SIZE 32

extern GDrawCommandImage *s_disconnect_icon;

void draw_disconnect_info(InfoLayer* info_layer);
void load_disconnect_icon();

#endif // DISCONNECT_H
