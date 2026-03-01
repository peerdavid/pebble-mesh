#include "utils.h"

void load_pdc_icon(GDrawCommandImage **icon, uint32_t resource_id) {
  if (*icon) {
    gdraw_command_image_destroy(*icon);
  }
  *icon = gdraw_command_image_create_with_resource(resource_id);

  scale_pdc(*icon, ORIG_ICON_SIZE, ICON_SIZE);

  // In dark theme, invert black↔white; in light theme, PDC colors are already correct
  if (!is_light_theme()) {
    GDrawCommandList *list = gdraw_command_image_get_command_list(*icon);
    int num_commands = gdraw_command_list_get_num_commands(list);
    for (int i = 0; i < num_commands; i++) {
      GDrawCommand *cmd = gdraw_command_list_get_command(list, i);
      GColor stroke = gdraw_command_get_stroke_color(cmd);
      if (gcolor_equal(stroke, GColorBlack)) {
        gdraw_command_set_stroke_color(cmd, GColorWhite);
      } else if (gcolor_equal(stroke, GColorWhite)) {
        gdraw_command_set_stroke_color(cmd, GColorBlack);
      }
      GColor fill = gdraw_command_get_fill_color(cmd);
      if (gcolor_equal(fill, GColorBlack)) {
        gdraw_command_set_fill_color(cmd, GColorWhite);
      } else if (gcolor_equal(fill, GColorWhite)) {
        gdraw_command_set_fill_color(cmd, GColorBlack);
      }
    }
  }
}

void scale_pdc(GDrawCommandImage *image, int original_size, int new_size) {
  gdraw_command_image_set_bounds_size(image, GSize(new_size, new_size));
  GDrawCommandList *list = gdraw_command_image_get_command_list(image);
  int num_commands = gdraw_command_list_get_num_commands(list);
  for (int i = 0; i < num_commands; i++) {
    GDrawCommand *cmd = gdraw_command_list_get_command(list, i);
    int num_points = gdraw_command_get_num_points(cmd);
    for (int j = 0; j < num_points; j++) {
      GPoint pt = gdraw_command_get_point(cmd, j);
      pt.x = pt.x * new_size / original_size;
      pt.y = pt.y * new_size / original_size;
      gdraw_command_set_point(cmd, j, pt);
    }
  }
}

