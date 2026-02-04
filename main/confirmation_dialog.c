#include "confirmation_dialog.h"
#include "display.h"
#include "touchscreen.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

bool confirm_dialog(const char* title, const char* message) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, title, COLOR_ORANGE, COLOR_BLACK);
    display_draw_text(10, 40, message, COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 60, "Are you sure?", COLOR_RED, COLOR_BLACK);
    
    display_fill_rect(40, 120, 70, 35, COLOR_GREEN);
    display_draw_rect(40, 120, 70, 35, COLOR_WHITE);
    display_draw_text(60, 133, "YES", COLOR_WHITE, COLOR_GREEN);
    
    display_fill_rect(130, 120, 70, 35, COLOR_RED);
    display_draw_rect(130, 120, 70, 35, COLOR_WHITE);
    display_draw_text(155, 133, "NO", COLOR_WHITE, COLOR_RED);
    
    while (1) {
        touch_point_t p = touchscreen_get_point();
        if (p.pressed) {
            if (p.y >= 120 && p.y <= 155) {
                if (p.x >= 40 && p.x <= 110) return true;
                if (p.x >= 130 && p.x <= 200) return false;
            }
            vTaskDelay(pdMS_TO_TICKS(300));
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
