#include "led_alerts.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "LED_ALERTS";

// ESP32-32E has no dedicated LED, alerts are visual only
void led_alerts_init(void) {
    ESP_LOGI(TAG, "LED alerts initialized (visual mode)");
}

void led_alert_success(void) {
    ESP_LOGI(TAG, "Success alert");
}

void led_alert_capture(void) {
    ESP_LOGI(TAG, "Capture alert");
}

void led_alert_error(void) {
    ESP_LOGI(TAG, "Error alert");
}

void led_pulse(int duration_ms) {
    ESP_LOGI(TAG, "Pulse alert: %dms", duration_ms);
}
