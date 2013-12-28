#ifndef HEADER_H
#define HEADER_H

#include <pebble.h>

/************************************************************
 *  DEFINED CONSTANTS
 *  *****************
 *
 *  BUFFER_LENGTH        - c string array size
 *  HOUR_CONVERT     	 - conversion factor for drinks/hr
 *	TEXT_HEIGHT  		 - height for TextLayers
 *
 *  DRINK_COUNT_KEY      - Persistance Key for drinkCount
 *  START_TIME_KEY       - Persistance Key for startTime
 *	FIRST_DRINK_KEY 	 - Persistance Key for firstDrink
 *	LAST_DRINK_KEY  	 - Persistance Key for LastDrink
 *
 *  DRINK_COUNT_DEFAULT  - Default value for drinkCount, 
 *                         if no value has been stored
 *
 *	LOWER_LAYER_COUNT	 - Number of lower layers that can
 *						   be displayed.
 ************************************************************/
#define BUFFER_LENGTH 16
#define HOUR_CONVERT 3600
#define TEXT_HEIGHT 30
//Persistance keys
#define DRINK_COUNT_KEY 0
#define START_TIME_KEY 1
#define FIRST_DRINK_KEY 2
#define LAST_DRINK_KEY 3
//Persistance defaults
#define DRINK_COUNT_DEFAULT 0
 //Number of DisplayLayers
#define LOWER_LAYER_COUNT 3

#endif