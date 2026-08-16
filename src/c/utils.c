#include "utils.h"

void load_pdc_icon(GDrawCommandImage **icon, uint32_t resource_id, int orig_icon_size, int target_icon_size) {
  if (*icon) {
    gdraw_command_image_destroy(*icon);
  }
  *icon = gdraw_command_image_create_with_resource(resource_id);

  scale_pdc(*icon, orig_icon_size, target_icon_size);

  // The PDCs are authored for black-on-white: black elements are the icon's
  // foreground, white elements are background filler. Remap them to the
  // current text and background colors so the filler blends into any
  // configured background color (on default themes this is a no-op in light
  // mode and the old black↔white swap in dark mode).
  GColor fg = get_text_color();
  // On a dithered gray background (BW platforms), color the filler
  // GColorLightGray as well: the firmware dithers it with the same
  // screen-aligned pattern, so it blends into the background
  GColor bg = is_bw_gray_background() ? GColorLightGray : get_background_color();
  GDrawCommandList *list = gdraw_command_image_get_command_list(*icon);
  int num_commands = gdraw_command_list_get_num_commands(list);
  for (int i = 0; i < num_commands; i++) {
    GDrawCommand *cmd = gdraw_command_list_get_command(list, i);
    GColor stroke = gdraw_command_get_stroke_color(cmd);
    if (gcolor_equal(stroke, GColorBlack)) {
      gdraw_command_set_stroke_color(cmd, fg);
    } else if (gcolor_equal(stroke, GColorWhite)) {
      gdraw_command_set_stroke_color(cmd, bg);
    }
    GColor fill = gdraw_command_get_fill_color(cmd);
    if (gcolor_equal(fill, GColorBlack)) {
      gdraw_command_set_fill_color(cmd, fg);
    } else if (gcolor_equal(fill, GColorWhite)) {
      gdraw_command_set_fill_color(cmd, bg);
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

