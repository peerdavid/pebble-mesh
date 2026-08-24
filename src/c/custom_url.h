#ifndef CUSTOM_URL_H
#define CUSTOM_URL_H

#include <pebble.h>
#include "config.h"

// Buffer holding the last value fetched from the user-configured URL.
// Max 32 displayable characters (plus null terminator).
extern char s_custom_data[33];
// True when all retry attempts were exhausted and no fresh data was received.
extern bool s_custom_data_stale;

void draw_custom_url_info(InfoLayer* info_layer);
bool request_custom_url_update(); // returns true if message was sent, false if outbox was busy

#endif // CUSTOM_URL_H
