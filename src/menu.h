#ifndef MENU_H
#define MENU_H

#include "header.h"

typedef struct 
{
    Window *window;
} MenuWindow;

MenuWindow* menu_create();
void menu_destroy(MenuWindow *menu);

#endif