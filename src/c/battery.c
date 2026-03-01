#include "battery.h"

GDrawCommandImage *s_battery_icon = NULL;
char s_battery_buffer[5];
int battery_level = 100;

static void battery_icon_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint origin = GPoint((bounds.size.w - gdraw_command_image_get_bounds_size(s_battery_icon).w) / 2,
                         (bounds.size.h - gdraw_command_image_get_bounds_size(s_battery_icon).h) / 2);
  gdraw_command_image_draw(ctx, s_battery_icon, origin);
}

void load_battery_icon() {
  load_pdc_icon(&s_battery_icon, RESOURCE_ID_IMAGE_BATTERY);
}

// Draw battery percentage in the specified info layer
void draw_battery_info(InfoLayer* info_layer) {
  GRect bounds = info_layer->bounds;
  Layer* layer = info_layer->layer;
  GRect bat_level_rect;

  int x_pos = bounds.size.w / 2 - ICON_SIZE / 2;
  int y_pos = bounds.size.h / 2 - ICON_SIZE / 2;

  // Battery icon via PDC draw command
  GRect icon_frame = GRect(x_pos, y_pos - 8, ICON_SIZE, ICON_SIZE);
  info_layer->custom_layer = layer_create(icon_frame);
  layer_set_update_proc(info_layer->custom_layer, battery_icon_layer_update_proc);

  // Draw a background rectangle for the battery level
  int full_width = ICON_SIZE - ICON_SIZE * 1/6;
  int full_height = ICON_SIZE / 2 - 2;

  bat_level_rect = GRect(x_pos, y_pos, full_width, full_height);
  info_layer->bitmap_layer_3 = bitmap_layer_create(bat_level_rect);
  bitmap_layer_set_background_color(info_layer->bitmap_layer_3, get_background_color());

  // Battery level fill rectangle
  int real_width = full_width * battery_level / 100;
  bat_level_rect = GRect(x_pos, y_pos, real_width, full_height);
  info_layer->bitmap_layer_2 = bitmap_layer_create(bat_level_rect);
  bitmap_layer_set_background_color(info_layer->bitmap_layer_2, GColorLightGray);

  // Battery percentage text
  GRect text_frame = GRect(0, y_pos + 12, bounds.size.w, 24);
  info_layer->text_layer1 = text_layer_create(text_frame);
  text_layer_set_background_color(info_layer->text_layer1, GColorClear);
  text_layer_set_text_color(info_layer->text_layer1, get_text_color());
  text_layer_set_text(info_layer->text_layer1, s_battery_buffer);
  text_layer_set_font(info_layer->text_layer1, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(info_layer->text_layer1, GTextAlignmentCenter);

  // Add to the info layer
  layer_add_child(layer, bitmap_layer_get_layer(info_layer->bitmap_layer_3));
  layer_add_child(layer, bitmap_layer_get_layer(info_layer->bitmap_layer_2));
  layer_add_child(layer, text_layer_get_layer(info_layer->text_layer1));
  layer_add_child(layer, info_layer->custom_layer);
}
