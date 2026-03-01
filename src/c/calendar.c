#include "calendar.h"

GDrawCommandImage *s_calendar_icon = NULL;
char s_day_buffer[3];

static void calendar_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint origin = GPoint((bounds.size.w - gdraw_command_image_get_bounds_size(s_calendar_icon).w) / 2,
                         (bounds.size.h - gdraw_command_image_get_bounds_size(s_calendar_icon).h) / 2);
  gdraw_command_image_draw(ctx, s_calendar_icon, origin);
}


void load_calendar_icon() {
  load_pdc_icon(&s_calendar_icon, RESOURCE_ID_IMAGE_CALENDAR);
}

void update_day() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  snprintf(s_day_buffer, sizeof(s_day_buffer), "%d", tick_time->tm_mday);
}

void draw_calendar_info(InfoLayer* info_layer) {
  GRect bounds = info_layer->bounds;
  Layer* layer = info_layer->layer;

  int x_pos = bounds.size.w / 2 - ICON_SIZE / 2;
  int y_pos = bounds.size.h / 2 - ICON_SIZE / 2;
  int left_shift = 0;

  // 2 digit numbers starting with '1' seem to be shifted slightly to the right, so we apply a small left shift to center them better
  if (s_day_buffer[0] == '1' && s_day_buffer[1] != '\0') {
    left_shift = -1;
  }

  // Calendar icon via PDC draw command
  GRect icon_frame = GRect(x_pos, y_pos, ICON_SIZE, ICON_SIZE);
  info_layer->custom_layer = layer_create(icon_frame);
  layer_set_update_proc(info_layer->custom_layer, calendar_layer_update_proc);

  // Day number text drawn over the icon
  GRect text_frame = GRect(x_pos + left_shift, y_pos+2, ICON_SIZE, ICON_SIZE);
  info_layer->text_layer1 = text_layer_create(text_frame);
  text_layer_set_background_color(info_layer->text_layer1, GColorClear);
  text_layer_set_text_color(info_layer->text_layer1, get_text_color());
  text_layer_set_text(info_layer->text_layer1, s_day_buffer);
  text_layer_set_font(info_layer->text_layer1, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(info_layer->text_layer1, GTextAlignmentCenter);

  layer_add_child(layer, info_layer->custom_layer);
  layer_add_child(layer, text_layer_get_layer(info_layer->text_layer1));
}
