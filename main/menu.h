#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    MENU_WIFI = 0,
    MENU_BLUETOOTH,
    MENU_SUBGHZ,
    MENU_IR_REMOTE,
    MENU_NFC_RFID,
    MENU_GPS,
    MENU_BADUSB,
    MENU_AUTOMATION,
    MENU_TOOLS,
    MENU_SETTING,
    MENU_COUNT
} menu_item_t;

typedef struct {
    int16_t x, y, w, h;
    const char* label;
} touch_button_t;

typedef struct {
    int current_index;
    bool in_submenu;
    bool initialized;
} menu_state_t;

void menu_init(void);
void menu_draw(void);
void menu_handle_input(void);
void menu_navigate_up(void);
void menu_navigate_down(void);
void menu_navigate_left(void);
void menu_navigate_right(void);
void menu_select(void);

#endif // MENU_H