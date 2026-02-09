#include "utils.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_system.h"
#include "display.h"
#include "touchscreen.h"
#include "board_config.h"
#include "battery_monitor.h"
#include "antenna_indicator.h"
#include "sd_card.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "UTILS";
static adc_oneshot_unit_handle_t adc1_handle;

void utils_init(void) {
    // Initialize ADC for battery monitoring
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
    
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, BATTERY_ADC_CHANNEL, &config));
    
    ESP_LOGI(TAG, "Utils initialized");
}

float read_battery_voltage(void) {
    if (adc1_handle == NULL) {
        ESP_LOGW(TAG, "ADC not initialized");
        return 3.7f;
    }
    
    int adc_raw;
    esp_err_t ret = adc_oneshot_read(adc1_handle, BATTERY_ADC_CHANNEL, &adc_raw);
    if (ret == ESP_OK) {
        // Convert ADC reading to voltage (ESP32 ADC reference is 3.3V)
        float voltage = (adc_raw / 4095.0) * 3.3 * BATTERY_VOLTAGE_DIVIDER;
        return voltage;
    }
    return 3.7f; // Default value if read fails
}

void draw_status_bar(float battery_voltage) {
    display_fill_rect(0, 0, DISPLAY_WIDTH, 20, COLOR_DARKGRAY);
    
    display_draw_text(5, 5, "NetRaze32", COLOR_WHITE, COLOR_DARKGRAY);
    
    // Battery percentage
    float pct = battery_get_percentage();
    char bat_str[8];
    snprintf(bat_str, sizeof(bat_str), "%.0f%%", pct);
    uint16_t bat_color = pct > 50 ? COLOR_GREEN : pct > 20 ? COLOR_ORANGE : COLOR_RED;
    display_draw_text(DISPLAY_WIDTH - 35, 5, bat_str, bat_color, COLOR_DARKGRAY);
    
    // SD card indicator
    if (sd_card_is_mounted()) {
        display_draw_text(DISPLAY_WIDTH - 60, 5, "SD", COLOR_GREEN, COLOR_DARKGRAY);
    }
}

void log_system_stats(void) {
    size_t free_heap = esp_get_free_heap_size();
    size_t min_free_heap = esp_get_minimum_free_heap_size();
    
    ESP_LOGI(TAG, "Free heap: %d bytes, Min free: %d bytes", free_heap, min_free_heap);
}

void enter_safe_mode(void) {
    ESP_LOGW(TAG, "Entering safe mode due to critical memory situation");
    
    display_fill_screen(COLOR_RED);
    display_draw_text(50, 100, "SAFE MODE", COLOR_WHITE, COLOR_RED);
    display_draw_text(30, 120, "Low Memory Detected", COLOR_WHITE, COLOR_RED);
    display_draw_text(40, 140, "Restarting...", COLOR_WHITE, COLOR_RED);
    
    vTaskDelay(pdMS_TO_TICKS(3000));
    esp_restart();
}

void wait_for_touch_with_timeout(int timeout_seconds) {
    int elapsed = 0;
    while (elapsed < timeout_seconds * 10) {
        if (touchscreen_is_touched()) {
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
        elapsed++;
    }
}

void wait_for_back_button(void) {
    display_fill_rect(80, 290, 80, 25, COLOR_BLUE);
    display_draw_text(105, 298, "BACK", COLOR_WHITE, COLOR_BLUE);
    
    while (true) {
        touch_point_t p = touchscreen_get_point();
        if (p.pressed && p.y >= 290 && p.y <= 315 && p.x >= 80 && p.x <= 160) {
            vTaskDelay(pdMS_TO_TICKS(200));
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}