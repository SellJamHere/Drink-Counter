#include <pebble.h>

/************************************************************
 *  DEFINED CONSTANTS
 *  *****************
 *
 *  BUFFER_LENGTH 16     - c string array size
 *
 *  DRINK_COUNT_KEY 0    - Persistance Key for drinkCount
 *  START_TIME_KEY 1     - Persistance Key for startTime
 *
 *  DRINK_COUNT_DEFAULT  - Default value for drinkCount, 
 *                         if no value has been stored
 ************************************************************/
#define BUFFER_LENGTH 16
#define HOUR_CONVERT 3600
//Persistance keys
#define DRINK_COUNT_KEY 0
#define START_TIME_KEY 1
//Persistance defaults
#define DRINK_COUNT_DEFAULT 0

static Window *window;

//Layout layers
static Layer *drinkNumberLayer;
static TextLayer *drinkNumberStaticTextLayer;     //"Drink #:"
static TextLayer *drinkNumberTextLayer;

static Layer *drinksPerHourLayer;
static TextLayer *drinksPerHourStaticTextLayer;   //"Drinks/hr:"
static TextLayer *drinksPerHourTextLayer;

static const char DRINK_NUM[] = "Drink #:";
static const char DRINK_PER[] = "Drink/hr:";

static uint drinkCount;
static uint startTime;
static char drinkNumStr[BUFFER_LENGTH];
static char drinksPerStr[BUFFER_LENGTH];


static void floatToString(char* buffer, int bufferSize, double number);
static void setDrinkCountStr(int count);
static void setDrinkPerStr(uint currentTime);
static void setDrinksPerHourTextLayerText();
static void drink_layer_load(Layer *parent);
static void drink_number_layer_load(Layer *parent);
static void drinks_per_hour_layer_load(Layer *parent);
static void display_layer_set(Layer *displayLayer, 
                              TextLayer *staticTextLayer, 
                              TextLayer *dynamicTextLayer, 
                              const char staticStr[], 
                              char dynamicStr[]);


static void select_click_handler(ClickRecognizerRef recognizer, void *context) {}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) 
{
  drinkCount++;
  setDrinkCountStr(drinkCount);
  setDrinksPerHourTextLayerText();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {}

static void click_config_provider(void *context) 
{
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) 
{
  Layer *window_layer = window_get_root_layer(window);

  drink_layer_load(window_layer);
}

static void window_unload(Window *window) 
{
  text_layer_destroy(drinkNumberStaticTextLayer);
  text_layer_destroy(drinkNumberTextLayer);
  text_layer_destroy(drinksPerHourStaticTextLayer);
  text_layer_destroy(drinksPerHourTextLayer);

  layer_destroy(drinkNumberLayer);
  layer_destroy(drinksPerHourLayer);
}

static void init(void) 
{
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;

  //initialize from persistant storage, if values exist
  APP_LOG(APP_LOG_LEVEL_DEBUG, "drinkCount exists?: %d,\tstartTime exists?: %d", persist_exists(DRINK_COUNT_KEY), persist_exists(START_TIME_KEY));
  drinkCount = persist_exists(DRINK_COUNT_KEY) ? (uint)persist_read_int(DRINK_COUNT_KEY) : (uint)DRINK_COUNT_DEFAULT;
  startTime = persist_exists(START_TIME_KEY) ? (uint)persist_read_int(START_TIME_KEY) : (uint)time(NULL);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "drinkCount: %d,\tstartTime: %d", drinkCount, startTime);

  window_stack_push(window, animated);
}

static void deinit(void) {
  //save to persistant storage
  APP_LOG(APP_LOG_LEVEL_DEBUG, "drinkCount: %d,\tstartTime: %d", drinkCount, startTime);
  int countStatus = persist_write_int(DRINK_COUNT_KEY, drinkCount);
  int startStatus = persist_write_int(START_TIME_KEY, startTime);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "countStatus: %d,\tstartStatus: %d", countStatus, startStatus);

  window_destroy(window);
}

int main(void) 
{
  init();
  app_event_loop();
  deinit();
}

static void floatToString(char* buffer, int bufferSize, double number)
{
  char decimalBuffer[5];

  snprintf(buffer, bufferSize, "%d", (int)number);
  strcat(buffer, ".");

  snprintf(decimalBuffer, 5, "%02d", (int)((double)(number - (int)number) * (double)100));
  strcat(buffer, decimalBuffer);
}

static void setDrinkCountStr(int count)
{
  snprintf(drinkNumStr, sizeof(drinkNumStr), "%d", count);
  text_layer_set_text(drinkNumberTextLayer, drinkNumStr);
}

static void setDrinkPerStr(uint currentTime)
{
  float hours = (currentTime - startTime) / HOUR_CONVERT;
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "currentTime: %d\tstartTime: %d", currentTime, startTime);
  float drinksPerHour = (float)drinkCount / (hours > 1 ? hours : 1.0);
  floatToString(drinksPerStr, BUFFER_LENGTH, drinksPerHour);
}

static void setDrinksPerHourTextLayerText()
{
  setDrinkPerStr((uint)time(NULL));
  text_layer_set_text(drinksPerHourTextLayer, drinksPerStr);
}

//setup drink layers
static void drink_layer_load(Layer *parent)
{
  drink_number_layer_load(parent);
  drinks_per_hour_layer_load(parent);

  layer_add_child(parent, drinkNumberLayer);
  layer_add_child(parent, drinksPerHourLayer);
}

static void drink_number_layer_load(Layer *parent)
{
  GRect parentBounds = layer_get_bounds(parent);
  drinkNumberLayer = layer_create(GRect(0, 0, parentBounds.size.w, parentBounds.size.h/2));
  //init static "Drink #:" text layer
  drinkNumberStaticTextLayer = text_layer_create(GRect(0, 0, parentBounds.size.w, 30));
  //init drink number layer
  GRect topBounds = layer_get_bounds(text_layer_get_layer(drinkNumberStaticTextLayer));
  drinkNumberTextLayer = text_layer_create(GRect(0, 1 + (topBounds.size.h + topBounds.origin.y), parentBounds.size.w, 30));
  setDrinkCountStr(drinkCount);
  
  display_layer_set(drinkNumberLayer, drinkNumberStaticTextLayer, drinkNumberTextLayer, DRINK_NUM, drinkNumStr);
}

static void drinks_per_hour_layer_load(Layer *parent)
{
  GRect parentBounds = layer_get_bounds(parent);
  drinksPerHourLayer = layer_create(GRect(0, 1 + (parentBounds.size.h/2), parentBounds.size.w, parentBounds.size.h/2 - 1));
  //init static "Drink/hr:" text layer
  drinksPerHourStaticTextLayer = text_layer_create(GRect(0, 0, parentBounds.size.w, 30));
  //init drink/hr number layer
  GRect topBounds = layer_get_bounds(text_layer_get_layer(drinksPerHourStaticTextLayer));
  drinksPerHourTextLayer = text_layer_create(GRect(0, 1 + (topBounds.size.h + topBounds.origin.y), parentBounds.size.w, 30));
  setDrinkPerStr((uint)time(NULL));

  display_layer_set(drinksPerHourLayer, drinksPerHourStaticTextLayer, drinksPerHourTextLayer, DRINK_PER, drinksPerStr);
}

static void display_layer_set(Layer *displayLayer, TextLayer *staticTextLayer, TextLayer *dynamicTextLayer, const char staticStr[], char dynamicStr[])
{
  //init static text layer
  text_layer_set_text(staticTextLayer, staticStr);
  text_layer_set_font(staticTextLayer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  layer_add_child(displayLayer, text_layer_get_layer(staticTextLayer));
  //init dynamic text layer
  text_layer_set_text(dynamicTextLayer, dynamicStr);
  text_layer_set_font(dynamicTextLayer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_alignment(dynamicTextLayer, GTextAlignmentRight);
  layer_add_child(displayLayer, text_layer_get_layer(dynamicTextLayer));
}
