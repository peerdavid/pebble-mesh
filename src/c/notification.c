
#include <pebble.h>
#include "notification.h"
#include "config.h"
#include "weather.h"
#include "utils.h"

static Layer *s_notification_top_layer = NULL;
static Layer *s_notification_bottom_layer = NULL;
static AppTimer *s_fade_timer = NULL;
static AppTimer *s_hide_timer = NULL;

// Fade state: 0 = hidden, NOTIFICATION_FADE_FRAMES = fully visible
static int s_fade_progress = 0;
static bool s_fading_in = false;
static bool s_fading_out = false;

// Forecast data
ForecastSlot s_forecast[NUM_FORECAST_SLOTS] = {
  { .temperature = 0, .condition_code = -1 },
  { .temperature = 0, .condition_code = -1 },
  { .temperature = 0, .condition_code = -1 }
};

// Forecast icons (PDC images)
static GDrawCommandImage *s_forecast_icons[NUM_FORECAST_SLOTS] = { NULL, NULL, NULL };

// Temperature label buffers
static char s_forecast_temp_buffers[NUM_FORECAST_SLOTS][10];

// Forecast labels: now, tomorrow, day after
static const char *s_hour_labels[NUM_FORECAST_SLOTS] = { "Now", "+1d", "+2d" };

// Hourly graph data (12 hours)
int s_hourly_temps[NUM_HOURLY_POINTS] = {0};
int s_hourly_precip[NUM_HOURLY_POINTS] = {0};
bool s_hourly_data_available = false;

#define FORECAST_ICON_ORIG_SIZE 50
#if defined(PBL_PLATFORM_EMERY)
  #define FORECAST_ICON_SIZE 30
#else
  #define FORECAST_ICON_SIZE 20
#endif

void notification_update_forecast_icons() {
  for (int i = 0; i < NUM_FORECAST_SLOTS; i++) {
    uint32_t resource_id = get_weather_image_resource(s_forecast[i].condition_code);
    load_pdc_icon(&s_forecast_icons[i], resource_id, FORECAST_ICON_ORIG_SIZE, FORECAST_ICON_SIZE);

    const char *unit = s_temperature_unit == 1 ? "F" : "C";
    snprintf(s_forecast_temp_buffers[i], sizeof(s_forecast_temp_buffers[i]),
             "%d°%s", s_forecast[i].temperature, unit);
  }
}

// Parse comma-separated integers into an array, returns count parsed
static int parse_csv_ints(const char *csv, int *out, int max_count) {
  int count = 0;
  const char *p = csv;
  while (*p && count < max_count) {
    // Parse sign and digits
    int neg = 0;
    if (*p == '-') { neg = 1; p++; }
    int val = 0;
    while (*p >= '0' && *p <= '9') {
      val = val * 10 + (*p - '0');
      p++;
    }
    out[count++] = neg ? -val : val;
    if (*p == ',') p++;
  }
  return count;
}

void notification_parse_hourly_temps(const char *csv) {
  if (csv && csv[0]) {
    parse_csv_ints(csv, s_hourly_temps, NUM_HOURLY_POINTS);
    s_hourly_data_available = true;
  }
}

void notification_parse_hourly_precip(const char *csv) {
  if (csv && csv[0]) {
    parse_csv_ints(csv, s_hourly_precip, NUM_HOURLY_POINTS);
  }
}

// Draw text with a 1px outline (background color border, then foreground in center)
static void draw_outlined_text(GContext *ctx, const char *text, GFont font,
                               GRect rect, GTextOverflowMode overflow,
                               GTextAlignment alignment, GColor fg_color) {
  GColor outline_color = get_background_color();
  graphics_context_set_text_color(ctx, outline_color);
  // Draw outline: 4 offsets
  graphics_draw_text(ctx, text, font, GRect(rect.origin.x - 1, rect.origin.y, rect.size.w, rect.size.h), overflow, alignment, NULL);
  graphics_draw_text(ctx, text, font, GRect(rect.origin.x + 1, rect.origin.y, rect.size.w, rect.size.h), overflow, alignment, NULL);
  graphics_draw_text(ctx, text, font, GRect(rect.origin.x, rect.origin.y - 1, rect.size.w, rect.size.h), overflow, alignment, NULL);
  graphics_draw_text(ctx, text, font, GRect(rect.origin.x, rect.origin.y + 1, rect.size.w, rect.size.h), overflow, alignment, NULL);
  // Draw center text
  graphics_context_set_text_color(ctx, fg_color);
  graphics_draw_text(ctx, text, font, rect, overflow, alignment, NULL);
}

// Draw the mesh dot grid pattern within a rect area
#define MESH_GRID_SIZE 8
static void draw_mesh_pattern(GContext *ctx, GRect area) {
  GColor frame_color = get_text_color();
  graphics_context_set_stroke_color(ctx, frame_color);
  graphics_context_set_stroke_width(ctx, 1);
  for (int y = area.origin.y; y < area.origin.y + area.size.h; y += MESH_GRID_SIZE) {
    for (int x = area.origin.x; x < area.origin.x + area.size.w; x += MESH_GRID_SIZE) {
      graphics_draw_line(ctx, GPoint(x, y), GPoint(x, y));
    }
  }
}

static void draw_notification_top(Layer *layer, GContext *ctx) {
  if (s_fade_progress <= 0) {
    return;
  }

  GRect bounds = layer_get_bounds(layer);
  float alpha = (float)s_fade_progress / NOTIFICATION_FADE_FRAMES;

  // Background: grows down from top
  graphics_context_set_fill_color(ctx, get_background_color());
  int visible_height = (int)(bounds.size.h * alpha);
  graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, visible_height), 0, GCornerNone);

  // Draw mesh grid pattern over the visible area
  draw_mesh_pattern(ctx, GRect(0, 0, bounds.size.w, visible_height));

  // Draw separator line at the bottom edge of the visible area
  GColor text_color = get_text_color();
  graphics_context_set_stroke_color(ctx, text_color);
  graphics_context_set_stroke_width(ctx, 3);
  int line_x_start = 0;
  int line_x_end = bounds.size.w;
  if (is_light_theme()) {
    int max_line_length = (int)(bounds.size.w * 0.8f);
    line_x_start = (bounds.size.w - max_line_length) / 2;
    line_x_end = line_x_start + max_line_length;
  }
  graphics_draw_line(ctx, GPoint(line_x_start, visible_height - 2), GPoint(line_x_end, visible_height - 2));

  // Only draw content once fully visible
  if (s_fade_progress < NOTIFICATION_FADE_FRAMES) {
    return;
  }

  // Layout: 3 columns, each with hour label + icon on top + temperature below
  int col_width = bounds.size.w / NUM_FORECAST_SLOTS;

#if defined(PBL_PLATFORM_EMERY)
  int label_y = 2;
  int icon_y = 18;
  int temp_y = icon_y + FORECAST_ICON_SIZE + 1;
  GFont temp_font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  GFont label_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
#else
  int label_y = -4;
  int icon_y = 14;
  int temp_y = icon_y + FORECAST_ICON_SIZE;
  GFont temp_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  GFont label_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
#endif

  for (int i = 0; i < NUM_FORECAST_SLOTS; i++) {
    int col_x = i * col_width;
    int center_x = col_x + col_width / 2;

    // Draw hour label (+3h, +6h, +9h)
    draw_outlined_text(ctx, s_hour_labels[i], label_font,
                       GRect(col_x, label_y, col_width, 16),
                       GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentCenter, text_color);

    // Draw forecast icon
    if (s_forecast_icons[i]) {
      GSize icon_size = gdraw_command_image_get_bounds_size(s_forecast_icons[i]);
      GPoint icon_origin = GPoint(center_x - icon_size.w / 2, icon_y);
      gdraw_command_image_draw(ctx, s_forecast_icons[i], icon_origin);
    }

    // Draw temperature
    draw_outlined_text(ctx, s_forecast_temp_buffers[i], temp_font,
                       GRect(col_x, temp_y, col_width, 20),
                       GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentCenter, text_color);
  }
}

static void draw_notification_bottom(Layer *layer, GContext *ctx) {
  if (s_fade_progress <= 0) {
    return;
  }

  GRect bounds = layer_get_bounds(layer);
  float alpha = (float)s_fade_progress / NOTIFICATION_FADE_FRAMES;

  // Background: grows up from bottom
  graphics_context_set_fill_color(ctx, get_background_color());
  int visible_height = (int)(bounds.size.h * alpha);
  int y_start = bounds.size.h - visible_height;
  graphics_fill_rect(ctx, GRect(0, y_start, bounds.size.w, visible_height), 0, GCornerNone);

  // Draw mesh grid pattern over the visible area
  draw_mesh_pattern(ctx, GRect(0, y_start, bounds.size.w, visible_height));

  // Draw separator line at the top edge of the visible area
  GColor text_color = get_text_color();
  graphics_context_set_stroke_color(ctx, text_color);
  graphics_context_set_stroke_width(ctx, 3);
  int line_x_start = 0;
  int line_x_end = bounds.size.w;
  if (is_light_theme()) {
    int max_line_length = (int)(bounds.size.w * 0.8f);
    line_x_start = (bounds.size.w - max_line_length) / 2;
    line_x_end = line_x_start + max_line_length;
  }
  graphics_draw_line(ctx, GPoint(line_x_start, y_start+1), GPoint(line_x_end, y_start+1));

  if (s_fade_progress < NOTIFICATION_FADE_FRAMES) {
    return;
  }

  if (!s_hourly_data_available) {
    graphics_context_set_text_color(ctx, text_color);
    GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
    graphics_draw_text(ctx, "No forecast data",
                       font,
                       GRect(4, bounds.size.h / 2 - 10, bounds.size.w - 8, 20),
                       GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentCenter, NULL);
    return;
  }

  // Graph area with margins
  const int margin_left = 4;
  const int margin_right = 4;
  const int margin_top = 8;
  const int margin_bottom = 2;
  const int graph_x = margin_left;
  const int graph_w = bounds.size.w - margin_left - margin_right;
  const int graph_y = margin_top;
  const int graph_h = bounds.size.h - margin_top - margin_bottom;

  // Find temp min/max for scaling
  int temp_min = s_hourly_temps[0];
  int temp_max = s_hourly_temps[0];
  for (int i = 1; i < NUM_HOURLY_POINTS; i++) {
    if (s_hourly_temps[i] < temp_min) temp_min = s_hourly_temps[i];
    if (s_hourly_temps[i] > temp_max) temp_max = s_hourly_temps[i];
  }
  // Ensure at least some range so we don't divide by zero
  if (temp_max == temp_min) { temp_max = temp_min + 1; }

  int col_w = graph_w / NUM_HOURLY_POINTS;
  int graph_offset = (graph_w - col_w * NUM_HOURLY_POINTS) / 2;
  const int gx = graph_x + graph_offset;

  // Get current hour for positioning markers
  time_t now = time(NULL);
  struct tm *now_tm = localtime(&now);
  int current_hour = now_tm->tm_hour;


  // Draw precipitation bars (filled from bottom, height = precip% of graph_h)
  for (int i = 0; i < NUM_HOURLY_POINTS; i++) {
    if (s_hourly_precip[i] <= 0) continue;
    int bar_h = (s_hourly_precip[i] * graph_h) / 100;
    if (bar_h < 1) bar_h = 1;
    int bar_x = gx + i * col_w;
    int bar_y = graph_y + graph_h - bar_h;

    // Use gray for precipitation bars
    graphics_context_set_fill_color(ctx, GColorLightGray);
    graphics_fill_rect(ctx, GRect(bar_x + 1, bar_y, col_w - 2, bar_h), 0, GCornerNone);
  }

  // Draw temperature line graph on top
  graphics_context_set_stroke_color(ctx, text_color);
  graphics_context_set_stroke_width(ctx, 2);

  for (int i = 0; i < NUM_HOURLY_POINTS - 1; i++) {
    int x1 = gx + i * col_w + col_w / 2;
    int y1 = graph_y + graph_h - ((s_hourly_temps[i] - temp_min) * graph_h / (temp_max - temp_min));
    int x2 = gx + (i + 1) * col_w + col_w / 2;
    int y2 = graph_y + graph_h - ((s_hourly_temps[i + 1] - temp_min) * graph_h / (temp_max - temp_min));
    graphics_draw_line(ctx, GPoint(x1, y1), GPoint(x2, y2));
  }

  // Draw "now" dashed line at current hour position (on top of everything)
  {
    int now_x = gx + current_hour * col_w + col_w / 2;
    graphics_context_set_stroke_color(ctx, text_color);
    graphics_context_set_stroke_width(ctx, 1);
    for (int dy = graph_y; dy < graph_y + graph_h; dy += 4) {
      graphics_draw_line(ctx, GPoint(now_x, dy), GPoint(now_x, dy + 1));
    }
  }

  // Draw min/max temp labels on right y-axis
  GFont label_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  char min_buf[8], max_buf[8];
  snprintf(min_buf, sizeof(min_buf), "%d°", temp_min);
  snprintf(max_buf, sizeof(max_buf), "%d°", temp_max);

  // Max at top-right
  draw_outlined_text(ctx, max_buf, label_font,
                     GRect(bounds.size.w - 30, graph_y - 2, 28, 16),
                     GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentRight, text_color);
  // Min at bottom-right
  draw_outlined_text(ctx, min_buf, label_font,
                     GRect(bounds.size.w - 30, graph_y + graph_h - 14, 28, 16),
                     GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentRight, text_color);

  // Draw precipitation scale labels on left y-axis (0% to 100%)
  // 100% at top-left
  draw_outlined_text(ctx, "100%", label_font,
                     GRect(2, graph_y - 2, 32, 16),
                     GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentLeft, text_color);
  // 0% at bottom-left
  draw_outlined_text(ctx, "0%", label_font,
                     GRect(2, graph_y + graph_h - 14, 32, 16),
                     GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentLeft, text_color);
}

static void fade_timer_callback(void *data);
static void hide_timer_callback(void *data);
static void save_notification_visible(bool visible);

static void fade_timer_callback(void *data) {
  s_fade_timer = NULL;

  if (s_fading_in) {
    s_fade_progress++;
    if (s_fade_progress >= NOTIFICATION_FADE_FRAMES) {
      s_fade_progress = NOTIFICATION_FADE_FRAMES;
      s_fading_in = false;
      // Schedule auto-hide after display duration (unless "forever")
      if (s_notification_duration != 2) {
        int display_ms = (s_notification_duration == 0) ? 5000 : 10000;
        s_hide_timer = app_timer_register(display_ms, hide_timer_callback, NULL);
      }
    } else {
      s_fade_timer = app_timer_register(NOTIFICATION_FADE_RATE_MS, fade_timer_callback, NULL);
    }
  } else if (s_fading_out) {
    s_fade_progress--;
    if (s_fade_progress <= 0) {
      s_fade_progress = 0;
      s_fading_out = false;
    } else {
      s_fade_timer = app_timer_register(NOTIFICATION_FADE_RATE_MS, fade_timer_callback, NULL);
    }
  }

  layer_mark_dirty(s_notification_top_layer);
  layer_mark_dirty(s_notification_bottom_layer);
}

static void hide_timer_callback(void *data) {
  s_hide_timer = NULL;
  s_fading_out = true;
  s_fading_in = false;
  s_fade_timer = app_timer_register(NOTIFICATION_FADE_RATE_MS, fade_timer_callback, NULL);
  save_notification_visible(false);
}

void notification_init(Layer *window_layer, GRect bounds) {
  // Top bar: from top of screen down to the upper horizontal line
  s_notification_top_layer = layer_create(GRect(0, 0, bounds.size.w, NOTIFICATION_BAR_HEIGHT+1));
  layer_set_update_proc(s_notification_top_layer, draw_notification_top);
  layer_add_child(window_layer, s_notification_top_layer);

  // Bottom bar: from lower horizontal line down to bottom of screen
  int bottom_y = bounds.size.h - NOTIFICATION_BAR_HEIGHT;
  s_notification_bottom_layer = layer_create(GRect(0, bottom_y, bounds.size.w, NOTIFICATION_BAR_HEIGHT));
  layer_set_update_proc(s_notification_bottom_layer, draw_notification_bottom);
  layer_add_child(window_layer, s_notification_bottom_layer);

  // Restore previous visible state (not if disabled)
  if (s_notification_duration != 3 && persist_exists(PERSIST_KEY_NOTIFICATION_VISIBLE)
      && persist_read_bool(PERSIST_KEY_NOTIFICATION_VISIBLE)) {
    // Show immediately without animation
    s_fade_progress = NOTIFICATION_FADE_FRAMES;
    s_fading_in = false;
    s_fading_out = false;
    layer_mark_dirty(s_notification_top_layer);
    layer_mark_dirty(s_notification_bottom_layer);

    // Schedule auto-hide if not "forever"
    if (s_notification_duration != 2) {
      int display_ms = (s_notification_duration == 0) ? 5000 : 10000;
      s_hide_timer = app_timer_register(display_ms, hide_timer_callback, NULL);
    }
  }
}

void notification_deinit() {
  if (s_fade_timer) {
    app_timer_cancel(s_fade_timer);
    s_fade_timer = NULL;
  }
  if (s_hide_timer) {
    app_timer_cancel(s_hide_timer);
    s_hide_timer = NULL;
  }
  if (s_notification_top_layer) {
    layer_destroy(s_notification_top_layer);
    s_notification_top_layer = NULL;
  }
  if (s_notification_bottom_layer) {
    layer_destroy(s_notification_bottom_layer);
    s_notification_bottom_layer = NULL;
  }
  for (int i = 0; i < NUM_FORECAST_SLOTS; i++) {
    if (s_forecast_icons[i]) {
      gdraw_command_image_destroy(s_forecast_icons[i]);
      s_forecast_icons[i] = NULL;
    }
  }
}

static void save_notification_visible(bool visible) {
  persist_write_bool(PERSIST_KEY_NOTIFICATION_VISIBLE, visible);
}

void notification_show() {
  // If fully visible (not still fading in), dismiss
  if (s_fade_progress > 0 && !s_fading_in) {
    if (s_fade_timer) {
      app_timer_cancel(s_fade_timer);
      s_fade_timer = NULL;
    }
    if (s_hide_timer) {
      app_timer_cancel(s_hide_timer);
      s_hide_timer = NULL;
    }
    s_fading_in = false;
    s_fading_out = true;
    s_fade_timer = app_timer_register(NOTIFICATION_FADE_RATE_MS, fade_timer_callback, NULL);
    save_notification_visible(false);
    return;
  }

  // Cancel any ongoing timers
  if (s_fade_timer) {
    app_timer_cancel(s_fade_timer);
    s_fade_timer = NULL;
  }
  if (s_hide_timer) {
    app_timer_cancel(s_hide_timer);
    s_hide_timer = NULL;
  }

  s_fading_out = false;
  s_fading_in = true;
  s_fade_timer = app_timer_register(NOTIFICATION_FADE_RATE_MS, fade_timer_callback, NULL);
  save_notification_visible(true);
}

bool notification_is_visible() {
  return s_fade_progress > 0;
}
