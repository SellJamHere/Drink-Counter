#ifndef DRINK_COUNTER_H
#define DRINK_COUNTER_H

#include "header.h" 

typedef struct
{
    /************************************************************
     *  int drinkCount
     *  uint startTime
     *  char firstDrinkStr[]
     *  char lastDrinkStr[]
     ************************************************************/
    int drinkCount;
    uint startTime;
    char firstDrinkStr[BUFFER_LENGTH];
    char lastDrinkStr[BUFFER_LENGTH];

} DrinkCounter;

void drink_counter_create();
void drink_counter_destroy();
void drink_counter_persist_load();
void drink_counter_persist_write();

DrinkCounter* drink_counter_get();

void drink_counter_reset();

#endif