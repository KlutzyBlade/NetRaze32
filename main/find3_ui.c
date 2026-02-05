#include "find3_ui.h"
#include "find3_scanner.h"
#include "display.h"
#include "touchscreen.h"
#include "wifi_connect.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
static bool ui_active = false;

#define BTN_START_X 20
#define BTN_START_Y 100
#define BTN_STOP_X 20
#define BTN_STOP_Y 150
#define BTN_LEARN_X 20
#define BTN_LEARN_Y 200
#define BTN_WIDTH 200
#define BTN_HEIGHT 35

static void draw_button(int16_t x, int16_t y, const char *text, uint16_t color) {
    display_fill_rect(x, y, BTN_WIDTH, BTN_HEIGHT, color);
    display_draw_rect(x, y, BTN_WIDTH, BTN_HEIGHT, COLOR_WHITE);
    display_draw_text(x + 10, y + 14, text, COLOR_WHITE, color);
}

static void update_status(void) {
    display_fill_rect(0, 50, DISPLAY_WIDTH, 30, COLOR_BLACK);
    if (find3_is_scanning()) {
        display_draw_text(10, 60, "Status: SCANNING", COLOR_GREEN, COLOR_BLACK);
    } else {
        display_draw_text(10, 60, "Status: IDLE", COLOR_GRAY, COLOR_BLACK);
    }
}

esp_err_t find3_ui_init(void) {
    find3_config_t config = {
        .server_url = "http://192.168.1.50:8003",
        .device_name = "NetRaze32",
        .family_name = "home",
        .location = "unknown",
        .scan_interval_ms = 5000,
        .learning_mode = false,
    };
    return find3_init(&config);
}

void find3_ui_show(void) {
    ui_active = true;
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "FIND3 Indoor Positioning", COLOR_ORANGE, COLOR_BLACK);
    
    draw_button(BTN_START_X, BTN_START_Y, "START SCAN", COLOR_DARKBLUE);
    draw_button(BTN_STOP_X, BTN_STOP_Y, "STOP SCAN", COLOR_DARKGRAY);
    draw_button(BTN_LEARN_X, BTN_LEARN_Y, "LEARN MODE", COLOR_PURPLE);
    
    update_status();
    
    while (ui_active) {
        touch_point_t point = touchscreen_get_point();
        if (point.pressed) {
            if (point.x >= BTN_START_X && point.x <= BTN_START_X + BTN_WIDTH &&
                point.y >= BTN_START_Y && point.y <= BTN_START_Y + BTN_HEIGHT) {
                find3_start_scanning();
                update_status();
                vTaskDelay(pdMS_TO_TICKS(300));
            }
            else if (point.x >= BTN_STOP_X && point.x <= BTN_STOP_X + BTN_WIDTH &&
                     point.y >= BTN_STOP_Y && point.y <= BTN_STOP_Y + BTN_HEIGHT) {
                find3_stop_scanning();
                update_status();
                vTaskDelay(pdMS_TO_TICKS(300));
            }
            else if (point.x >= BTN_LEARN_X && point.x <= BTN_LEARN_X + BTN_WIDTH &&
                     point.y >= BTN_LEARN_Y && point.y <= BTN_LEARN_Y + BTN_HEIGHT) {
                find3_set_learning_mode(true, "living_room");
                vTaskDelay(pdMS_TO_TICKS(300));
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
