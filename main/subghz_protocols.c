#include "subghz_protocols.h"
#include "cc1101_driver.h"
#include "display.h"
#include "touchscreen.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "SUBGHZ_PROTOCOLS";

esp_err_t subghz_protocols_init(void) {
    esp_err_t ret = cc1101_init();
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "CC1101 initialized");
        cc1101_set_frequency(CC1101_FREQ_433);
    } else {
        ESP_LOGW(TAG, "CC1101 not found - SubGHz disabled");
    }
    return ret;
}

void subghz_send_princeton(uint32_t data, uint8_t bit_count, uint16_t te, uint8_t repeat) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Princeton TX", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    char info[64];
    snprintf(info, sizeof(info), "Data: 0x%08lX", data);
    display_draw_text(10, 40, info, COLOR_GREEN, COLOR_BLACK);
    
    snprintf(info, sizeof(info), "Bits: %d", bit_count);
    display_draw_text(10, 60, info, COLOR_BLUE, COLOR_BLACK);
    
    snprintf(info, sizeof(info), "TE: %d us", te);
    display_draw_text(10, 80, info, COLOR_ORANGE, COLOR_BLACK);
    
    display_draw_text(10, 110, "Transmitting...", COLOR_RED, COLOR_BLACK);
    
    // Princeton protocol transmission simulation
    for (int r = 0; r < repeat; r++) {
        // Preamble
        for (int i = 0; i < 10; i++) {
            gpio_set_level(4, 1);
            esp_rom_delay_us(te);
            gpio_set_level(4, 0);
            esp_rom_delay_us(te * 3);
        }
        
        // Data transmission
        for (int i = bit_count - 1; i >= 0; i--) {
            if (data & (1UL << i)) {
                // Bit 1: High for 3*TE, Low for TE
                gpio_set_level(4, 1);
                esp_rom_delay_us(te * 3);
                gpio_set_level(4, 0);
                esp_rom_delay_us(te);
            } else {
                // Bit 0: High for TE, Low for 3*TE
                gpio_set_level(4, 1);
                esp_rom_delay_us(te);
                gpio_set_level(4, 0);
                esp_rom_delay_us(te * 3);
            }
        }
        
        // Sync bit
        gpio_set_level(4, 1);
        esp_rom_delay_us(te);
        gpio_set_level(4, 0);
        esp_rom_delay_us(te * 31);
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    display_draw_text(10, 130, "Transmission complete!", COLOR_GREEN, COLOR_BLACK);
    display_draw_text(10, 280, "Touch to continue", COLOR_GRAY, COLOR_BLACK);
    
    while (true) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.pressed) break;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void subghz_send_came(uint32_t data, uint8_t bit_count) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "CAME TX", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    char info[64];
    snprintf(info, sizeof(info), "Data: 0x%08lX", data);
    display_draw_text(10, 40, info, COLOR_GREEN, COLOR_BLACK);
    
    display_draw_text(10, 70, "Transmitting CAME...", COLOR_RED, COLOR_BLACK);
    
    // CAME protocol (12-bit, 320us timing)
    for (int r = 0; r < 5; r++) {
        for (int i = bit_count - 1; i >= 0; i--) {
            if (data & (1UL << i)) {
                gpio_set_level(4, 1);
                esp_rom_delay_us(640);
                gpio_set_level(4, 0);
                esp_rom_delay_us(320);
            } else {
                gpio_set_level(4, 1);
                esp_rom_delay_us(320);
                gpio_set_level(4, 0);
                esp_rom_delay_us(640);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    
    display_draw_text(10, 90, "CAME transmission done!", COLOR_GREEN, COLOR_BLACK);
    vTaskDelay(pdMS_TO_TICKS(2000));
}

void subghz_send_nice(uint32_t data, uint8_t bit_count) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Nice TX", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 40, "Transmitting Nice...", COLOR_BLUE, COLOR_BLACK);
    
    // Nice FLO protocol (12-bit, 700us timing)
    for (int r = 0; r < 3; r++) {
        for (int i = bit_count - 1; i >= 0; i--) {
            if (data & (1UL << i)) {
                gpio_set_level(4, 1);
                esp_rom_delay_us(1400);
                gpio_set_level(4, 0);
                esp_rom_delay_us(700);
            } else {
                gpio_set_level(4, 1);
                esp_rom_delay_us(700);
                gpio_set_level(4, 0);
                esp_rom_delay_us(1400);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(25));
    }
    
    display_draw_text(10, 60, "Nice transmission done!", COLOR_GREEN, COLOR_BLACK);
    vTaskDelay(pdMS_TO_TICKS(2000));
}

void subghz_send_linear(uint32_t data, uint8_t bit_count) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Linear TX", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 40, "Transmitting Linear...", COLOR_ORANGE, COLOR_BLACK);
    
    // Linear protocol (10-bit, 500us timing)
    for (int r = 0; r < 4; r++) {
        // Preamble
        for (int i = 0; i < 6; i++) {
            gpio_set_level(4, 1);
            esp_rom_delay_us(500);
            gpio_set_level(4, 0);
            esp_rom_delay_us(500);
        }
        
        for (int i = bit_count - 1; i >= 0; i--) {
            if (data & (1UL << i)) {
                gpio_set_level(4, 1);
                esp_rom_delay_us(1500);
                gpio_set_level(4, 0);
                esp_rom_delay_us(500);
            } else {
                gpio_set_level(4, 1);
                esp_rom_delay_us(500);
                gpio_set_level(4, 0);
                esp_rom_delay_us(1500);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(15));
    }
    
    display_draw_text(10, 60, "Linear transmission done!", COLOR_GREEN, COLOR_BLACK);
    vTaskDelay(pdMS_TO_TICKS(2000));
}

void subghz_send_chamberlain(uint32_t data, uint8_t bit_count) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Chamberlain TX", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 40, "Transmitting Security+...", COLOR_PURPLE, COLOR_BLACK);
    
    // Chamberlain Security+ protocol
    for (int r = 0; r < 2; r++) {
        // Preamble burst
        for (int i = 0; i < 40; i++) {
            gpio_set_level(4, 1);
            esp_rom_delay_us(500);
            gpio_set_level(4, 0);
            esp_rom_delay_us(500);
        }
        
        // Data with Manchester encoding
        for (int i = bit_count - 1; i >= 0; i--) {
            if (data & (1UL << i)) {
                gpio_set_level(4, 0);
                esp_rom_delay_us(1000);
                gpio_set_level(4, 1);
                esp_rom_delay_us(1000);
            } else {
                gpio_set_level(4, 1);
                esp_rom_delay_us(1000);
                gpio_set_level(4, 0);
                esp_rom_delay_us(1000);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(30));
    }
    
    display_draw_text(10, 60, "Chamberlain TX done!", COLOR_GREEN, COLOR_BLACK);
    vTaskDelay(pdMS_TO_TICKS(2000));
}

void subghz_send_keeloq(uint64_t data, uint32_t serial, uint16_t counter) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "KeeLoq TX", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 40, "Rolling code system", COLOR_RED, COLOR_BLACK);
    
    char info[64];
    snprintf(info, sizeof(info), "Serial: 0x%08lX", serial);
    display_draw_text(10, 60, info, COLOR_BLUE, COLOR_BLACK);
    
    snprintf(info, sizeof(info), "Counter: %d", counter);
    display_draw_text(10, 80, info, COLOR_GREEN, COLOR_BLACK);
    
    display_draw_text(10, 110, "Transmitting KeeLoq...", COLOR_ORANGE, COLOR_BLACK);
    
    // KeeLoq transmission (simplified)
    uint32_t packet = (uint32_t)(data & 0xFFFFFFFF);
    
    for (int r = 0; r < 3; r++) {
        // Preamble
        for (int i = 0; i < 12; i++) {
            gpio_set_level(4, 1);
            esp_rom_delay_us(400);
            gpio_set_level(4, 0);
            esp_rom_delay_us(400);
        }
        
        // Transmit 66-bit packet
        for (int i = 65; i >= 0; i--) {
            int bit = (i < 32) ? ((packet >> i) & 1) : ((serial >> (i - 32)) & 1);
            
            if (bit) {
                gpio_set_level(4, 1);
                esp_rom_delay_us(1200);
                gpio_set_level(4, 0);
                esp_rom_delay_us(400);
            } else {
                gpio_set_level(4, 1);
                esp_rom_delay_us(400);
                gpio_set_level(4, 0);
                esp_rom_delay_us(1200);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    
    display_draw_text(10, 130, "KeeLoq TX complete!", COLOR_GREEN, COLOR_BLACK);
    vTaskDelay(pdMS_TO_TICKS(3000));
}

void subghz_capture_raw(uint32_t frequency, uint32_t duration_ms) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Raw Capture", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    char info[64];
    snprintf(info, sizeof(info), "Freq: %lu MHz", frequency);
    display_draw_text(10, 40, info, COLOR_GREEN, COLOR_BLACK);
    
    snprintf(info, sizeof(info), "Duration: %lu ms", duration_ms);
    display_draw_text(10, 60, info, COLOR_BLUE, COLOR_BLACK);
    
    display_draw_text(10, 90, "Capturing signal...", COLOR_RED, COLOR_BLACK);
    
    // Raw signal capture simulation
    uint32_t samples = 0;
    uint32_t start_time = xTaskGetTickCount();
    
    while ((xTaskGetTickCount() - start_time) < pdMS_TO_TICKS(duration_ms)) {
        // Simulate signal sampling
        gpio_get_level(16); // Example input pin
        samples++;
        
        if (samples % 1000 == 0) {
            snprintf(info, sizeof(info), "Samples: %lu", samples);
            display_fill_rect(10, 110, 200, 20, COLOR_BLACK);
            display_draw_text(10, 110, info, COLOR_ORANGE, COLOR_BLACK);
        }
        
        esp_rom_delay_us(100);
    }
    
    display_draw_text(10, 140, "Capture complete!", COLOR_GREEN, COLOR_BLACK);
    snprintf(info, sizeof(info), "Total samples: %lu", samples);
    display_draw_text(10, 160, info, COLOR_BLUE, COLOR_BLACK);
    
    display_draw_text(10, 280, "Touch to continue", COLOR_GRAY, COLOR_BLACK);
    
    while (true) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.pressed) break;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}