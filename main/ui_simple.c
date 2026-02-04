#include "ui_simple.h"
#include "display.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void ui_loading_dots(int x, int y, uint16_t color) {
    static uint8_t dot_count = 0;
    
    // Clear area
    display_fill_rect(x, y, 30, 8, COLOR_BLACK);
    
    // Draw dots
    for (int i = 0; i <= (dot_count % 4); i++) {
        display_draw_text(x + i*6, y, ".", color, COLOR_BLACK);
    }
    
    dot_count++;
}

void ui_progress_spinner(int x, int y, uint16_t color) {
    static uint8_t spin_state = 0;
    const char* spinner[] = {"|", "/", "-", "\\"};
    
    display_draw_text(x, y, spinner[spin_state % 4], color, COLOR_BLACK);
    spin_state++;
}

void ui_blink_text(int x, int y, const char* text, uint16_t color) {
    static uint8_t blink_state = 0;
    
    if (blink_state % 10 < 5) {
        display_draw_text(x, y, text, color, COLOR_BLACK);
    } else {
        display_draw_text(x, y, text, COLOR_BLACK, COLOR_BLACK);
    }
    
    blink_state++;
}

void ui_status_indicator(int x, int y, bool active) {
    uint16_t color = active ? COLOR_GREEN : COLOR_RED;
    display_fill_rect(x, y, 8, 8, color);
    display_draw_rect(x, y, 8, 8, COLOR_WHITE);
}