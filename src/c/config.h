
#ifndef CONFIG_H
#define CONFIG_H

/*
 * Definitions
 */
#define PERSIST_KEY_COLOR_THEME 1
#define PERSIST_KEY_WEATHER_CODE 2
#define PERSIST_KEY_TEMPERATURE 3
#define PERSIST_KEY_LOCATION 4
#define PERSIST_KEY_STEP_GOAL 5
#define PERSIST_KEY_TEMPERATURE_UNIT 6
#define PERSIST_KEY_IS_DAY 7
#define PERSIST_KEY_ENABLE_ANIMATIONS 8
#define PERSIST_KEY_LAYOUT_UPPER_LEFT 9
#define PERSIST_KEY_LAYOUT_UPPER_RIGHT 10
#define PERSIST_KEY_LAYOUT_LOWER_LEFT 11
#define PERSIST_KEY_LAYOUT_LOWER_RIGHT 12
#define PERSIST_KEY_DISCONNECT_POSITION 13
#define PERSIST_KEY_ENABLE_WEATHER_FORECAST 14
#define PERSIST_KEY_WEATHER_FORECAST_DURATION 15
#define PERSIST_KEY_WEATHER_FORECAST_VISIBLE 16
#define PERSIST_KEY_FORECAST_DATA 17
#define PERSIST_KEY_HOURLY_TEMPS 18
#define PERSIST_KEY_HOURLY_PRECIP 19
#define PERSIST_KEY_HOURLY_DATA_AVAILABLE 20
#define PERSIST_KEY_WEATHER_FORECAST_FLICK_MODE 21
#define PERSIST_KEY_ENABLE_MESH 22
#define PERSIST_KEY_DATE_FORMAT 23
#define PERSIST_KEY_LIGHT_SHOW_BACKGROUND 24
#define PERSIST_KEY_DARK_SHOW_BORDER 25
#define PERSIST_KEY_VIBRATE_ON_DISCONNECT 26
// Keys 27/28 held free-form 0xRRGGBB colors in older versions; the enum
// colors use fresh keys so stale hex values are ignored and everyone
// starts from the defaults again.
#define PERSIST_KEY_CUSTOM_LINE_1 29
#define PERSIST_KEY_CUSTOM_LINE_2 30
#define PERSIST_KEY_LIGHT_BG_COLOR 31
#define PERSIST_KEY_DARK_BG_COLOR 32
#define PERSIST_KEY_WEATHER_TIMESTAMP 33
#define PERSIST_KEY_CUSTOM_DATA_TIMESTAMP 34

// Background color choices. Must match BG_COLOR_ENUM in src/pkjs/index.js.
// The settings UI only offers WHITE, BLACK and GRAY on BW platforms.
typedef enum {
  BG_COLOR_WHITE = 0,
  BG_COLOR_BLACK = 1,
  BG_COLOR_RED = 2,
  BG_COLOR_GREEN = 3,
  BG_COLOR_BLUE = 4,
  BG_COLOR_YELLOW = 5,
  BG_COLOR_GRAY = 6
} BgColor;

// Layer position and alignment enums
typedef enum {
  LAYER_UPPER_LEFT = 0,
  LAYER_UPPER_RIGHT = 1,
  LAYER_LOWER_LEFT = 2,
  LAYER_LOWER_RIGHT = 3,
  NUM_INFO_LAYERS = 4
} InfoLayerPosition;

typedef enum {
  ALIGN_LEFT = 0,
  ALIGN_RIGHT = 1
} InfoLayerAlignment;

// Info layer structure with layer tracking
typedef struct {
  Layer* layer;
  GRect bounds;
  int position;
  TextLayer* text_layer1;
  TextLayer* text_layer2;
  BitmapLayer* bitmap_layer_1; 
  BitmapLayer* bitmap_layer_2;
  BitmapLayer* bitmap_layer_3;
  Layer* custom_layer;
} InfoLayer;


// Layer assignment configuration - maps each layer position to an info type
typedef enum {
  INFO_TYPE_WEATHER = 0,
  INFO_TYPE_TEMPERATURE = 1,
  INFO_TYPE_STEPS = 2,
  INFO_TYPE_BATTERY = 3,
  INFO_TYPE_COLORED_BOX = 4,
  INFO_TYPE_NONE = 5,
  INFO_TYPE_CALENDAR = 6,
  INFO_TYPE_DISCONNECT = 7,
  INFO_TYPE_HEART_RATE = 8,
  INFO_TYPE_CUSTOM_URL = 9
} InfoType;

// Current layer assignments (can be changed dynamically)
extern InfoType s_layer_assignments[NUM_INFO_LAYERS];

// Info layers array
extern InfoLayer s_info_layers[NUM_INFO_LAYERS];

/*
 * Dynamic Config
 */
extern int s_color_theme;
extern int s_step_goal;
extern int s_temperature_unit;  // 0 = celsius, 1 = fahrenheit
extern int s_is_day; // 1 = day, 0 = night
extern int s_enable_animations; // 1 = enabled, 0 = disabled
extern int s_disconnect_position; // 0 = disabled, 1-4 = UL/UR/LL/LR
extern int s_enable_weather_forecast; // 1 = enabled, 0 = disabled
extern int s_weather_forecast_duration; // 0 = 5s, 1 = 10s, 2 = forever
extern int s_weather_forecast_flick_mode; // 0 = disabled, 1 = single flick, 2 = double flick
extern int s_enable_mesh; // 1 = enabled, 0 = disabled
extern char s_date_format[16]; // strftime format string for date display
extern int s_light_show_background; // 1 = show gray box in light theme, 0 = hide
extern int s_dark_show_border; // 1 = show border in dark theme, 0 = hide
extern int s_vibrate_on_disconnect; // 1 = vibrate on connect/disconnect, 0 = disabled
extern int s_light_bg_color; // Background color for light theme (BgColor enum)
extern int s_dark_bg_color; // Background color for dark theme (BgColor enum)
extern char s_custom_line1[33]; // Extracted first line (large font)
extern char s_custom_line2[33]; // Extracted second line (small font), empty = single line
extern time_t s_custom_data_last_received; // 0 = never received live data

/*
 * Function Declarations
 */
void save_theme_to_storage();
void load_theme_from_storage();
void save_step_goal_to_storage();
void load_step_goal_from_storage();
void save_temperature_unit_to_storage();
void load_temperature_unit_from_storage();
void save_enable_animations_to_storage();
void load_enable_animations_from_storage();
void save_layout_to_storage();
void load_layout_from_storage();
void save_disconnect_position_to_storage();
void load_disconnect_position_from_storage();
void save_weather_forecast_duration_to_storage();
void load_weather_forecast_duration_from_storage();
void save_weather_forecast_flick_mode_to_storage();
void load_weather_forecast_flick_mode_from_storage();
void save_enable_mesh_to_storage();
void load_enable_mesh_from_storage();
void save_date_format_to_storage();
void load_date_format_from_storage();
void save_light_show_background_to_storage();
void load_light_show_background_from_storage();
void save_dark_show_border_to_storage();
void load_dark_show_border_from_storage();
void save_vibrate_on_disconnect_to_storage();
void load_vibrate_on_disconnect_from_storage();
void save_bg_colors_to_storage();
void load_bg_colors_from_storage();
void save_custom_lines_to_storage();
void load_custom_lines_from_storage();
bool is_dark_theme();
bool is_light_theme();
GColor get_background_color();
GColor get_text_color();
bool is_bw_gray_background();

#endif // CONFIG_H
