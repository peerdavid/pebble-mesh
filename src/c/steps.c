#include "steps.h"

/*
 * Global Variables
 */
GDrawCommandImage *s_step_icon = NULL;
char s_step_buffer[20];
int step_count = 0;

/*
 * Function Implementations
 */

static void step_icon_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint origin = GPoint((bounds.size.w - gdraw_command_image_get_bounds_size(s_step_icon).w) / 2,
                         (bounds.size.h - gdraw_command_image_get_bounds_size(s_step_icon).h) / 2);
  gdraw_command_image_draw(ctx, s_step_icon, origin);
}

void load_step_icon() {
  load_pdc_icon(&s_step_icon, RESOURCE_ID_IMAGE_STEP);
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

  int y_pos = bounds.size.h / 4 + 1;
  int x_pos = bounds.size.w / 2 - ICON_SIZE / 2;
  icon_frame = GRect(x_pos, y_pos-ICON_SIZE / 2 + 2, ICON_SIZE, ICON_SIZE);
  text_frame = GRect(0, y_pos+6, bounds.size.w, 28);
  
  // Create step icon layer via PDC draw command
  info_layer->custom_layer = layer_create(icon_frame);
  layer_set_update_proc(info_layer->custom_layer, step_icon_layer_update_proc);
  
  info_layer->text_layer1 = text_layer_create(text_frame);
  text_layer_set_background_color(info_layer->text_layer1, GColorClear);
  text_layer_set_text_color(info_layer->text_layer1, get_text_color());
  text_layer_set_text(info_layer->text_layer1, s_step_buffer);
  text_layer_set_font(info_layer->text_layer1, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(info_layer->text_layer1, GTextAlignmentCenter);
  
  int full_width = ICON_SIZE-1;
  int full_height = ICON_SIZE / 2;

  // Full background rectangle layer
  step_count_rect = GRect(x_pos, y_pos-ICON_SIZE / 6, full_width, full_height);
  info_layer->bitmap_layer_3 = bitmap_layer_create(step_count_rect);
  bitmap_layer_set_background_color(info_layer->bitmap_layer_3, get_background_color());

  // Create small rectangle layer with text color background
  int steps = step_count > s_step_goal ? s_step_goal : step_count;
  int real_width = (steps * full_width) / s_step_goal;
  step_count_rect = GRect(x_pos, y_pos-ICON_SIZE / 6, real_width, full_height);
  info_layer->bitmap_layer_2 = bitmap_layer_create(step_count_rect);
  bitmap_layer_set_background_color(info_layer->bitmap_layer_2, GColorLightGray);
  
  layer_add_child(layer, bitmap_layer_get_layer(info_layer->bitmap_layer_3));
  layer_add_child(layer, bitmap_layer_get_layer(info_layer->bitmap_layer_2));
  layer_add_child(layer, text_layer_get_layer(info_layer->text_layer1));
  layer_add_child(layer, info_layer->custom_layer);
}