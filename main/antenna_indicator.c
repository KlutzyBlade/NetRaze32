#include "antenna_indicator.h"
#include "display.h"
#include "touchscreen.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_phy_init.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static antenna_mode_t current_mode = ANTENNA_INTERNAL;

void antenna_set_mode(antenna_mode_t mode) {
    // Antenna switching is not currently implemented
    // TODO: Implement proper antenna switching for your hardware
    (void)mode;  // Suppress unused parameter warning
}

antenna_mode_t antenna_get_mode(void) {
    return current_mode;
}

void antenna_draw_indicator(int x, int y) {
    uint16_t color = (current_mode == ANTENNA_EXTERNAL) ? COLOR_GREEN : COLOR_GRAY;
    const char* text = (current_mode == ANTENNA_EXTERNAL) ? "EXT" : "INT";
    
    display_draw_rect(x, y, 30, 15, color);
    display_draw_text(x + 5, y + 3, text, color, COLOR_BLACK);
    
    for (int i = 0; i < 3; i++) {
        int h = 3 + i * 3;
        display_fill_rect(x + 32 + i * 4, y + 15 - h, 2, h, color);
    }
}

esp_err_t antenna_toggle_ui(void) {
    bool running = true;
    
    while (running) {
        display_fill_screen(COLOR_BLACK);
        display_draw_text(10, 10, "Antenna Mode", COLOR_WHITE, COLOR_BLACK);
        
        display_fill_rect(40, 60, 160, 40, 
            (current_mode == ANTENNA_INTERNAL) ? COLOR_BLUE : COLOR_DARKGRAY);
        display_draw_text(70, 75, "INTERNAL", COLOR_WHITE, 
            (current_mode == ANTENNA_INTERNAL) ? COLOR_BLUE : COLOR_DARKGRAY);
        
        display_fill_rect(40, 120, 160, 40, 
            (current_mode == ANTENNA_EXTERNAL) ? COLOR_GREEN : COLOR_DARKGRAY);
        display_draw_text(70, 135, "EXTERNAL", COLOR_WHITE, 
            (current_mode == ANTENNA_EXTERNAL) ? COLOR_GREEN : COLOR_DARKGRAY);
        
        display_draw_text(10, 180, "External antenna", COLOR_GRAY, COLOR_BLACK);
        display_draw_text(10, 200, "provides better range", COLOR_GRAY, COLOR_BLACK);
        display_draw_text(10, 220, "and signal strength", COLOR_GRAY, COLOR_BLACK);
        
        display_draw_text(10, 280, "Touch to select", COLOR_ORANGE, COLOR_BLACK);
        
        touch_point_t p = touchscreen_get_point();
        if (p.pressed) {
            if (p.y >= 60 && p.y <= 100 && p.x >= 40 && p.x <= 200) {
                current_mode = ANTENNA_INTERNAL;
                antenna_set_mode(ANTENNA_INTERNAL);
                running = false;
            } else if (p.y >= 120 && p.y <= 160 && p.x >= 40 && p.x <= 200) {
                current_mode = ANTENNA_EXTERNAL;
                antenna_set_mode(ANTENNA_EXTERNAL);
                running = false;
            }
            vTaskDelay(pdMS_TO_TICKS(300));
        }
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    
    return ESP_OK;
}
