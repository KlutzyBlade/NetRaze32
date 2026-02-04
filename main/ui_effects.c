#include "ui_effects.h"
#include "display.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>
#include <string.h>

void ui_matrix_rain(uint16_t duration_ms) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 100, "MATRIX MODE", COLOR_GREEN, COLOR_BLACK);
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
}

void ui_glitch_transition(void) {
    // Simple screen flash effect
    for (int i = 0; i < 3; i++) {
        display_fill_rect(0, 0, DISPLAY_WIDTH, 20, COLOR_RED);
        vTaskDelay(pdMS_TO_TICKS(50));
        display_fill_rect(0, 0, DISPLAY_WIDTH, 20, COLOR_BLACK);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void ui_animated_progress(int x, int y, int width, int progress, uint16_t color) {
    // Background
    display_fill_rect(x, y, width, 8, COLOR_DARKGRAY);
    
    // Progress bar with animated segments
    int filled_width = (progress * width) / 100;
    
    for (int i = 0; i < filled_width; i += 4) {
        uint16_t segment_color = color;
        
        // Animated pulse effect
        uint32_t time = xTaskGetTickCount() / 10;
        if ((time + i) % 20 < 10) {
            segment_color = COLOR_WHITE; // Bright pulse
        }
        
        int segment_width = (i + 4 <= filled_width) ? 4 : (filled_width - i);
        display_fill_rect(x + i, y, segment_width, 8, segment_color);
    }
    
    // Border
    display_draw_rect(x, y, width, 8, COLOR_WHITE);
}

void ui_pulse_element(int x, int y, int width, int height, uint16_t color) {
    uint32_t time = xTaskGetTickCount() / 5;
    float pulse = (sin(time * 0.1f) + 1.0f) / 2.0f; // 0 to 1
    
    uint8_t intensity = (uint8_t)(pulse * 255);
    
    // Create pulsing effect by varying color intensity
    uint16_t pulsed_color = color;
    if (intensity > 128) {
        pulsed_color = COLOR_WHITE;
    }
    
    display_fill_rect(x, y, width, height, pulsed_color);
    display_draw_rect(x, y, width, height, COLOR_GRAY);
}

void ui_scan_lines(void) {
    // Simple animated border
    static uint8_t border_state = 0;
    uint16_t color = (border_state % 2) ? COLOR_GREEN : COLOR_DARKGRAY;
    
    display_draw_rect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, color);
    display_draw_rect(1, 1, DISPLAY_WIDTH-2, DISPLAY_HEIGHT-2, color);
    
    border_state++;
}

void ui_terminal_text(int x, int y, const char* text, uint16_t color, uint16_t delay_ms) {
    int len = strlen(text);
    char buffer[2] = {0, 0};
    
    for (int i = 0; i < len; i++) {
        buffer[0] = text[i];
        display_draw_text(x + i * 6, y, buffer, color, COLOR_BLACK);
        
        // Cursor effect
        display_fill_rect(x + (i + 1) * 6, y, 6, 8, color);
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
        display_fill_rect(x + (i + 1) * 6, y, 6, 8, COLOR_BLACK);
    }
}

void ui_hex_stream(int x, int y, int width, int height) {
    display_draw_text(x, y, "HEX STREAM", COLOR_GREEN, COLOR_BLACK);
}