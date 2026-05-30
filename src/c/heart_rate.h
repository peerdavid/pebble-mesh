#ifndef HEART_RATE_H
#define HEART_RATE_H

#include <pebble.h>
#include "config.h"
#include "utils.h"

#define ORIG_HEART_ICON_SIZE 25
#if defined(PBL_PLATFORM_EMERY)
  #define HEART_ICON_SIZE 44
#else
  #define HEART_ICON_SIZE 32
#endif

extern GDrawCommandImage *s_heart_icon;
extern char s_heart_buffer[8];
extern int heart_rate_bpm;

void draw_heart_rate_info(InfoLayer* info_layer);
void update_heart_rate();
void load_heart_icon();

#endif // HEART_RATE_H
