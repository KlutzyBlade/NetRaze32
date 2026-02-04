#include "nfc_functions.h"
#include "esp_log.h"
#include "display.h"
#include "touchscreen.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

static const char* TAG = "NFC";

esp_err_t nfc_init(void) {
    ESP_LOGI(TAG, "NFC initialized");
    return ESP_OK;
}

void nfc_scan_cards(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "NFC Scanner", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 30, "Scanning for cards...", COLOR_GREEN, COLOR_BLACK);
    
    // Simulate NFC scanning
    for (int i = 0; i < 100; i++) {
        char status[30];
        snprintf(status, sizeof(status), "Scan %d/100", i + 1);
        display_fill_rect(10, 50, 200, 15, COLOR_BLACK);
        display_draw_text(10, 50, status, COLOR_WHITE, COLOR_BLACK);
        
        // Simulate card detection
        if (i == 30) {
            display_draw_text(10, 70, "Card detected!", COLOR_GREEN, COLOR_BLACK);
            display_draw_text(10, 90, "UID: 04:A3:B2:C1", COLOR_WHITE, COLOR_BLACK);
            display_draw_text(10, 110, "Type: MIFARE Classic", COLOR_WHITE, COLOR_BLACK);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
        if (touchscreen_is_touched()) break;
    }
    
    display_draw_text(10, 280, "Scan complete", COLOR_GREEN, COLOR_BLACK);
}