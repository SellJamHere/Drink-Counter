#include "header.h"
#include "display_layer.h"

static Window *window;

//Layout layers
DisplayLayer drinkCountDisplayLayer;
DisplayLayer drinksPerHourDisplayLayer;
// static Layer *drinkNumberLayer;
// static TextLayer *drinkNumberStaticTextLayer;     //"Drink #:"
// static TextLayer *drinkNumberTextLayer;

// static Layer *drinksPerHourLayer;
// static TextLayer *drinksPerHourStaticTextLayer;   //"Drinks/hr:"
// static TextLayer *drinksPerHourTextLayer;

static const char *DRINK_NUM = "Drink #:";
static const char *DRINK_PER = "Drink/hr:";

static int drinkCount;
static uint startTime;
static char drinkCountStr[BUFFER_LENGTH];
static char drinksPerStr[BUFFER_LENGTH];


static void floatToString(char* buffer, int bufferSize, double number);
static void setDrinkCountStr(int count);
static void setDrinkPerStr(uint currentTime);
static void setDrinksPerHourTextLayerText();
static void resetDrinkCount();
static void drink_layer_load(Layer *parent);
static void drink_number_layer_load(Layer *parent);
static void drinks_per_hour_layer_load(Layer *parent);
static void display_layer_set(Layer *displayLayer, 
                              TextLayer *staticTextLayer, 
                              TextLayer *dynamicTextLayer, 
                              const char staticStr[], 
                              char dynamicStr[]);
static void window_layer_update_callback(Layer *layer, GContext *ctx);


static void select_click_handler(ClickRecognizerRef recognizer, void *context) 
{
  resetDrinkCount();
}

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) 
{
  drinkCount++;
  setDrinkCountStr(drinkCount);
  setDrinksPerHourTextLayerText();
}

static void up_double_click_handler(ClickRecognizerRef recognizer, void *context)
{
  if(drinkCount > 0)
  {
    drinkCount--;
    setDrinkCountStr(drinkCount);
    setDrinksPerHourTextLayerText();
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {}

static void click_config_provider(void *context) 
{
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
  window_multi_click_subscribe(BUTTON_ID_UP, 2, 0, 200, false, up_double_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) 
{
  Layer *window_layer = window_get_root_layer(window);

  layer_set_update_proc(window_layer, window_layer_update_callback);

  drink_layer_load(window_layer);
}

static void window_unload(Window *window) 
{
  text_layer_destroy(drinkCountDisplayLayer.staticTextLayer);
  text_layer_destroy(drinkCountDisplayLayer.dynamicTextLayer);
  text_layer_destroy(drinksPerHourDisplayLayer.staticTextLayer);
  text_layer_destroy(drinksPerHourDisplayLayer.dynamicTextLayer);

  layer_destroy(drinkCountDisplayLayer.displayLayer);
  layer_destroy(drinkCountDisplayLayer.displayLayer);
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
  drinkCount = persist_exists(DRINK_COUNT_KEY) ? persist_read_int(DRINK_COUNT_KEY) : DRINK_COUNT_DEFAULT;
  startTime = persist_exists(START_TIME_KEY) ? (uint)persist_read_int(START_TIME_KEY) : (uint)time(NULL);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "drinkCount: %d,\tstartTime: %d", drinkCount, startTime);

  window_stack_push(window, animated);
}

static void deinit(void) 
{
  if(drinkCount != 0)
  {
    //save to persistant storage
    APP_LOG(APP_LOG_LEVEL_DEBUG, "drinkCount: %d,\tstartTime: %d", drinkCount, startTime);
    int countStatus = persist_write_int(DRINK_COUNT_KEY, drinkCount);
    int startStatus = persist_write_int(START_TIME_KEY, startTime);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "countStatus: %d,\tstartStatus: %d", countStatus, startStatus);
  }

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
  snprintf(drinkCountStr, sizeof(drinkCountStr), "%d", count);
  text_layer_set_text(drinkCountDisplayLayer.dynamicTextLayer, drinkCountStr);
  layer_mark_dirty(text_layer_get_layer(drinkCountDisplayLayer.dynamicTextLayer));
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
  text_layer_set_text(drinksPerHourDisplayLayer.dynamicTextLayer, drinksPerStr);
  layer_mark_dirty(text_layer_get_layer(drinksPerHourDisplayLayer.dynamicTextLayer));
}

static void resetDrinkCount()
{
  drinkCount = 0;
  startTime = (uint)time(NULL);
  setDrinkCountStr(drinkCount);
  setDrinksPerHourTextLayerText();

  persist_delete(DRINK_COUNT_KEY);
  persist_delete(START_TIME_KEY);
}

//setup drink layers
static void drink_layer_load(Layer *parent)
{
  // drink_number_layer_load(parent);
  // drinks_per_hour_layer_load(parent);
  GRect bounds = layer_get_bounds(parent);
  
  drinkCountDisplayLayer = display_layer_create(bounds, DRINK_NUM, drinkCountStr);
  setDrinkCountStr(drinkCount);

  drinksPerHourDisplayLayer = display_layer_create(GRect(0, 1 + (bounds.size.h/2), bounds.size.w, bounds.size.h/2 - 1), DRINK_PER, drinksPerStr);
  setDrinkPerStr((uint)time(NULL));

  layer_add_child(parent, drinkCountDisplayLayer.displayLayer);
  layer_add_child(parent, drinksPerHourDisplayLayer.displayLayer);
}

// static void drink_number_layer_load(Layer *parent)
// {
//   GRect parentBounds = layer_get_bounds(parent);
//   drinkNumberLayer = layer_create(GRect(0, 0, parentBounds.size.w, parentBounds.size.h/2));
//   //init static "Drink #:" text layer
//   drinkNumberStaticTextLayer = text_layer_create(GRect(0, 0, parentBounds.size.w, TEXT_HEIGHT));
//   //init drink number layer
//   GRect topBounds = layer_get_bounds(text_layer_get_layer(drinkNumberStaticTextLayer));
//   drinkNumberTextLayer = text_layer_create(GRect(0, 1 + (topBounds.size.h + topBounds.origin.y), parentBounds.size.w, TEXT_HEIGHT));
//   setDrinkCountStr(drinkCount);
  
//   display_layer_set(drinkNumberLayer, drinkNumberStaticTextLayer, drinkNumberTextLayer, DRINK_NUM, drinkCountStr);
// }

// static void drinks_per_hour_layer_load(Layer *parent)
// {
//   GRect parentBounds = layer_get_bounds(parent);
//   drinksPerHourLayer = layer_create(GRect(0, 1 + (parentBounds.size.h/2), parentBounds.size.w, parentBounds.size.h/2 - 1));
//   //init static "Drink/hr:" text layer
//   drinksPerHourStaticTextLayer = text_layer_create(GRect(0, 0, parentBounds.size.w, TEXT_HEIGHT));
//   //init drink/hr number layer
//   GRect topBounds = layer_get_bounds(text_layer_get_layer(drinksPerHourStaticTextLayer));
//   drinksPerHourTextLayer = text_layer_create(GRect(0, 1 + (topBounds.size.h + topBounds.origin.y), parentBounds.size.w, TEXT_HEIGHT));
//   setDrinkPerStr((uint)time(NULL));

//   display_layer_set(drinksPerHourLayer, drinksPerHourStaticTextLayer, drinksPerHourTextLayer, DRINK_PER, drinksPerStr);
// }

// static void display_layer_set(Layer *displayLayer, TextLayer *staticTextLayer, TextLayer *dynamicTextLayer, const char *staticStr, char dynamicStr[])
// {
//   //init static text layer
//   text_layer_set_text(staticTextLayer, staticStr);
//   text_layer_set_font(staticTextLayer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
//   layer_add_child(displayLayer, text_layer_get_layer(staticTextLayer));
//   //init dynamic text layer
//   text_layer_set_text(dynamicTextLayer, dynamicStr);
//   text_layer_set_font(dynamicTextLayer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
//   text_layer_set_text_alignment(dynamicTextLayer, GTextAlignmentRight);
//   layer_add_child(displayLayer, text_layer_get_layer(dynamicTextLayer));
// }

static void window_layer_update_callback(Layer *layer, GContext *ctx)
{
  GRect bounds = layer_get_bounds(layer);
  GPoint p0 = GPoint(0, bounds.size.h/2);
  GPoint p1 = GPoint(bounds.size.w, bounds.size.h/2);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_line(ctx, p0, p1);
}
