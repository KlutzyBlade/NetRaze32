#include "error_handler.h"
#include "esp_log.h"
#include "display.h"

static const char* TAG = "ERROR_HANDLER";

void error_handler_init(void) {
    ESP_LOGI(TAG, "Error handler initialized");
}

void handle_critical_error(const char* error_msg) {
    ESP_LOGE(TAG, "Critical error: %s", error_msg);
    
    display_fill_screen(COLOR_RED);
    display_draw_text(50, 100, "CRITICAL ERROR", COLOR_WHITE, COLOR_RED);
    display_draw_text(30, 120, error_msg, COLOR_WHITE, COLOR_RED);
    display_draw_text(40, 140, "System Restart", COLOR_WHITE, COLOR_RED);
    
    vTaskDelay(pdMS_TO_TICKS(5000));
    esp_restart();
}

void handle_warning(const char* warning_msg) {
    ESP_LOGW(TAG, "Warning: %s", warning_msg);
}

void log_error(const char* component, esp_err_t error_code) {
    ESP_LOGE(TAG, "%s error: %s", component, esp_err_to_name(error_code));
}