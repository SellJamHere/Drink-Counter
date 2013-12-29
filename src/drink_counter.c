#include "header.h"
#include "display_layer.h"

/********************************************************** 
* 
* DRINK COUNTER
* 
*_________________________________________________________ 
* This Pebble watchapp allows a user to monitor drink 
* consumption. The total number of drinks, time of the 
* first drink, time of the last drink and drinks per hour
* are recorded. Drink count is always displayed on the top 
* half of the screen, while first, last and drinks per
* hour can be cycled through on the bottom half.
*_________________________________________________________ 
* Input is given through button clicks.
* UP single click     - increment drink count
* UP double click     - decrement drink count
* SELECT double click - reset drink counter
* DOWN single click   - change lower display
***********************************************************/ 

static Window *window;

//Layout DisplayLayers
static DisplayLayer *drinkCountDisplayLayer;
static DisplayLayer *drinksPerHourDisplayLayer;
static DisplayLayer *firstDrinkDisplayLayer;
static DisplayLayer *lastDrinkDisplayLater;

static DisplayLayer* lowerLayers[LOWER_LAYER_COUNT];
static int layerIndex = 0;

static const char *DRINK_NUM = "Drink #:";
static const char *DRINK_PER = "Drink/hr:";
static const char *FIRST = "First:";
static const char *LAST = "Last:";

static int drinkCount;
static uint startTime;
static char firstDrinkStr[BUFFER_LENGTH];
static char lastDrinkStr[BUFFER_LENGTH];

//Pebble Foundation functions
static void init(void);
static void deinit(void);
static void window_load(Window *window);
static void window_unload(Window *window);
//Click handlers
static void click_config_provider(void *context);
static void up_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void up_double_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_double_click_handler(ClickRecognizerRef recognizer, void *context);
static void down_click_handler(ClickRecognizerRef recognizer, void *context);

static void drink_layer_load(Layer *parent);
static void floatToString(char* buffer, int bufferSize, double number);
static void setDrinkCountStr(int count);
static void setDrinkCountTextLayerText(int count);
static void setDrinkPerStr(uint currentTime);
static void setDrinksPerHourTextLayerText();
static void setFirstDrinkTextLayerText();
static void updateFirstDrinkTextLayerText();
static void clearFirstDrinkTextLayerText();
static void setLastDrinkTextLayerText();
static void updateLastDrinkTextLayerText();
static void clearLastDrinkTextLayerText();
static void resetDrinkCount();
static void window_layer_update_callback(Layer *layer, GContext *ctx);

int main(void) 
{
  init();
  app_event_loop();
  deinit();
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
  drinkCount = persist_exists(DRINK_COUNT_KEY) ? persist_read_int(DRINK_COUNT_KEY) : DRINK_COUNT_DEFAULT;
  startTime = persist_exists(START_TIME_KEY) ? (uint)persist_read_int(START_TIME_KEY) : 0;
  if(drinkCount != 0)
  {
    if(persist_exists(FIRST_DRINK_KEY))
    {
      persist_read_string(FIRST_DRINK_KEY, firstDrinkStr, BUFFER_LENGTH);
    }
    if(persist_exists(LAST_DRINK_KEY))
    {
      persist_read_string(LAST_DRINK_KEY, lastDrinkStr, BUFFER_LENGTH);
    }
    
  }
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "drinkCount: %d,\tstartTime: %d,\tfirstDrink: %s", drinkCount, startTime, firstDrinkStr);

  window_stack_push(window, animated);
}

static void deinit(void) 
{
  if(drinkCount != 0)
  {
    //save to persistant storage
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "drinkCount: %d,\tstartTime: %d", drinkCount, startTime);
    persist_write_int(DRINK_COUNT_KEY, drinkCount);
    persist_write_int(START_TIME_KEY, startTime);
    persist_write_string(FIRST_DRINK_KEY, firstDrinkStr);
    persist_write_string(LAST_DRINK_KEY, lastDrinkStr);
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "countStatus: %d,\tstartStatus: %d", countStatus, startStatus);
  }

  free(drinkCountDisplayLayer);

  for (int i = 0; i < LOWER_LAYER_COUNT; i++)
  {
    DisplayLayer *lowerLayer = lowerLayers[i];
    free(lowerLayer);
  }

  window_destroy(window);
}

static void window_load(Window *window) 
{
  Layer *window_layer = window_get_root_layer(window);

  layer_set_update_proc(window_layer, window_layer_update_callback);

  drink_layer_load(window_layer);

  lowerLayers[0] = drinksPerHourDisplayLayer;
  lowerLayers[1] = firstDrinkDisplayLayer;
  lowerLayers[2] = lastDrinkDisplayLater;
}

static void window_unload(Window *window) 
{
  display_layer_destroy(drinkCountDisplayLayer);
  display_layer_destroy(drinksPerHourDisplayLayer);
  display_layer_destroy(firstDrinkDisplayLayer);
  display_layer_destroy(lastDrinkDisplayLater);
}

static void click_config_provider(void *context) 
{
  window_multi_click_subscribe(BUTTON_ID_SELECT, 2, 0, 200, false, select_double_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
  window_multi_click_subscribe(BUTTON_ID_UP, 2, 0, 200, false, up_double_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) 
{
  if(drinkCount == 0)
  {
    startTime = (uint)time(NULL);
    setFirstDrinkTextLayerText();
  }
  drinkCount++;
  setDrinkCountTextLayerText(drinkCount);
  setDrinksPerHourTextLayerText();
  setLastDrinkTextLayerText();
}

static void up_double_click_handler(ClickRecognizerRef recognizer, void *context)
{
  if(drinkCount - 1 == 0)
  {
    resetDrinkCount();
  }
  else if(drinkCount > 0)
  {
    drinkCount--;
    setDrinkCountTextLayerText(drinkCount);
    setDrinksPerHourTextLayerText();
  }
}

static void select_double_click_handler(ClickRecognizerRef recognizer, void *context) 
{
  resetDrinkCount();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) 
{
  DisplayLayer *previousDisplayLayer = lowerLayers[layerIndex];

  (layerIndex == LOWER_LAYER_COUNT - 1) ? layerIndex = 0 : layerIndex++;
  DisplayLayer *currentDisplayLayer = lowerLayers[layerIndex];

  layer_remove_from_parent(previousDisplayLayer->displayLayer);
  layer_add_child(window_get_root_layer(window), currentDisplayLayer->displayLayer);
     
}




static void drink_layer_load(Layer *parent)
{
  GRect bounds = layer_get_bounds(parent);
  
  drinkCountDisplayLayer = display_layer_create(bounds, DRINK_NUM);
  setDrinkCountTextLayerText(drinkCount);

  GRect lowerLayerBounds = GRect(0, 1 + (bounds.size.h/2), bounds.size.w, bounds.size.h/2 - 1);
  drinksPerHourDisplayLayer = display_layer_create(lowerLayerBounds, DRINK_PER);
  setDrinksPerHourTextLayerText();

  firstDrinkDisplayLayer = display_layer_create(lowerLayerBounds, FIRST);
  updateFirstDrinkTextLayerText();

  lastDrinkDisplayLater = display_layer_create(lowerLayerBounds, LAST);
  updateLastDrinkTextLayerText();

  layer_add_child(parent, drinkCountDisplayLayer->displayLayer);
  layer_add_child(parent, drinksPerHourDisplayLayer->displayLayer);
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
  float hours = (currentTime - startTime) / (float)HOUR_CONVERT;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "currentTime: %d\tstartTime: %d", currentTime, startTime);
  float drinksPerHour;
  if(drinkCount == 1)
  {
    drinksPerHour = 1;
  }
  else
  {
    drinksPerHour = (float)drinkCount / hours;
  }

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
  clock_copy_time_string(firstDrinkStr, BUFFER_LENGTH);
  updateFirstDrinkTextLayerText();
}

static void updateFirstDrinkTextLayerText()
{
  strcpy(firstDrinkDisplayLayer->dynamicStr, firstDrinkStr);
  text_layer_set_text(firstDrinkDisplayLayer->dynamicTextLayer, firstDrinkDisplayLayer->dynamicStr);
  layer_mark_dirty(text_layer_get_layer(firstDrinkDisplayLayer->dynamicTextLayer));
}

static void clearFirstDrinkTextLayerText()
{
  memset(firstDrinkStr, 0, sizeof(firstDrinkStr));
  updateFirstDrinkTextLayerText();
}

static void setLastDrinkTextLayerText()
{
  clock_copy_time_string(lastDrinkStr, BUFFER_LENGTH);
  updateLastDrinkTextLayerText();
}

static void updateLastDrinkTextLayerText()
{
  strcpy(lastDrinkDisplayLater->dynamicStr, lastDrinkStr);
  text_layer_set_text(lastDrinkDisplayLater->dynamicTextLayer, lastDrinkDisplayLater->dynamicStr);
  layer_mark_dirty(text_layer_get_layer(lastDrinkDisplayLater->dynamicTextLayer));
}

static void clearLastDrinkTextLayerText()
{
  memset(lastDrinkStr, 0, sizeof(lastDrinkStr));
  updateLastDrinkTextLayerText();
}

static void resetDrinkCount()
{
  drinkCount = 0;
  startTime = (uint)time(NULL);
  setDrinkCountStr(drinkCount);
  setDrinksPerHourTextLayerText();
  clearFirstDrinkTextLayerText();
  clearLastDrinkTextLayerText();

  persist_delete(DRINK_COUNT_KEY);
  persist_delete(START_TIME_KEY);
  persist_delete(FIRST_DRINK_KEY);
  persist_delete(LAST_DRINK_KEY);
}

static void window_layer_update_callback(Layer *layer, GContext *ctx)
{
  GRect bounds = layer_get_bounds(layer);
  GPoint p0 = GPoint(0, bounds.size.h/2);
  GPoint p1 = GPoint(bounds.size.w, bounds.size.h/2);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_line(ctx, p0, p1);
}
