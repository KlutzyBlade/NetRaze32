#include "data_logger.h"
#include "esp_log.h"
#include "display.h"
#include "touchscreen.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

static const char* TAG = "DATA_LOGGER";
static int log_entries = 0;

void data_logger_init(void) {
    ESP_LOGI(TAG, "Data logger initialized");
}

void log_scan_results(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Data Logger", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 30, "Logging to memory", COLOR_GREEN, COLOR_BLACK);
    
    // Simulate logging process
    for (int i = 0; i < 20; i++) {
        log_entries++;
        
        char entry_info[40];
        snprintf(entry_info, sizeof(entry_info), "Entry %d: WiFi scan data", log_entries);
        display_fill_rect(10, 50, 200, 15, COLOR_BLACK);
        display_draw_text(10, 50, entry_info, COLOR_WHITE, COLOR_BLACK);
        
        char timestamp[30];
        snprintf(timestamp, sizeof(timestamp), "Time: %02d:%02d:%02d", 
                (log_entries / 3600) % 24, (log_entries / 60) % 60, log_entries % 60);
        display_fill_rect(10, 70, 200, 15, COLOR_BLACK);
        display_draw_text(10, 70, timestamp, COLOR_GRAY, COLOR_BLACK);
        
        vTaskDelay(pdMS_TO_TICKS(200));
        if (touchscreen_is_touched()) break;
    }
    
    char total_info[30];
    snprintf(total_info, sizeof(total_info), "Total entries: %d", log_entries);
    display_draw_text(10, 280, total_info, COLOR_GREEN, COLOR_BLACK);
}