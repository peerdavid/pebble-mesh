#include "calendar.h"

GBitmap *s_calendar_icon_bitmap = NULL;
char s_day_buffer[3];

void load_calendar_icon() {
  if (s_calendar_icon_bitmap) {
    gbitmap_destroy(s_calendar_icon_bitmap);
  }
  uint32_t resource_id = is_light_theme() ? RESOURCE_ID_IMAGE_CALENDAR_LIGHT : RESOURCE_ID_IMAGE_CALENDAR_DARK;
  s_calendar_icon_bitmap = gbitmap_create_with_resource(resource_id);
}

void update_day() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  snprintf(s_day_buffer, sizeof(s_day_buffer), "%d", tick_time->tm_mday);
}

void draw_calendar_info(InfoLayer* info_layer) {
  GRect bounds = info_layer->bounds;
  Layer* layer = info_layer->layer;

  int x_pos = bounds.size.w / 2 - 16;
  int y_pos = bounds.size.h / 2 - 16;
  int left_shift = 0;

  // 2 digit numbers starting with '1' seem to be shifted slightly to the right, so we apply a small left shift to center them better
  // If the day is 2 digits and starts with a '1', shift the icon slightly to the left to center it better
  if (s_day_buffer[0] == '1' && s_day_buffer[1] != '\0') {
    left_shift = -1;
  }

  // Calendar icon
  GRect icon_frame = GRect(x_pos, y_pos, 32, 32);
  info_layer->bitmap_layer_1 = bitmap_layer_create(icon_frame);
  bitmap_layer_set_bitmap(info_layer->bitmap_layer_1, s_calendar_icon_bitmap);
  bitmap_layer_set_compositing_mode(info_layer->bitmap_layer_1, GCompOpSet);

  // Day number text drawn over the icon
  GRect text_frame = GRect(x_pos + left_shift, y_pos, 32, 32);
  info_layer->text_layer1 = text_layer_create(text_frame);
  text_layer_set_background_color(info_layer->text_layer1, GColorClear);
  text_layer_set_text_color(info_layer->text_layer1, get_text_color());
  text_layer_set_text(info_layer->text_layer1, s_day_buffer);
  text_layer_set_font(info_layer->text_layer1, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(info_layer->text_layer1, GTextAlignmentCenter);

  layer_add_child(layer, bitmap_layer_get_layer(info_layer->bitmap_layer_1));
  layer_add_child(layer, text_layer_get_layer(info_layer->text_layer1));
}
