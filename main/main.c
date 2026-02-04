#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "board_config.h"
#include "display.h"
#include "touchscreen.h"
#include "menu.h"
#include "utils.h"
#include "wifi_functions.h"
#include "bluetooth_functions.h"
#include "rf_functions.h"
#include "ir_functions.h"
#include "nfc_functions.h"
#include "gps_functions.h"
#include "badusb_functions.h"
#include "settings.h"
#include "data_logger.h"
#include "led_alerts.h"
#include "antenna_indicator.h"
#include "oui_spy.h"

static const char* TAG = "MAIN";

// Loading screen with progress
static void display_loading(int progress, uint16_t color) {
    static bool screen_initialized = false;
    
    // Only clear screen once
    if (!screen_initialized) {
        display_fill_screen(COLOR_BLACK);
        
        // Portrait orientation for loading screen
        lcd_cmd(ILI9341_MADCTL);
        lcd_data(0x48); // BGR + MX for correct text in portrait
        
        // NetRaze32 title - 2x size orange (centered)
        display_draw_text_2x(55, 60, "NetRaze32", COLOR_ORANGE, COLOR_BLACK);
        
        // Subtitle (centered)
        display_draw_text(50, 110, "Security Testing Device", COLOR_WHITE, COLOR_BLACK);
        
        // Progress bar frame
        display_draw_rect(40, 160, 160, 20, COLOR_WHITE);
        
        // Version info (centered)
        display_draw_text(105, 260, "v1.0.0", COLOR_GRAY, COLOR_BLACK);
        
        screen_initialized = true;
    }
    
    // Only update progress bar and text
    int bar_width = (progress * 156) / 100;
    display_fill_rect(42, 162, 156, 16, COLOR_BLACK); // Clear bar area
    display_fill_rect(42, 162, bar_width, 16, color); // Draw progress
    
    // Progress text (centered)
    char progress_text[20];
    snprintf(progress_text, sizeof(progress_text), "Loading... %d%%", progress);
    display_fill_rect(70, 190, 100, 10, COLOR_BLACK); // Clear text area
    display_draw_text(85, 190, progress_text, COLOR_GREEN, COLOR_BLACK);
}

void app_main(void) {
    ESP_LOGI(TAG, "NetRaze32 starting...");
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialize critical components
    ESP_ERROR_CHECK(display_init());
    ESP_ERROR_CHECK(touchscreen_init());
    settings_init();
    utils_init();
    menu_init();
    data_logger_init();
    led_alerts_init();
    antenna_set_mode(ANTENNA_INTERNAL);
    
    // Show loading screen with progress
    display_loading(25, COLOR_ORANGE);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    display_loading(50, COLOR_ORANGE);
    wifi_init();
    vTaskDelay(pdMS_TO_TICKS(500));
    
    display_loading(75, COLOR_ORANGE);
    bluetooth_init();
    rf_24ghz_init();
    rf_subghz_init();
    vTaskDelay(pdMS_TO_TICKS(500));
    
    display_loading(90, COLOR_ORANGE);
    ir_init();
    nfc_init();
    gps_init();
    badusb_init();
    oui_spy_init();
    vTaskDelay(pdMS_TO_TICKS(500));
    
    display_loading(100, COLOR_GREEN);
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Restore portrait orientation with X flip
    lcd_cmd(ILI9341_MADCTL);
    lcd_data(0x48); // BGR + MX for correct text orientation
    display_fill_screen(COLOR_BLACK);
    menu_draw();
    
    ESP_LOGI(TAG, "NetRaze32 initialized successfully");
    
    // Main loop
    static uint32_t last_status_update = 0;
    while (1) {
        menu_handle_input();
        
        // Update status bar only every 5 seconds to prevent flicker
        uint32_t current_time = xTaskGetTickCount();
        if (current_time - last_status_update > pdMS_TO_TICKS(5000)) {
            draw_status_bar(read_battery_voltage());
            last_status_update = current_time;
        }
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}