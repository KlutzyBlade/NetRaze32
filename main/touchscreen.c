#include "touchscreen.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "TOUCHSCREEN";

#define XPT2046_CMD_X   0xD0
#define XPT2046_CMD_Y   0x90

esp_err_t touchscreen_init(void) {
    gpio_config_t out_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << TOUCH_CS_PIN) | (1ULL << TOUCH_SCK_PIN) | (1ULL << TOUCH_MOSI_PIN),
    };
    gpio_config(&out_conf);
    
    gpio_config_t in_conf = {
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << TOUCH_MISO_PIN) | (1ULL << TOUCH_IRQ_PIN),
    };
    gpio_config(&in_conf);
    
    gpio_set_level(TOUCH_CS_PIN, 1);
    gpio_set_level(TOUCH_SCK_PIN, 0);
    gpio_set_level(TOUCH_MOSI_PIN, 0);
    
    ESP_LOGI(TAG, "Touch init: CS=%d IRQ=%d MISO=%d MOSI=%d SCK=%d", 
             TOUCH_CS_PIN, TOUCH_IRQ_PIN, TOUCH_MISO_PIN, TOUCH_MOSI_PIN, TOUCH_SCK_PIN);
    return ESP_OK;
}

bool touchscreen_is_touched(void) {
    return gpio_get_level(TOUCH_IRQ_PIN) == 0;
}

static void touch_write_byte(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        gpio_set_level(TOUCH_MOSI_PIN, (data >> (7 - i)) & 1);
        gpio_set_level(TOUCH_SCK_PIN, 1);
        gpio_set_level(TOUCH_SCK_PIN, 0);
    }
}

static uint16_t touch_read_data(void) {
    uint16_t data = 0;
    for (int i = 0; i < 12; i++) {
        data |= gpio_get_level(TOUCH_MISO_PIN) << (11 - i);
        gpio_set_level(TOUCH_SCK_PIN, 1);
        gpio_set_level(TOUCH_SCK_PIN, 0);
    }
    return data;
}

static uint16_t touch_read_channel(uint8_t cmd) {
    gpio_set_level(TOUCH_CS_PIN, 0);
    touch_write_byte(cmd);
    gpio_set_level(TOUCH_SCK_PIN, 1);
    gpio_set_level(TOUCH_SCK_PIN, 0);
    uint16_t data = touch_read_data();
    gpio_set_level(TOUCH_SCK_PIN, 1);
    gpio_set_level(TOUCH_SCK_PIN, 0);
    gpio_set_level(TOUCH_SCK_PIN, 1);
    gpio_set_level(TOUCH_SCK_PIN, 0);
    gpio_set_level(TOUCH_SCK_PIN, 1);
    gpio_set_level(TOUCH_SCK_PIN, 0);
    gpio_set_level(TOUCH_CS_PIN, 1);
    gpio_set_level(TOUCH_MOSI_PIN, 0);
    return data;
}

touch_point_t touchscreen_get_point(void) {
    touch_point_t point = {0};
    
    // Check IRQ pin first (active low when touched)
    if (gpio_get_level(TOUCH_IRQ_PIN) != 0) {
        point.pressed = false;
        return point;
    }
    
    uint16_t raw_x = touch_read_channel(XPT2046_CMD_X);
    uint16_t raw_y = touch_read_channel(XPT2046_CMD_Y);
    
    // Debounce - read again and check consistency
    vTaskDelay(pdMS_TO_TICKS(1));
    uint16_t raw_x2 = touch_read_channel(XPT2046_CMD_X);
    uint16_t raw_y2 = touch_read_channel(XPT2046_CMD_Y);
    
    if (abs(raw_x - raw_x2) > 100 || abs(raw_y - raw_y2) > 100) {
        point.pressed = false;
        return point;
    }
    
    // Validate range
    if (raw_x > 4000 || raw_y > 4000 || raw_x < 200 || raw_y < 200) {
        point.pressed = false;
        return point;
    }
    
    // Map to screen coordinates (X is inverted)
    point.x = DISPLAY_WIDTH - ((raw_x - TS_MINX) * DISPLAY_WIDTH) / (TS_MAXX - TS_MINX);
    point.y = ((raw_y - TS_MINY) * DISPLAY_HEIGHT) / (TS_MAXY - TS_MINY);
    
    // Clamp to display bounds
    if (point.x < 0) point.x = 0;
    if (point.x >= DISPLAY_WIDTH) point.x = DISPLAY_WIDTH - 1;
    if (point.y < 0) point.y = 0;
    if (point.y >= DISPLAY_HEIGHT) point.y = DISPLAY_HEIGHT - 1;
    
    point.pressed = true;
    return point;
}