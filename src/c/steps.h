#ifndef STEPS_H
#define STEPS_H

#include <pebble.h>
#include "config.h"

/*
 * Definitions
 */
extern GBitmap *s_step_icon_bitmap;
extern char s_step_buffer[20];
extern int step_count;

/*
 * Function Declarations
 */
void draw_steps_info(InfoLayer* info_layer);
void update_step_count();
void load_step_icon();

#endif // STEPS_H