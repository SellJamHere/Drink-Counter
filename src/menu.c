#include "menu.h"

static void window_load(Window *window);
static void window_unload(Window *window);

MenuWindow* menu_create()
{
    MenuWindow *menu = malloc(sizeof(MenuWindow));

    Window *window = window_create();
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload
    });

    menu->window = window;

    return menu;
}

static void window_load(Window *window) 
{

}

static void window_unload(Window *window) 
{

}

void menu_destroy(MenuWindow *menu)
{
    window_destroy(menu->window);
}