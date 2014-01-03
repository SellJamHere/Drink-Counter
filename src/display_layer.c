
#include "display_layer.h"

static void display_layer_set(DisplayLayer* displayLayer);

DisplayLayer* display_layer_create(GRect frame, const char *staticStr)
{
	DisplayLayer *displayLayer = malloc(sizeof(DisplayLayer));

	displayLayer->staticStr = staticStr;
	displayLayer->displayLayer = layer_create(frame);
	displayLayer->staticTextLayer = text_layer_create(GRect(0, 0, frame.size.w, TEXT_HEIGHT));
	GRect topBounds = layer_get_bounds(text_layer_get_layer(displayLayer->staticTextLayer));
  	displayLayer->dynamicTextLayer = text_layer_create(GRect(0, 1 + (topBounds.size.h + topBounds.origin.y), frame.size.w, TEXT_HEIGHT));

  	display_layer_set(displayLayer);

	return displayLayer;
}

static void display_layer_set(DisplayLayer *displayLayer)
{
	//init static text layer
	text_layer_set_text(displayLayer->staticTextLayer, displayLayer->staticStr);
	text_layer_set_font(displayLayer->staticTextLayer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
	layer_add_child(displayLayer->displayLayer, text_layer_get_layer(displayLayer->staticTextLayer));
	//init dynamic text layer
	text_layer_set_text(displayLayer->dynamicTextLayer, displayLayer->dynamicStr);
	text_layer_set_font(displayLayer->dynamicTextLayer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
	text_layer_set_text_alignment(displayLayer->dynamicTextLayer, GTextAlignmentRight);
	layer_add_child(displayLayer->displayLayer, text_layer_get_layer(displayLayer->dynamicTextLayer));
}

void display_layer_destroy(DisplayLayer *displayLayer)
{
	text_layer_destroy(displayLayer->staticTextLayer);
	text_layer_destroy(displayLayer->dynamicTextLayer);

	layer_destroy(displayLayer->displayLayer);

	free(displayLayer);
}