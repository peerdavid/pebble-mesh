#include "battery.h"


int battery_level = 100;

char s_battery_buffer[5];
GBitmap *s_battery_icon_bitmap;


// Draw battery percentage in the specified info layer
void draw_battery_info(InfoLayer* info_layer) {
  GRect bounds = info_layer->bounds;
  Layer* layer = info_layer->layer;
  
  GRect icon_frame;
  GRect text_frame;
  GRect bat_level_rect;

  int x_pos = bounds.size.w / 2 - 16;
  int y_pos = bounds.size.h / 2 - 16;
  
  icon_frame = GRect(x_pos, y_pos-8, 32, 32);
  text_frame = GRect(0, y_pos+12, bounds.size.w, 24);
  
  // Create battery icon layer
  info_layer->bitmap_layer_1 = bitmap_layer_create(icon_frame);
  bitmap_layer_set_bitmap(info_layer->bitmap_layer_1, s_battery_icon_bitmap);
  bitmap_layer_set_compositing_mode(info_layer->bitmap_layer_1, GCompOpSet);
  
  // Create small rectangle layer with text color background
  int real_width = (battery_level * 19) / 100;
  bat_level_rect = GRect(x_pos + 5, y_pos+3, real_width, 10);
  info_layer->bitmap_layer_2 = bitmap_layer_create(bat_level_rect);
  bitmap_layer_set_background_color(info_layer->bitmap_layer_2, GColorLightGray);
  
  info_layer->text_layer1 = text_layer_create(text_frame);
  text_layer_set_background_color(info_layer->text_layer1, GColorClear);
  text_layer_set_text_color(info_layer->text_layer1, get_text_color());
  text_layer_set_text(info_layer->text_layer1, s_battery_buffer);
  text_layer_set_font(info_layer->text_layer1, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(info_layer->text_layer1, GTextAlignmentCenter);

  // Add to the info layer
  layer_add_child(layer, bitmap_layer_get_layer(info_layer->bitmap_layer_1));
  layer_add_child(layer, bitmap_layer_get_layer(info_layer->bitmap_layer_2));
  layer_add_child(layer, text_layer_get_layer(info_layer->text_layer1));
}