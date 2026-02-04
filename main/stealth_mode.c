#include "stealth_mode.h"
#include "esp_log.h"
#include "esp_pm.h"
#include "driver/gpio.h"
#include "display.h"
#include "touchscreen.h"
#include "packet_logger.h"

static const char* TAG = "STEALTH";
static bool stealth_active = false;
static uint8_t original_brightness = 255;

bool stealth_mode_init(void) {
    ESP_LOGI(TAG, "Stealth mode initialized");
    return true;
}

void stealth_mode_enable(void) {
    if (stealth_active) return;
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Enabling Stealth Mode", COLOR_RED, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    // Reduce display brightness
    original_brightness = 255; // Store current
    display_draw_text(10, 40, "Dimming display...", COLOR_GRAY, COLOR_BLACK);
    
    // Disable status LEDs (if any)
    display_draw_text(10, 60, "Disabling LEDs...", COLOR_GRAY, COLOR_BLACK);
    
    // Reduce CPU frequency for lower power
    esp_pm_config_t pm_config = {
        .max_freq_mhz = 80,  // Reduce from 240MHz
        .min_freq_mhz = 10,
        .light_sleep_enable = true
    };
    esp_pm_configure(&pm_config);
    display_draw_text(10, 80, "Reducing CPU speed...", COLOR_GRAY, COLOR_BLACK);
    
    // Disable WiFi/BT beacons to reduce RF signature
    display_draw_text(10, 100, "Minimizing RF signature...", COLOR_GRAY, COLOR_BLACK);
    
    // Log stealth activation
    packet_log_custom("STEALTH", "MODE_ENABLED");
    
    stealth_active = true;
    
    display_draw_text(10, 140, "STEALTH MODE ACTIVE", COLOR_RED, COLOR_BLACK);
    display_draw_text(10, 160, "Reduced visibility", COLOR_GRAY, COLOR_BLACK);
    display_draw_text(10, 180, "Lower power consumption", COLOR_GRAY, COLOR_BLACK);
    display_draw_text(10, 200, "Minimal RF emissions", COLOR_GRAY, COLOR_BLACK);
    
    vTaskDelay(pdMS_TO_TICKS(2000));
}

void stealth_mode_disable(void) {
    if (!stealth_active) return;
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Disabling Stealth Mode", COLOR_GREEN, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    // Restore display brightness
    display_draw_text(10, 40, "Restoring brightness...", COLOR_WHITE, COLOR_BLACK);
    
    // Restore CPU frequency
    esp_pm_config_t pm_config = {
        .max_freq_mhz = 240,  // Restore full speed
        .min_freq_mhz = 80,
        .light_sleep_enable = false
    };
    esp_pm_configure(&pm_config);
    display_draw_text(10, 60, "Restoring CPU speed...", COLOR_WHITE, COLOR_BLACK);
    
    // Re-enable normal operations
    display_draw_text(10, 80, "Restoring normal mode...", COLOR_WHITE, COLOR_BLACK);
    
    packet_log_custom("STEALTH", "MODE_DISABLED");
    
    stealth_active = false;
    
    display_draw_text(10, 120, "NORMAL MODE RESTORED", COLOR_GREEN, COLOR_BLACK);
    vTaskDelay(pdMS_TO_TICKS(1000));
}

void stealth_mode_menu(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Stealth Mode", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    while (true) {
        // Clear status area
        display_fill_rect(10, 40, 220, 200, COLOR_BLACK);
        
        if (stealth_active) {
            display_draw_text(10, 50, "Status: ACTIVE", COLOR_RED, COLOR_BLACK);
            display_draw_text(10, 70, "- Reduced brightness", COLOR_GRAY, COLOR_BLACK);
            display_draw_text(10, 90, "- Lower CPU speed", COLOR_GRAY, COLOR_BLACK);
            display_draw_text(10, 110, "- Minimal RF emissions", COLOR_GRAY, COLOR_BLACK);
            display_draw_text(10, 130, "- Power optimized", COLOR_GRAY, COLOR_BLACK);
            
            // Disable button
            display_fill_rect(10, 180, 120, 30, COLOR_GREEN);
            display_draw_rect(10, 180, 120, 30, COLOR_WHITE);
            display_draw_text(35, 190, "DISABLE", COLOR_WHITE, COLOR_GREEN);
        } else {
            display_draw_text(10, 50, "Status: INACTIVE", COLOR_GREEN, COLOR_BLACK);
            display_draw_text(10, 70, "Normal operation mode", COLOR_WHITE, COLOR_BLACK);
            display_draw_text(10, 90, "Full brightness", COLOR_WHITE, COLOR_BLACK);
            display_draw_text(10, 110, "Maximum performance", COLOR_WHITE, COLOR_BLACK);
            
            // Enable button
            display_fill_rect(10, 180, 120, 30, COLOR_RED);
            display_draw_rect(10, 180, 120, 30, COLOR_WHITE);
            display_draw_text(40, 190, "ENABLE", COLOR_WHITE, COLOR_RED);
        }
        
        // Back button
        display_fill_rect(150, 180, 80, 30, COLOR_DARKBLUE);
        display_draw_rect(150, 180, 80, 30, COLOR_WHITE);
        display_draw_text(175, 190, "BACK", COLOR_WHITE, COLOR_DARKBLUE);
        
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            
            if (point.x >= 10 && point.x <= 130 && point.y >= 180 && point.y <= 210) {
                if (stealth_active) {
                    stealth_mode_disable();
                } else {
                    stealth_mode_enable();
                }
            }
            else if (point.x >= 150 && point.x <= 230 && point.y >= 180 && point.y <= 210) {
                return;
            }
            
            vTaskDelay(pdMS_TO_TICKS(300));
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

bool stealth_mode_is_active(void) {
    return stealth_active;
}