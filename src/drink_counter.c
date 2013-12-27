#include "header.h"
#include "display_layer.h"

static Window *window;

typedef enum {DRINK_PER_HOUR, FIRST_DRINK} LowerLayer;

//Layout DisplayLayers
static DisplayLayer *drinkCountDisplayLayer;
static DisplayLayer *drinksPerHourDisplayLayer;
static DisplayLayer *firstDrinkDisplayLayer;

static DisplayLayer* displayLayers[LOWER_LAYER_COUNT];
static LowerLayer layerIndex = DRINK_PER_HOUR;

static const char *DRINK_NUM = "Drink #:";
static const char *DRINK_PER = "Drink/hr:";
static const char *FIRST = "First:";

static int drinkCount;
static uint startTime;

static void floatToString(char* buffer, int bufferSize, double number);
static void setDrinkCountStr(int count);
static void setDrinkCountTextLayerText(int count);
static void setDrinkPerStr(uint currentTime);
static void setDrinksPerHourTextLayerText();
static void setFirstDrinkTextLayerText();
static void resetDrinkCount();
static void drink_layer_load(Layer *parent);
static void window_layer_update_callback(Layer *layer, GContext *ctx);


static void select_click_handler(ClickRecognizerRef recognizer, void *context) 
{
  resetDrinkCount();
}

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) 
{
  drinkCount++;
  setDrinkCountTextLayerText(drinkCount);
  setDrinksPerHourTextLayerText();
}

static void up_double_click_handler(ClickRecognizerRef recognizer, void *context)
{
  if(drinkCount > 0)
  {
    drinkCount--;
    setDrinkCountTextLayerText(drinkCount);
    setDrinksPerHourTextLayerText();
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) 
{
  layerIndex == LOWER_LAYER_COUNT - 1 ? layerIndex = 0 : layerIndex++;

  switch(layerIndex)
  {
    case DRINK_PER_HOUR:
      layer_remove_from_parent(firstDrinkDisplayLayer->displayLayer);
      layer_add_child(window_get_root_layer(window), drinksPerHourDisplayLayer->displayLayer);
      break;
    case FIRST_DRINK:
      layer_remove_from_parent(drinksPerHourDisplayLayer->displayLayer);
      layer_add_child(window_get_root_layer(window), firstDrinkDisplayLayer->displayLayer);
      break;
  }
}

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

  displayLayers[0] = drinksPerHourDisplayLayer;
  displayLayers[1] = firstDrinkDisplayLayer;
}

static void window_unload(Window *window) 
{
  text_layer_destroy(drinkCountDisplayLayer->staticTextLayer);
  text_layer_destroy(drinkCountDisplayLayer->dynamicTextLayer);
  text_layer_destroy(drinksPerHourDisplayLayer->staticTextLayer);
  text_layer_destroy(drinksPerHourDisplayLayer->dynamicTextLayer);

  layer_destroy(drinkCountDisplayLayer->displayLayer);
  layer_destroy(drinkCountDisplayLayer->displayLayer);
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
  snprintf(drinkCountDisplayLayer->dynamicStr, sizeof(drinkCountDisplayLayer->dynamicStr), "%d", count);
}

static void setDrinkCountTextLayerText(int count)
{
  setDrinkCountStr(count);
  text_layer_set_text(drinkCountDisplayLayer->dynamicTextLayer, drinkCountDisplayLayer->dynamicStr);
  layer_mark_dirty(text_layer_get_layer(drinkCountDisplayLayer->dynamicTextLayer));
}

static void setDrinkPerStr(uint currentTime)
{
  float hours = (currentTime - startTime) / HOUR_CONVERT;
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "currentTime: %d\tstartTime: %d", currentTime, startTime);
  float drinksPerHour = (float)drinkCount / (hours > 1 ? hours : 1.0);
  floatToString(drinksPerHourDisplayLayer->dynamicStr, BUFFER_LENGTH, drinksPerHour);
}

static void setDrinksPerHourTextLayerText()
{
  setDrinkPerStr((uint)time(NULL));
  text_layer_set_text(drinksPerHourDisplayLayer->dynamicTextLayer, drinksPerHourDisplayLayer->dynamicStr);
  layer_mark_dirty(text_layer_get_layer(drinksPerHourDisplayLayer->dynamicTextLayer));
}

static void setFirstDrinkTextLayerText()
{

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

static void drink_layer_load(Layer *parent)
{
  GRect bounds = layer_get_bounds(parent);
  
  drinkCountDisplayLayer = display_layer_create(bounds, DRINK_NUM);
  setDrinkCountTextLayerText(drinkCount);

  GRect LowerLayerBounds = GRect(0, 1 + (bounds.size.h/2), bounds.size.w, bounds.size.h/2 - 1);
  drinksPerHourDisplayLayer = display_layer_create(LowerLayerBounds, DRINK_PER);
  setDrinksPerHourTextLayerText();

  firstDrinkDisplayLayer = display_layer_create(LowerLayerBounds, FIRST);
  setDrinksPerHourTextLayerText();

  layer_add_child(parent, drinkCountDisplayLayer->displayLayer);
  layer_add_child(parent, drinksPerHourDisplayLayer->displayLayer);
}

static void window_layer_update_callback(Layer *layer, GContext *ctx)
{
  GRect bounds = layer_get_bounds(layer);
  GPoint p0 = GPoint(0, bounds.size.h/2);
  GPoint p1 = GPoint(bounds.size.w, bounds.size.h/2);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_line(ctx, p0, p1);
}
