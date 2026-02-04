#include "gps_functions.h"
#include "esp_log.h"
#include "display.h"
#include "touchscreen.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

static const char* TAG = "GPS";

esp_err_t gps_init(void) {
    ESP_LOGI(TAG, "GPS initialized");
    return ESP_OK;
}

void gps_get_location(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "GPS Tracker", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 30, "Acquiring satellites...", COLOR_GREEN, COLOR_BLACK);
    
    // Simulate GPS acquisition
    for (int i = 0; i < 50; i++) {
        char sat_info[30];
        snprintf(sat_info, sizeof(sat_info), "Satellites: %d/12", (i / 4) + 1);
        display_fill_rect(10, 50, 200, 15, COLOR_BLACK);
        display_draw_text(10, 50, sat_info, COLOR_WHITE, COLOR_BLACK);
        
        if (i > 30) {
            display_draw_text(10, 70, "GPS Fix acquired!", COLOR_GREEN, COLOR_BLACK);
            display_draw_text(10, 90, "Lat: 37.7749 N", COLOR_WHITE, COLOR_BLACK);
            display_draw_text(10, 110, "Lon: 122.4194 W", COLOR_WHITE, COLOR_BLACK);
            display_draw_text(10, 130, "Alt: 52m", COLOR_WHITE, COLOR_BLACK);
            display_draw_text(10, 150, "Accuracy: 3m", COLOR_GREEN, COLOR_BLACK);
        }
        
        vTaskDelay(pdMS_TO_TICKS(200));
        if (touchscreen_is_touched()) break;
    }
    
    display_draw_text(10, 280, "GPS tracking active", COLOR_GREEN, COLOR_BLACK);
}