#include "settings_menu.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "display.h"
#include "touchscreen.h"
#include "board_config.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <string.h>
#include <stdio.h>

static const char* TAG = "SETTINGS_MENU";
static const char* NVS_NAMESPACE = "settings";
static app_settings_t settings = {
    .brightness = 80,
    .attack_power = 100,
    .auto_save = true,
    .stealth_mode = false,
    .default_channel = 6,
    .device_name = "NetRaze32",
    .show_battery = true,
    .sound_enabled = false
};

void settings_menu_init(void) {
    settings_menu_load();
    ESP_LOGI(TAG, "Settings menu initialized");
}

app_settings_t* settings_menu_get(void) {
    return &settings;
}

bool settings_menu_save(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS");
        return false;
    }
    
    nvs_set_blob(nvs_handle, "settings", &settings, sizeof(app_settings_t));
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    
    ESP_LOGI(TAG, "Settings saved to NVS");
    return true;
}

bool settings_menu_load(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "No saved settings, using defaults");
        return false;
    }
    
    size_t required_size = sizeof(app_settings_t);
    err = nvs_get_blob(nvs_handle, "settings", &settings, &required_size);
    nvs_close(nvs_handle);
    
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Settings loaded from NVS");
        return true;
    }
    return false;
}

static void draw_slider(int x, int y, int width, uint8_t value, const char* label) {
    // Label
    display_draw_text(x, y, label, COLOR_WHITE, COLOR_BLACK);
    
    // Slider background
    display_draw_rect(x, y + 15, width, 10, COLOR_GRAY);
    
    // Slider fill
    int fill_width = (width - 4) * value / 100;
    display_fill_rect(x + 2, y + 17, fill_width, 6, COLOR_GREEN);
    
    // Value text
    char val_str[8];
    snprintf(val_str, sizeof(val_str), "%d%%", value);
    display_draw_text(x + width + 5, y + 15, val_str, COLOR_BLUE, COLOR_BLACK);
}

static void draw_toggle(int x, int y, bool enabled, const char* label) {
    display_draw_text(x, y, label, COLOR_WHITE, COLOR_BLACK);
    
    // Toggle switch
    uint16_t bg_color = enabled ? COLOR_GREEN : COLOR_GRAY;
    display_fill_rect(x + 150, y, 40, 15, bg_color);
    display_draw_rect(x + 150, y, 40, 15, COLOR_WHITE);
    
    const char* state = enabled ? "ON" : "OFF";
    display_draw_text(x + 155, y + 3, state, COLOR_WHITE, bg_color);
}

void settings_menu_ui(void) {
    bool running = true;
    bool needs_redraw = true;
    static uint32_t last_touch_time = 0;
    
    while (running) {
        if (needs_redraw) {
            display_fill_screen(COLOR_BLACK);
            display_draw_text(10, 10, "Settings", COLOR_WHITE, COLOR_BLACK);
            display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
            
            // Brightness slider
            draw_slider(10, 35, 150, settings.brightness, "Brightness");
            
            // Attack power slider
            draw_slider(10, 65, 150, settings.attack_power, "Attack Power");
            
            // Auto-save toggle
            draw_toggle(10, 95, settings.auto_save, "Auto-Save");
            
            // Stealth mode toggle
            draw_toggle(10, 115, settings.stealth_mode, "Stealth Mode");
            
            // Battery display toggle
            draw_toggle(10, 135, settings.show_battery, "Show Battery");
            
            // Device name
            display_draw_text(10, 155, "Device Name:", COLOR_WHITE, COLOR_BLACK);
            display_draw_text(10, 170, settings.device_name, COLOR_GREEN, COLOR_BLACK);
            
            // Channel
            char ch_str[32];
            snprintf(ch_str, sizeof(ch_str), "Default Channel: %d", settings.default_channel);
            display_draw_text(10, 190, ch_str, COLOR_WHITE, COLOR_BLACK);
            
            // Buttons
            display_fill_rect(10, 240, 70, 25, COLOR_GREEN);
            display_draw_text(25, 248, "SAVE", COLOR_WHITE, COLOR_GREEN);
            
            display_fill_rect(90, 240, 70, 25, COLOR_ORANGE);
            display_draw_text(100, 248, "RESET", COLOR_WHITE, COLOR_ORANGE);
            
            display_fill_rect(170, 240, 60, 25, COLOR_BLUE);
            display_draw_text(185, 248, "BACK", COLOR_WHITE, COLOR_BLUE);
            
            needs_redraw = false;
        }
        
        // Handle input with debouncing
        uint32_t current_time = xTaskGetTickCount();
        touch_point_t point = touchscreen_get_point();
        if (point.pressed && (current_time - last_touch_time > pdMS_TO_TICKS(300))) {
            // Brightness slider
            if (point.y >= 50 && point.y <= 60 && point.x >= 10 && point.x <= 160) {
                settings.brightness = ((point.x - 10) * 100) / 150;
                if (settings.brightness > 100) settings.brightness = 100;
                needs_redraw = true;
                last_touch_time = current_time;
            }
            
            // Attack power slider
            else if (point.y >= 80 && point.y <= 90 && point.x >= 10 && point.x <= 160) {
                settings.attack_power = ((point.x - 10) * 100) / 150;
                if (settings.attack_power > 100) settings.attack_power = 100;
                needs_redraw = true;
                last_touch_time = current_time;
            }
            
            // Auto-save toggle
            else if (point.y >= 95 && point.y <= 110 && point.x >= 150 && point.x <= 190) {
                settings.auto_save = !settings.auto_save;
                needs_redraw = true;
                last_touch_time = current_time;
            }
            
            // Stealth mode toggle
            else if (point.y >= 115 && point.y <= 130 && point.x >= 150 && point.x <= 190) {
                settings.stealth_mode = !settings.stealth_mode;
                needs_redraw = true;
                last_touch_time = current_time;
            }
            
            // Battery toggle
            else if (point.y >= 135 && point.y <= 150 && point.x >= 150 && point.x <= 190) {
                settings.show_battery = !settings.show_battery;
                needs_redraw = true;
                last_touch_time = current_time;
            }
            
            // Channel adjustment
            else if (point.y >= 190 && point.y <= 205) {
                if (point.x >= 10 && point.x <= 120) {
                    settings.default_channel++;
                    if (settings.default_channel > 14) settings.default_channel = 1;
                    needs_redraw = true;
                    last_touch_time = current_time;
                }
            }
            
            // Buttons
            else if (point.y >= 240 && point.y <= 265) {
                if (point.x >= 10 && point.x <= 80) {
                    // Save
                    settings_menu_save();
                    display_fill_rect(10, 215, 220, 20, COLOR_BLACK);
                    display_draw_text(10, 215, "Settings saved!", COLOR_GREEN, COLOR_BLACK);
                    last_touch_time = current_time;
                    vTaskDelay(pdMS_TO_TICKS(1500));
                } else if (point.x >= 90 && point.x <= 160) {
                    // Reset to defaults
                    settings.brightness = 80;
                    settings.attack_power = 100;
                    settings.auto_save = true;
                    settings.stealth_mode = false;
                    settings.default_channel = 6;
                    settings.show_battery = true;
                    strcpy(settings.device_name, "NetRaze32");
                    needs_redraw = true;
                    last_touch_time = current_time;
                } else if (point.x >= 170 && point.x <= 230) {
                    // Back
                    running = false;
                    last_touch_time = current_time;
                }
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
