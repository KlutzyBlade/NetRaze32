#include "ir_functions.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "display.h"
#include "touchscreen.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "IR";

#define IR_TX_PIN 26  // ESP32-32E compatible GPIO
#define IR_RX_PIN 27  // ESP32-32E compatible GPIO

esp_err_t ir_init(void) {
    // Configure IR TX pin
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << IR_TX_PIN),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf);
    
    // Configure IR RX pin
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << IR_RX_PIN);
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
    
    ESP_LOGI(TAG, "IR initialized");
    return ESP_OK;
}

void ir_send_nec(uint32_t address, uint32_t command) {
    // Simple GPIO-based NEC transmission
    const int pulse_us = 560;
    const int start_pulse_us = 9000;
    const int start_space_us = 4500;
    const int one_space_us = 1690;
    const int zero_space_us = 560;
    
    // Start burst
    gpio_set_level(IR_TX_PIN, 1);
    esp_rom_delay_us(start_pulse_us);
    gpio_set_level(IR_TX_PIN, 0);
    esp_rom_delay_us(start_space_us);
    
    // Send address bits (16 bits)
    for (int i = 0; i < 16; i++) {
        gpio_set_level(IR_TX_PIN, 1);
        esp_rom_delay_us(pulse_us);
        gpio_set_level(IR_TX_PIN, 0);
        
        if (address & (1 << i)) {
            esp_rom_delay_us(one_space_us);
        } else {
            esp_rom_delay_us(zero_space_us);
        }
    }
    
    // Send command bits (16 bits)
    for (int i = 0; i < 16; i++) {
        gpio_set_level(IR_TX_PIN, 1);
        esp_rom_delay_us(pulse_us);
        gpio_set_level(IR_TX_PIN, 0);
        
        if (command & (1 << i)) {
            esp_rom_delay_us(one_space_us);
        } else {
            esp_rom_delay_us(zero_space_us);
        }
    }
    
    // Stop bit
    gpio_set_level(IR_TX_PIN, 1);
    esp_rom_delay_us(pulse_us);
    gpio_set_level(IR_TX_PIN, 0);
}

void ir_tv_power_attack(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "TV Power Attack", COLOR_RED, COLOR_BLACK);
    display_draw_text(10, 30, "Sending power codes", COLOR_WHITE, COLOR_BLACK);
    
    // Common TV power codes
    uint32_t tv_codes[][2] = {
        {0x00FF, 0x02FD}, // Samsung
        {0x0000, 0x08F7}, // LG
        {0x0000, 0x40BF}, // Sony
        {0x0000, 0x20DF}, // Panasonic
        {0x0000, 0x807F}, // Philips
        {0x0000, 0x48B7}, // Sharp
        {0x0000, 0x10EF}, // Toshiba
    };
    
    for (int i = 0; i < 7; i++) {
        char brand_info[30];
        const char* brands[] = {"Samsung", "LG", "Sony", "Panasonic", "Philips", "Sharp", "Toshiba"};
        snprintf(brand_info, sizeof(brand_info), "Trying: %s", brands[i]);
        
        display_fill_rect(10, 50, 200, 15, COLOR_BLACK);
        display_draw_text(10, 50, brand_info, COLOR_GREEN, COLOR_BLACK);
        
        // Send power code 3 times
        for (int j = 0; j < 3; j++) {
            ir_send_nec(tv_codes[i][0], tv_codes[i][1]);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
        if (touchscreen_is_touched()) break;
    }
    
    display_draw_text(10, 280, "Attack complete", COLOR_GREEN, COLOR_BLACK);
}

void ir_ac_attack(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "AC Control Attack", COLOR_RED, COLOR_BLACK);
    display_draw_text(10, 30, "Sending AC codes", COLOR_WHITE, COLOR_BLACK);
    
    // AC control codes
    uint32_t ac_codes[][2] = {
        {0x0000, 0x88}, // Power
        {0x0000, 0x02}, // Temp Up
        {0x0000, 0x82}, // Temp Down
        {0x0000, 0x20}, // Mode
    };
    
    const char* commands[] = {"Power", "Temp Up", "Temp Down", "Mode"};
    
    for (int i = 0; i < 4; i++) {
        char cmd_info[30];
        snprintf(cmd_info, sizeof(cmd_info), "Command: %s", commands[i]);
        
        display_fill_rect(10, 50, 200, 15, COLOR_BLACK);
        display_draw_text(10, 50, cmd_info, COLOR_GREEN, COLOR_BLACK);
        
        ir_send_nec(ac_codes[i][0], ac_codes[i][1]);
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        if (touchscreen_is_touched()) break;
    }
    
    display_draw_text(10, 280, "AC attack complete", COLOR_GREEN, COLOR_BLACK);
}

void ir_universal_remote(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Universal Remote", COLOR_WHITE, COLOR_BLACK);
    
    const char* devices[] = {"TV", "AC", "DVD", "STB"};
    const uint32_t device_codes[] = {0x02FD, 0x08F7, 0x40BF, 0x20DF};
    
    for (int i = 0; i < 4; i++) {
        char device_info[30];
        snprintf(device_info, sizeof(device_info), "Device: %s", devices[i]);
        display_draw_text(10, 50 + i * 20, device_info, COLOR_WHITE, COLOR_BLACK);
    }
    
    display_draw_text(10, 150, "Touch device to control", COLOR_GREEN, COLOR_BLACK);
    
    while (!touchscreen_is_touched()) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    touch_point_t point = touchscreen_get_point();
    int selected = (point.y - 50) / 20;
    
    if (selected >= 0 && selected < 4) {
        char selected_info[30];
        snprintf(selected_info, sizeof(selected_info), "Controlling: %s", devices[selected]);
        display_fill_rect(10, 170, 200, 15, COLOR_BLACK);
        display_draw_text(10, 170, selected_info, COLOR_GREEN, COLOR_BLACK);
        
        ir_send_nec(0x00FF, device_codes[selected]);
        display_draw_text(10, 190, "Command sent!", COLOR_GREEN, COLOR_BLACK);
    }
}

void ir_signal_recorder(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "IR Recorder", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 30, "Point remote and press", COLOR_GREEN, COLOR_BLACK);
    display_draw_text(10, 50, "Recording for 5 seconds", COLOR_WHITE, COLOR_BLACK);
    
    // Simple GPIO-based recording
    for (int i = 0; i < 50; i++) {
        char countdown[20];
        snprintf(countdown, sizeof(countdown), "Time: %d/5s", i / 10);
        display_fill_rect(10, 70, 200, 15, COLOR_BLACK);
        display_draw_text(10, 70, countdown, COLOR_GREEN, COLOR_BLACK);
        
        // Check for IR signal
        if (!gpio_get_level(IR_RX_PIN)) {
            display_draw_text(10, 90, "Signal detected!", COLOR_RED, COLOR_BLACK);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
        if (touchscreen_is_touched()) break;
    }
    
    display_draw_text(10, 280, "Recording complete", COLOR_GREEN, COLOR_BLACK);
}

void ir_jammer(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "IR Jammer", COLOR_RED, COLOR_BLACK);
    display_draw_text(10, 30, "Jamming IR signals", COLOR_WHITE, COLOR_BLACK);
    
    // Send continuous IR noise
    for (int i = 0; i < 1000; i++) {
        gpio_set_level(IR_TX_PIN, 1);
        esp_rom_delay_us(100);
        gpio_set_level(IR_TX_PIN, 0);
        esp_rom_delay_us(100);
        
        if (i % 100 == 0) {
            char status[30];
            snprintf(status, sizeof(status), "Jamming: %d%%", i / 10);
            display_fill_rect(10, 50, 200, 15, COLOR_BLACK);
            display_draw_text(10, 50, status, COLOR_RED, COLOR_BLACK);
        }
        
        if (touchscreen_is_touched()) break;
    }
    
    display_draw_text(10, 280, "Jamming stopped", COLOR_GREEN, COLOR_BLACK);
}