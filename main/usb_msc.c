#include "usb_msc.h"
#include "sd_card.h"
#include "display.h"
#include "touchscreen.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "USB_MSC";

esp_err_t usb_msc_init(void) {
    ESP_LOGI(TAG, "USB MSC init");
    return ESP_OK;
}

void usb_msc_deinit(void) {
    ESP_LOGI(TAG, "USB MSC deinit");
}

esp_err_t usb_msc_mode_ui(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "USB Mass Storage", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 40, "Feature disabled", COLOR_RED, COLOR_BLACK);
    display_draw_text(10, 60, "TinyUSB not configured", COLOR_ORANGE, COLOR_BLACK);
    display_draw_text(10, 280, "Touch to continue", COLOR_GRAY, COLOR_BLACK);
    while (!touchscreen_get_point().pressed) vTaskDelay(pdMS_TO_TICKS(100));
    return ESP_OK;
}