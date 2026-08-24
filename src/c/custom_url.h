#ifndef CUSTOM_URL_H
#define CUSTOM_URL_H

#include <pebble.h>
#include "config.h"

// Extracted lines sent from the phone, each max 32 displayable characters.
// Line 1 renders in the large font, line 2 in the small font (empty = single line).
extern char s_custom_line1[33];
extern char s_custom_line2[33];
// True when all retry attempts were exhausted and no fresh data was received.
extern bool s_custom_data_stale;

void draw_custom_url_info(InfoLayer* info_layer);
bool request_custom_url_update(); // returns true if message was sent, false if outbox was busy

#endif // CUSTOM_URL_H
