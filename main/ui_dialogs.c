#include "ui_dialogs.h"
#include "esp_log.h"

static const char* TAG = "UI_DIALOGS";

bool confirm_dialog(const char* title, const char* message) {
    ESP_LOGI(TAG, "Confirm: %s - %s", title, message);
    // Always return true for now
    return true;
}
