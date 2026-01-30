
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
} InfoLayer;


// Layer assignment configuration - maps each layer position to an info type
typedef enum {
  INFO_TYPE_WEATHER = 0,
  INFO_TYPE_TEMPERATURE = 1,
  INFO_TYPE_STEPS = 2,
  INFO_TYPE_BATTERY = 3,
  INFO_TYPE_COLORED_BOX = 4,
  INFO_TYPE_NONE = 5
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

/*
 * Function Declarations
 */
void save_theme_to_storage();
void load_theme_from_storage();
void save_step_goal_to_storage();
void load_step_goal_from_storage();
void save_temperature_unit_to_storage();
void load_temperature_unit_from_storage();
GColor get_background_color();
GColor get_text_color();

#endif // CONFIG_H