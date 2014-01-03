#include "header.h"
#include "drink_counter.h"
#include "main_display.h"

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

static MainDisplay *mainDisplay;

static void init(void);
static void deinit(void);

int main(void) 
{
  init();
  app_event_loop();
  deinit();
}

static void init(void) 
{
    drink_counter_create();
    drink_counter_persist_load();
    mainDisplay = main_display_create();
    window_set_user_data(mainDisplay->window, mainDisplay);
    main_display_window_init(mainDisplay);
}

static void deinit(void) 
{
    drink_counter_persist_write();
    drink_counter_destroy();
    main_display_destroy(mainDisplay);
}
