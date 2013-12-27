#ifndef HEADER_H
#define HEADER_H

#include <pebble.h>

/************************************************************
 *  DEFINED CONSTANTS
 *  *****************
 *
 *  BUFFER_LENGTH 16     - c string array size
 *  HOUR_CONVERT 3600	 - conversion factor for drinks/hr
 *	TEXT_HEIGHT 30 		 - height for TextLayers
 *
 *  DRINK_COUNT_KEY 0    - Persistance Key for drinkCount
 *  START_TIME_KEY 1     - Persistance Key for startTime
 *
 *  DRINK_COUNT_DEFAULT  - Default value for drinkCount, 
 *                         if no value has been stored
 ************************************************************/
#define BUFFER_LENGTH 16
#define HOUR_CONVERT 3600
#define TEXT_HEIGHT 30
//Persistance keys
#define DRINK_COUNT_KEY 0
#define START_TIME_KEY 1
//Persistance defaults
#define DRINK_COUNT_DEFAULT 0
 //Number of DisplayLayers
#define LOWER_LAYER_COUNT 2

#endif