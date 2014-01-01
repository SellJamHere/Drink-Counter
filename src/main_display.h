#ifndef MAIN_DISPLAY_H
#define MAIN_DISPLAY_H

#include "header.h"
#include "drink_counter.h"
#include "display_layer.h"
#include "menu.h"

typedef struct 
{
    Window *window;

    DisplayLayer *drinkCountDisplayLayer;
    DisplayLayer *lowerLayers[LOWER_LAYER_COUNT];
    int layerIndex;

} MainDisplay;

MainDisplay * main_display_create();
void main_display_destroy(MainDisplay *mainDisplay);
void main_display_window_init(MainDisplay *mainDisplay);

#endif