#include "bmp_display.h"
#include "display.h"
#include "esp_log.h"

static const char* TAG = "BMP_DISPLAY";

esp_err_t bmp_display_init(void) {
    ESP_LOGI(TAG, "BMP display initialized");
    return ESP_OK;
}

void display_bmp_from_buffer(const uint8_t* bmp_data, size_t data_size, int x, int y) {
    // Simple BMP display implementation
    ESP_LOGI(TAG, "Displaying BMP at (%d, %d), size: %d bytes", x, y, data_size);
    
    // For now, just draw a placeholder rectangle
    display_fill_rect(x, y, 64, 64, COLOR_BLUE);
    display_draw_text(x + 10, y + 30, "BMP", COLOR_WHITE, COLOR_BLUE);
}

void display_bmp_from_file(const char* filename, int x, int y) {
    ESP_LOGI(TAG, "Loading BMP file: %s", filename);
    
    // Placeholder implementation
    display_fill_rect(x, y, 64, 64, COLOR_GREEN);
    display_draw_text(x + 10, y + 30, "IMG", COLOR_WHITE, COLOR_GREEN);
}