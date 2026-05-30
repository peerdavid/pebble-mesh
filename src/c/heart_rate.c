#include "heart_rate.h"

GDrawCommandImage *s_heart_icon = NULL;
char s_heart_buffer[8] = "--";
int heart_rate_bpm = 0;

static void heart_icon_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint origin = GPoint((bounds.size.w - gdraw_command_image_get_bounds_size(s_heart_icon).w) / 2,
                         (bounds.size.h - gdraw_command_image_get_bounds_size(s_heart_icon).h) / 2);
  gdraw_command_image_draw(ctx, s_heart_icon, origin);
}

void load_heart_icon() {
  load_pdc_icon(&s_heart_icon, RESOURCE_ID_IMAGE_HEART, ORIG_HEART_ICON_SIZE, HEART_ICON_SIZE);
}

void update_heart_rate() {
  HealthValue v = health_service_peek_current_value(HealthMetricHeartRateBPM);
  heart_rate_bpm = (int)v;
  if (heart_rate_bpm > 0) {
    snprintf(s_heart_buffer, sizeof(s_heart_buffer), "%d", heart_rate_bpm);
  } else {
    snprintf(s_heart_buffer, sizeof(s_heart_buffer), "--");
  }
}

void draw_heart_rate_info(InfoLayer* info_layer) {
  GRect bounds = info_layer->bounds;
  Layer* layer = info_layer->layer;

  int x_pos = bounds.size.w / 2 - HEART_ICON_SIZE / 2;
  int y_pos = bounds.size.h / 2 - HEART_ICON_SIZE / 2;
  int y_offset = 0;

#if defined(PBL_PLATFORM_EMERY)
  y_offset = 3;
#endif

  // Heart icon via PDC draw command
  GRect icon_frame = GRect(x_pos, y_pos - 8 - y_offset, HEART_ICON_SIZE, HEART_ICON_SIZE);
  info_layer->custom_layer = layer_create(icon_frame);
  layer_set_update_proc(info_layer->custom_layer, heart_icon_layer_update_proc);

  // BPM text below the icon
#if defined(PBL_PLATFORM_EMERY)
  GRect text_frame = GRect(0, y_pos + 16, bounds.size.w, 28);
#else
  GRect text_frame = GRect(0, y_pos + 12, bounds.size.w, 24);
#endif
  info_layer->text_layer1 = text_layer_create(text_frame);
  text_layer_set_background_color(info_layer->text_layer1, GColorClear);
  text_layer_set_text_color(info_layer->text_layer1, get_text_color());
  text_layer_set_text(info_layer->text_layer1, s_heart_buffer);
#if defined(PBL_PLATFORM_EMERY)
  text_layer_set_font(info_layer->text_layer1, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
#else
  text_layer_set_font(info_layer->text_layer1, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
#endif
  text_layer_set_text_alignment(info_layer->text_layer1, GTextAlignmentCenter);

  layer_add_child(layer, text_layer_get_layer(info_layer->text_layer1));
  layer_add_child(layer, info_layer->custom_layer);
}
