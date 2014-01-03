#include "drink_counter.h"

static DrinkCounter *drinkCounter = NULL;

void drink_counter_create()
{
    drinkCounter = malloc(sizeof(DrinkCounter));

    drinkCounter->drinkCount = 0;
    drinkCounter->startTime = (uint)time(NULL);
    memset(drinkCounter->firstDrinkStr, 0, sizeof(drinkCounter->firstDrinkStr));
    memset(drinkCounter->lastDrinkStr, 0, sizeof(drinkCounter->lastDrinkStr));
}

void drink_counter_destroy()
{
    free(drinkCounter);
    drinkCounter = NULL;
}

void drink_counter_persist_load()
{
    drinkCounter->drinkCount = persist_exists(DRINK_COUNT_KEY) ? persist_read_int(DRINK_COUNT_KEY) : DRINK_COUNT_DEFAULT;
    drinkCounter->startTime = persist_exists(START_TIME_KEY) ? (uint)persist_read_int(START_TIME_KEY) : START_TIME_DEFAULT;
    if(drinkCounter->drinkCount != 0)
    {
        if(persist_exists(FIRST_DRINK_KEY))
        {
            persist_read_string(FIRST_DRINK_KEY, drinkCounter->firstDrinkStr, BUFFER_LENGTH);
        }
        if(persist_exists(LAST_DRINK_KEY))
        {
            persist_read_string(LAST_DRINK_KEY, drinkCounter->lastDrinkStr, BUFFER_LENGTH);
        }
    }
}

void drink_counter_persist_write()
{
    if(drinkCounter->drinkCount != 0)
    {
        //save to persistant storage
        persist_write_int(DRINK_COUNT_KEY, drinkCounter->drinkCount);
        persist_write_int(START_TIME_KEY, drinkCounter->startTime);
        persist_write_string(FIRST_DRINK_KEY, drinkCounter->firstDrinkStr);
        persist_write_string(LAST_DRINK_KEY, drinkCounter->lastDrinkStr);
    }
}

DrinkCounter* drink_counter_get()
{
    return drinkCounter;
}

void drink_counter_reset()
{
    drinkCounter->drinkCount = 0;
    drinkCounter->startTime = (uint)time(NULL);
    memset(drinkCounter->firstDrinkStr, 0, sizeof(drinkCounter->firstDrinkStr));
    memset(drinkCounter->lastDrinkStr, 0, sizeof(drinkCounter->lastDrinkStr));

    persist_delete(DRINK_COUNT_KEY);
    persist_delete(START_TIME_KEY);
    persist_delete(FIRST_DRINK_KEY);
    persist_delete(LAST_DRINK_KEY);
}