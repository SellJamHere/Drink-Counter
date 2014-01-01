#include "main_display.h"

/************************************************************
 *  Constant c-strings used as headers in the display
 ************************************************************/
static const char *DRINK_NUM = "Drink #:";
static const char *DRINK_PER = "Drink/hr:";
static const char *FIRST = "First:";
static const char *LAST = "Last:";

typedef enum {DRINKS_PER_HOUR, FIRST_DRINK, LAST_DRINK} LowerLayerType;

static void window_load(Window *window);
static void window_unload(Window *window);

//Click handlers
static void click_config_provider(void *context);
static void up_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void up_double_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_double_click_handler(ClickRecognizerRef recognizer, void *context);
static void down_click_handler(ClickRecognizerRef recognizer, void *context);

static void drink_layer_load(Layer *parent, MainDisplay *mainDisplay);
static void floatToString(char* buffer, int bufferSize, double number);
static void setDrinkCountStr(MainDisplay *mainDisplay, int count);
static void setDrinkCountTextLayerText(MainDisplay *mainDisplay, int count);
static void setDrinkPerStr(MainDisplay *mainDisplay, uint currentTime);
static void setDrinksPerHourTextLayerText(MainDisplay *mainDisplay);
static void setFirstDrinkTextLayerText(MainDisplay *mainDisplay);
static void updateFirstDrinkTextLayerText(MainDisplay *mainDisplay);
static void clearFirstDrinkTextLayerText(MainDisplay *mainDisplay);
static void setLastDrinkTextLayerText(MainDisplay *mainDisplay);
static void updateLastDrinkTextLayerText(MainDisplay *mainDisplay);
static void clearLastDrinkTextLayerText(MainDisplay *mainDisplay);
static void resetDrinkCount(MainDisplay *mainDisplay);
static void window_layer_update_callback(Layer *layer, GContext *ctx);

MainDisplay *main_display_create()
{
    MainDisplay *display = malloc(sizeof(MainDisplay));
    display->window = window_create();
    display->layerIndex = 0;
    return display;
}

void main_display_destroy(MainDisplay *mainDisplay)
{
    free(mainDisplay->drinkCountDisplayLayer);

    for (int i = 0; i < LOWER_LAYER_COUNT; i++)
    {
        DisplayLayer *lowerLayer = mainDisplay->lowerLayers[i];
        free(lowerLayer);
    }

    // if(menuWindow != NULL)
    // {
    //     menu_destroy(menuWindow);
    //     free(menuWindow);
    // }
    window_destroy(mainDisplay->window);
}

void main_display_window_init(MainDisplay *mainDisplay)
{
    Window *window = mainDisplay->window;

    window_set_click_config_provider(window, click_config_provider);
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });

    // APP_LOG(APP_LOG_LEVEL_DEBUG, "drinkCount: %d,\tstartTime: %d,\tfirstDrink: %s", drinkCount, startTime, firstDrinkStr);
    window_stack_push(window, true);
}

static void window_load(Window *window) 
{
    MainDisplay *mainDisplay = (MainDisplay*)window_get_user_data(window);
    Layer *window_layer = window_get_root_layer(window);

    layer_set_update_proc(window_layer, window_layer_update_callback);

    drink_layer_load(window_layer, mainDisplay);

}



static void window_unload(Window *window) 
{
    // MainDisplay *mainDisplay = (MainDisplay*)window_get_user_data(window);
    // display_layer_destroy(mainDisplay->drinkCountDisplayLayer);
    // display_layer_destroy(mainDisplay->drinksPerHourDisplayLayer);
    // display_layer_destroy(mainDisplay->firstDrinkDisplayLayer);
    // display_layer_destroy(mainDisplay->lastDrinkDisplayLater);
}



static void click_config_provider(void *context) 
{
    window_multi_click_subscribe(BUTTON_ID_SELECT, 2, 0, 200, false, select_double_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
    window_multi_click_subscribe(BUTTON_ID_UP, 2, 0, 200, false, up_double_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) 
{
    Window *window = (Window*)context;
    MainDisplay *mainDisplay = window_get_user_data(window);
    DrinkCounter *drinkCounter = drink_counter_get();

    if(drinkCounter->drinkCount == 0)
    {
        drinkCounter->startTime = (uint)time(NULL);
        setFirstDrinkTextLayerText(mainDisplay);
    }
    drinkCounter->drinkCount++;
    setDrinkCountTextLayerText(mainDisplay, drinkCounter->drinkCount);
    setDrinksPerHourTextLayerText(mainDisplay);
    setLastDrinkTextLayerText(mainDisplay);
}

static void up_double_click_handler(ClickRecognizerRef recognizer, void *context)
{
    Window *window = (Window*)context;
    MainDisplay *mainDisplay = window_get_user_data(window);
    DrinkCounter *drinkCounter = drink_counter_get();

    if(drinkCounter->drinkCount - 1 == 0)
    {
        resetDrinkCount(mainDisplay);
    }
    else if(drinkCounter->drinkCount > 0)
    {
        drinkCounter->drinkCount--;
        setDrinkCountTextLayerText(mainDisplay, drinkCounter->drinkCount);
        setDrinksPerHourTextLayerText(mainDisplay);
    }
}

static void select_single_click_handler(ClickRecognizerRef recognizer, void *context)
{
    // if(menuWindow == NULL)
    // {
    //     menuWindow = menu_create();
    // }
    // window_stack_push(menuWindow->window, true);
}

static void select_double_click_handler(ClickRecognizerRef recognizer, void *context) 
{
    Window *window = (Window*)context;
    MainDisplay *mainDisplay = window_get_user_data(window);

    resetDrinkCount(mainDisplay);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) 
{
    Window *window = (Window*)context;
    MainDisplay *mainDisplay = window_get_user_data(window);

    DisplayLayer *previousDisplayLayer = mainDisplay->lowerLayers[mainDisplay->layerIndex];

    (mainDisplay->layerIndex == LOWER_LAYER_COUNT - 1) ? mainDisplay->layerIndex = DRINKS_PER_HOUR : mainDisplay->layerIndex++;
    DisplayLayer *currentDisplayLayer = mainDisplay->lowerLayers[mainDisplay->layerIndex];

    layer_remove_from_parent(previousDisplayLayer->displayLayer);
    layer_add_child(window_get_root_layer(mainDisplay->window), currentDisplayLayer->displayLayer);
}



static void drink_layer_load(Layer *parent, MainDisplay *mainDisplay)
{
    DrinkCounter *drinkCounter = drink_counter_get();
    GRect parentBounds = layer_get_bounds(parent);

    mainDisplay->drinkCountDisplayLayer = display_layer_create(parentBounds, DRINK_NUM);
    setDrinkCountTextLayerText(mainDisplay, drinkCounter->drinkCount);

    GRect lowerLayerBounds = GRect(0, 1 + (parentBounds.size.h/2), parentBounds.size.w, parentBounds.size.h/2 - 1);
    mainDisplay->lowerLayers[DRINKS_PER_HOUR] = display_layer_create(lowerLayerBounds, DRINK_PER);
    setDrinksPerHourTextLayerText(mainDisplay);

    mainDisplay->lowerLayers[FIRST_DRINK] = display_layer_create(lowerLayerBounds, FIRST);
    updateFirstDrinkTextLayerText(mainDisplay);

    mainDisplay->lowerLayers[LAST_DRINK] = display_layer_create(lowerLayerBounds, LAST);
    updateLastDrinkTextLayerText(mainDisplay);

    layer_add_child(parent, mainDisplay->drinkCountDisplayLayer->displayLayer);
    layer_add_child(parent, mainDisplay->lowerLayers[DRINKS_PER_HOUR]->displayLayer);
}

static void floatToString(char* buffer, int bufferSize, double number)
{
    char decimalBuffer[5];

    snprintf(buffer, bufferSize, "%d", (int)number);
    strcat(buffer, ".");

    snprintf(decimalBuffer, 5, "%02d", (int)((double)(number - (int)number) * (double)100));
    strcat(buffer, decimalBuffer);
}

static void setDrinkCountStr(MainDisplay *mainDisplay, int count)
{
    snprintf(mainDisplay->drinkCountDisplayLayer->dynamicStr, sizeof(mainDisplay->drinkCountDisplayLayer->dynamicStr), "%d", count);
}

static void setDrinkCountTextLayerText(MainDisplay *mainDisplay, int count)
{
    setDrinkCountStr(mainDisplay, count);
    text_layer_set_text(mainDisplay->drinkCountDisplayLayer->dynamicTextLayer, mainDisplay->drinkCountDisplayLayer->dynamicStr);
    layer_mark_dirty(text_layer_get_layer(mainDisplay->drinkCountDisplayLayer->dynamicTextLayer));
}

static void setDrinkPerStr(MainDisplay *mainDisplay, uint currentTime)
{
    DrinkCounter *drinkCounter = drink_counter_get();
    float hours = (currentTime - drinkCounter->startTime) / (float)HOUR_CONVERT;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "currentTime: %d\tstartTime: %d", currentTime, drinkCounter->startTime);
    float drinksPerHour;
    if(drinkCounter->drinkCount == 1)
    {
        drinksPerHour = 1;
    }
    else
    {
        drinksPerHour = (float)drinkCounter->drinkCount / hours;
    }

    floatToString(mainDisplay->lowerLayers[DRINKS_PER_HOUR]->dynamicStr, BUFFER_LENGTH, drinksPerHour);
}

static void setDrinksPerHourTextLayerText(MainDisplay *mainDisplay)
{
    setDrinkPerStr(mainDisplay, (uint)time(NULL));
    text_layer_set_text(mainDisplay->lowerLayers[DRINKS_PER_HOUR]->dynamicTextLayer, mainDisplay->lowerLayers[DRINKS_PER_HOUR]->dynamicStr);
    layer_mark_dirty(text_layer_get_layer(mainDisplay->lowerLayers[DRINKS_PER_HOUR]->dynamicTextLayer));
}

static void setFirstDrinkTextLayerText(MainDisplay *mainDisplay)
{
    DrinkCounter *drinkCounter = drink_counter_get();
    clock_copy_time_string(drinkCounter->firstDrinkStr, BUFFER_LENGTH);
    updateFirstDrinkTextLayerText(mainDisplay);
}

static void updateFirstDrinkTextLayerText(MainDisplay *mainDisplay)
{
    DrinkCounter *drinkCounter = drink_counter_get();
    DisplayLayer *firstDrinkDisplayLayer = mainDisplay->lowerLayers[FIRST_DRINK];
    strcpy(firstDrinkDisplayLayer->dynamicStr, drinkCounter->firstDrinkStr);
    text_layer_set_text(firstDrinkDisplayLayer->dynamicTextLayer, firstDrinkDisplayLayer->dynamicStr);
    layer_mark_dirty(text_layer_get_layer(firstDrinkDisplayLayer->dynamicTextLayer));
}

static void clearFirstDrinkTextLayerText(MainDisplay *mainDisplay)
{
    DrinkCounter *drinkCounter = drink_counter_get();
    memset(drinkCounter->firstDrinkStr, 0, sizeof(drinkCounter->firstDrinkStr));
    updateFirstDrinkTextLayerText(mainDisplay);
}

static void setLastDrinkTextLayerText(MainDisplay *mainDisplay)
{
    DrinkCounter *drinkCounter = drink_counter_get();
    clock_copy_time_string(drinkCounter->lastDrinkStr, BUFFER_LENGTH);
    updateLastDrinkTextLayerText(mainDisplay);
}

static void updateLastDrinkTextLayerText(MainDisplay *mainDisplay)
{
    DrinkCounter *drinkCounter = drink_counter_get();
    DisplayLayer *lastDrinkDisplayLater = mainDisplay->lowerLayers[LAST_DRINK];
    strcpy(lastDrinkDisplayLater->dynamicStr, drinkCounter->lastDrinkStr);
    text_layer_set_text(lastDrinkDisplayLater->dynamicTextLayer, lastDrinkDisplayLater->dynamicStr);
    layer_mark_dirty(text_layer_get_layer(lastDrinkDisplayLater->dynamicTextLayer));
}

static void clearLastDrinkTextLayerText(MainDisplay *mainDisplay)
{
    DrinkCounter *drinkCounter = drink_counter_get();
    memset(drinkCounter->lastDrinkStr, 0, sizeof(drinkCounter->lastDrinkStr));
    updateLastDrinkTextLayerText(mainDisplay);
}

static void resetDrinkCount(MainDisplay *mainDisplay)
{
    DrinkCounter *drinkCounter = drink_counter_get();
    drink_counter_reset(drinkCounter);
    setDrinkCountStr(mainDisplay, drinkCounter->drinkCount);
    setDrinksPerHourTextLayerText(mainDisplay);
    clearFirstDrinkTextLayerText(mainDisplay);
    clearLastDrinkTextLayerText(mainDisplay);
}

static void window_layer_update_callback(Layer *layer, GContext *ctx)
{
    GRect bounds = layer_get_bounds(layer);
    GPoint p0 = GPoint(0, bounds.size.h/2);
    GPoint p1 = GPoint(bounds.size.w, bounds.size.h/2);
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_draw_line(ctx, p0, p1);
}