#include "steps.h"

/*
 * Global Variables
 */
GBitmap *s_step_icon_bitmap = NULL;
char s_step_buffer[20];
int step_count = 0;

/*
 * Function Implementations
 */

void load_step_icon() {
  if (s_step_icon_bitmap) {
    gbitmap_destroy(s_step_icon_bitmap);
  }
  uint32_t step_resource_id = s_color_theme == 1 ? RESOURCE_ID_IMAGE_STEP_LIGHT : RESOURCE_ID_IMAGE_STEP_DARK;
  s_step_icon_bitmap = gbitmap_create_with_resource(step_resource_id);
}

void update_step_count() {
  step_count = (int)health_service_sum_today(HealthMetricStepCount);
  if (step_count < 1000) {
    snprintf(s_step_buffer, sizeof(s_step_buffer), "%d", step_count);
  } else {
    snprintf(s_step_buffer, sizeof(s_step_buffer), "%d.%01dk", step_count / 1000, (step_count % 1000) / 100);
  }
}

void draw_steps_info(InfoLayer* info_layer) {
  GRect bounds = info_layer->bounds;
  Layer* layer = info_layer->layer;
  
  GRect icon_frame;
  GRect text_frame;
  GRect step_count_rect;

  int y_pos = bounds.size.h / 4;
  int x_pos = bounds.size.w / 2 - 16;
  icon_frame = GRect(x_pos, y_pos-12, 32, 32);
  text_frame = GRect(0, y_pos+6, bounds.size.w, 28);
  
  // Create step icon layer
  info_layer->bitmap_layer_1 = bitmap_layer_create(icon_frame);
  bitmap_layer_set_bitmap(info_layer->bitmap_layer_1, s_step_icon_bitmap);
  bitmap_layer_set_compositing_mode(info_layer->bitmap_layer_1, GCompOpSet);
  
  info_layer->text_layer1 = text_layer_create(text_frame);
  text_layer_set_background_color(info_layer->text_layer1, GColorClear);
  text_layer_set_text_color(info_layer->text_layer1, get_text_color());
  text_layer_set_text(info_layer->text_layer1, s_step_buffer);
  text_layer_set_font(info_layer->text_layer1, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(info_layer->text_layer1, GTextAlignmentCenter);
  
  // Draw a background color rectangle
  int full_width = 28;
  int full_height = 11;
  step_count_rect = GRect(x_pos + 2, y_pos-2, full_width, full_height);
  info_layer->bitmap_layer_3 = bitmap_layer_create(step_count_rect);
  bitmap_layer_set_background_color(info_layer->bitmap_layer_3, get_background_color());

  // Create small rectangle layer with text color background
  int steps = step_count > 10000 ? 10000 : step_count;
  int real_width = (steps * full_width) / 10000;
  step_count_rect = GRect(x_pos + 2, y_pos-2, real_width, full_height);
  info_layer->bitmap_layer_2 = bitmap_layer_create(step_count_rect);
  bitmap_layer_set_background_color(info_layer->bitmap_layer_2, GColorLightGray);
  
  // Add to the info layer
  layer_add_child(layer, bitmap_layer_get_layer(info_layer->bitmap_layer_3));
  layer_add_child(layer, bitmap_layer_get_layer(info_layer->bitmap_layer_2));
  layer_add_child(layer, bitmap_layer_get_layer(info_layer->bitmap_layer_1));
  layer_add_child(layer, text_layer_get_layer(info_layer->text_layer1));
}