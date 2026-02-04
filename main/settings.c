#include "settings.h"
#include "esp_log.h"

static const char* TAG = "SETTINGS";

esp_err_t settings_init(void) {
    ESP_LOGI(TAG, "Settings initialized");
    return ESP_OK;
}