#include "brightness_control.h"
#include "driver/ledc.h"
#include "board_config.h"
#include "display.h"
#include "touchscreen.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static uint8_t brightness = 255;

void brightness_init(void) {
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_2,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer);
    
    ledc_channel_config_t channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_2,
        .timer_sel = LEDC_TIMER_2,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = LCD_BL_PIN,
        .duty = 255,
        .hpoint = 0
    };
    ledc_channel_config(&channel);
}

void brightness_set(uint8_t level) {
    brightness = level;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, level);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
}

uint8_t brightness_get(void) {
    return brightness;
}

esp_err_t brightness_control_ui(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Screen Brightness", COLOR_WHITE, COLOR_BLACK);
    
    uint8_t levels[] = {64, 128, 192, 255};
    const char* labels[] = {"25%", "50%", "75%", "100%"};
    
    for (int i = 0; i < 4; i++) {
        int y = 50 + i * 45;
        uint16_t bg = (brightness == levels[i]) ? COLOR_BLUE : COLOR_DARKGRAY;
        display_fill_rect(60, y, 120, 35, bg);
        display_draw_rect(60, y, 120, 35, COLOR_WHITE);
        display_draw_text(95, y + 12, labels[i], COLOR_WHITE, bg);
    }
    
    display_draw_text(10, 280, "Touch to select", COLOR_GRAY, COLOR_BLACK);
    
    while (1) {
        touch_point_t p = touchscreen_get_point();
        if (p.pressed) {
            for (int i = 0; i < 4; i++) {
                int y = 50 + i * 45;
                if (p.y >= y && p.y <= y + 35 && p.x >= 60 && p.x <= 180) {
                    brightness_set(levels[i]);
                    return ESP_OK;
                }
            }
            vTaskDelay(pdMS_TO_TICKS(300));
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
