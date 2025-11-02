
#ifndef CONFIG_H
#define CONFIG_H

/*
 * Definitions
 */
#define PERSIST_KEY_COLOR_THEME 1
#define PERSIST_KEY_WEATHER_CODE 2
#define PERSIST_KEY_TEMPERATURE 3
#define PERSIST_KEY_LOCATION 4


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


// Info layers system
InfoLayer s_info_layers[NUM_INFO_LAYERS];

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
InfoType s_layer_assignments[NUM_INFO_LAYERS];


/*
 * Dynamic Config
 */
int s_color_theme;


/*
 * Function Declarations
 */
void save_theme_to_storage();
void load_theme_from_storage();
GColor get_background_color();
GColor get_text_color();

#endif // CONFIG_H