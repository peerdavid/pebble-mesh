#include "disconnect.h"
#include "utils.h"

GDrawCommandImage *s_disconnect_icon = NULL;

static void disconnect_icon_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint origin = GPoint((bounds.size.w - gdraw_command_image_get_bounds_size(s_disconnect_icon).w) / 2,
                         (bounds.size.h - gdraw_command_image_get_bounds_size(s_disconnect_icon).h) / 2);
  gdraw_command_image_draw(ctx, s_disconnect_icon, origin);
}

void load_disconnect_icon() {
  load_pdc_icon(&s_disconnect_icon, RESOURCE_ID_IMAGE_DISCONNECT, ORIG_DISCONNECT_ICON_SIZE, DISCONNECT_ICON_SIZE);
}

void draw_disconnect_info(InfoLayer* info_layer) {
  Layer* layer = info_layer->layer;
  GRect bounds = layer_get_bounds(layer);
  int y_pos = bounds.size.h / 2 - DISCONNECT_ICON_SIZE / 2;
  int x_pos = (bounds.size.w / 2) - DISCONNECT_ICON_SIZE / 2;

  info_layer->custom_layer = layer_create(GRect(x_pos, y_pos, DISCONNECT_ICON_SIZE, DISCONNECT_ICON_SIZE));
  layer_set_update_proc(info_layer->custom_layer, disconnect_icon_layer_update_proc);

  layer_add_child(layer, info_layer->custom_layer);
}
