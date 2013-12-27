#ifndef DISPLAY_LAYER_H
#define DISPLAY_LAYER_H

#include "header.h"

typedef struct
{
  Layer *displayLayer;
  TextLayer *staticTextLayer;
  TextLayer *dynamicTextLayer;
} DisplayLayer;

DisplayLayer display_layer_create(GRect frame, const char *staticStr, char dynamicStr[]);

#endif