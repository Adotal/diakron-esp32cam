#ifndef SERVICE_UI_H
#define SERVICE_UI_H

#include <Arduino.h>

// Estados
enum UIState {
    UI_MENU,
    UI_VIEW_SENSORS,
    UI_VIEW_LEVELS,
    UI_CAMERA_TEST,
    UI_WIFI_STATUS
};

//Estructura MENU

typedef struct {
    const char* label;
    UIState targetState;
} MenuItem;


extern MenuItem menuItems[];
extern const uint8_t menuLength;

// Logo Diakron Inicio
#define LOGO_WIDTH    84
#define LOGO_HEIGHT   52

extern const unsigned char logo[];

// Metodos
bool service_ui_init();
void service_ui_update();
void service_ui_enable(bool enable);
bool service_ui_isActive();
void ui_nextItem();
void ui_select();
UIState ui_getState();

#endif
