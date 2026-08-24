#include "custom_url.h"

char s_custom_line1[33] = "N/A";
char s_custom_line2[33] = "";
bool s_custom_data_stale = false;

bool request_custom_url_update() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Requesting custom URL update");

  DictionaryIterator *iter;
  AppMessageResult result = app_message_outbox_begin(&iter);

  if (result != APP_MSG_OK || iter == NULL) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Outbox busy (result=%d), custom URL request not sent", (int)result);
    return false;
  }

  dict_write_uint8(iter, MESSAGE_KEY_CUSTOM_URL_REQUEST, 1);
  app_message_outbox_send();
  return true;
}

void draw_custom_url_info(InfoLayer* info_layer) {
  GRect bounds = info_layer->bounds;
  Layer* layer = info_layer->layer;
  GColor stale_color = is_dark_theme() ? GColorYellow : GColorRed;
  GColor text_color = s_custom_data_stale ? PBL_IF_COLOR_ELSE(stale_color, get_text_color()) : get_text_color();

  bool two_lines = (s_custom_line2[0] != '\0');
  int y_center = bounds.size.h / 2 - 10;

  if (two_lines) {
    // Line 1 - bigger font (matches temperature line)
#if defined(PBL_PLATFORM_EMERY)
    GRect line1_frame = GRect(0, y_center - 16, bounds.size.w, 28);
#else
    GRect line1_frame = GRect(0, y_center - 14, bounds.size.w, 24);
#endif
    info_layer->text_layer1 = text_layer_create(line1_frame);
    text_layer_set_background_color(info_layer->text_layer1, GColorClear);
    text_layer_set_text_color(info_layer->text_layer1, text_color);
    text_layer_set_text(info_layer->text_layer1, s_custom_line1);
#if defined(PBL_PLATFORM_EMERY)
    text_layer_set_font(info_layer->text_layer1, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
#else
    text_layer_set_font(info_layer->text_layer1, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
#endif
    text_layer_set_text_alignment(info_layer->text_layer1, GTextAlignmentCenter);
    layer_add_child(layer, text_layer_get_layer(info_layer->text_layer1));

    // Line 2 - smaller font (matches location line)
#if defined(PBL_PLATFORM_EMERY)
    GRect line2_frame = GRect(0, y_center + 12, bounds.size.w, 24);
#else
    GRect line2_frame = GRect(0, y_center + 8, bounds.size.w, 18);
#endif
    info_layer->text_layer2 = text_layer_create(line2_frame);
    text_layer_set_background_color(info_layer->text_layer2, GColorClear);
    text_layer_set_text_color(info_layer->text_layer2, text_color);
    text_layer_set_text(info_layer->text_layer2, s_custom_line2);
#if defined(PBL_PLATFORM_EMERY)
    text_layer_set_font(info_layer->text_layer2, fonts_get_system_font(FONT_KEY_GOTHIC_18));
#else
    text_layer_set_font(info_layer->text_layer2, fonts_get_system_font(FONT_KEY_GOTHIC_14));
#endif
    text_layer_set_text_alignment(info_layer->text_layer2, GTextAlignmentCenter);
    layer_add_child(layer, text_layer_get_layer(info_layer->text_layer2));

  } else {
    // Single line - same large font as line 1 of the two-line layout
    info_layer->text_layer1 = text_layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
    text_layer_set_background_color(info_layer->text_layer1, GColorClear);
    text_layer_set_text_color(info_layer->text_layer1, text_color);
    text_layer_set_text(info_layer->text_layer1, s_custom_line1);
#if defined(PBL_PLATFORM_EMERY)
    text_layer_set_font(info_layer->text_layer1, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
#else
    text_layer_set_font(info_layer->text_layer1, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
#endif
    text_layer_set_text_alignment(info_layer->text_layer1, GTextAlignmentCenter);
    text_layer_set_overflow_mode(info_layer->text_layer1, GTextOverflowModeWordWrap);
    layer_add_child(layer, text_layer_get_layer(info_layer->text_layer1));
  }
}
