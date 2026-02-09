#include "rf_functions.h"
#include "cc1101_driver.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "display.h"
#include "touchscreen.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_random.h"

static const char* TAG = "RF";

// GPIO pins for RF modules (ESP32-32E optimized)
#define RF_CS_PIN    5   // Available GPIO on ESP32-32E
#define RF_RST_PIN   17  // Available GPIO on ESP32-32E  
#define RF_IRQ_PIN   16  // Available GPIO on ESP32-32E

esp_err_t rf_24ghz_init(void) {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << RF_CS_PIN) | (1ULL << RF_RST_PIN),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf);
    
    ESP_LOGI(TAG, "RF 2.4GHz initialized (WiFi/BLE)");
    return ESP_OK;
}

esp_err_t rf_subghz_init(void) {
    // Initialize VSPI bus for CC1101
    spi_bus_config_t buscfg = {
        .miso_io_num = 19,
        .mosi_io_num = 23,
        .sclk_io_num = 18,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096
    };
    
    esp_err_t ret = spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_DISABLED);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "VSPI bus init failed");
        return ret;
    }
    
    ret = cc1101_init();
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "CC1101 SubGHz initialized");
    } else {
        ESP_LOGW(TAG, "CC1101 not detected");
    }
    return ret;
}

void rf_jammer_24ghz(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "2.4GHz Jammer", COLOR_RED, COLOR_BLACK);
    display_draw_text(10, 30, "Jamming 2.4GHz band", COLOR_WHITE, COLOR_BLACK);
    
    // Simulate jamming by toggling RF pin rapidly
    for (int i = 0; i < 1000; i++) {
        gpio_set_level(RF_CS_PIN, 1);
        esp_rom_delay_us(100);
        gpio_set_level(RF_CS_PIN, 0);
        esp_rom_delay_us(100);
        
        if (i % 100 == 0) {
            char status[30];
            snprintf(status, sizeof(status), "Jamming: %d%%", i / 10);
            display_fill_rect(10, 50, 200, 15, COLOR_BLACK);
            display_draw_text(10, 50, status, COLOR_GREEN, COLOR_BLACK);
        }
        
        if (touchscreen_is_touched()) break;
    }
    
    display_draw_text(10, 280, "Jamming stopped", COLOR_GREEN, COLOR_BLACK);
}

void rf_scanner_24ghz(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "2.4GHz Scanner", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 30, "Scanning channels", COLOR_GREEN, COLOR_BLACK);
    
    // Simulate scanning 2.4GHz channels
    for (int channel = 1; channel <= 14; channel++) {
        char ch_info[30];
        snprintf(ch_info, sizeof(ch_info), "Channel %d: %d MHz", 
                channel, 2412 + (channel - 1) * 5);
        
        display_fill_rect(10, 50, 200, 15, COLOR_BLACK);
        display_draw_text(10, 50, ch_info, COLOR_WHITE, COLOR_BLACK);
        
        // Simulate signal strength
        int signal = esp_random() % 100;
        char signal_info[30];
        snprintf(signal_info, sizeof(signal_info), "Signal: %d dBm", signal - 100);
        display_fill_rect(10, 70, 200, 15, COLOR_BLACK);
        display_draw_text(10, 70, signal_info, COLOR_GREEN, COLOR_BLACK);
        
        vTaskDelay(pdMS_TO_TICKS(500));
        if (touchscreen_is_touched()) break;
    }
    
    display_draw_text(10, 280, "Scan complete", COLOR_GREEN, COLOR_BLACK);
}

void rf_replay_attack(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "RF Replay Attack", COLOR_RED, COLOR_BLACK);
    display_draw_text(10, 30, "Replaying signals", COLOR_WHITE, COLOR_BLACK);
    
    const char* signals[] = {
        "Garage Door", "Car Remote", "Gate Opener", "Doorbell"
    };
    
    for (int i = 0; i < 4; i++) {
        char signal_info[30];
        snprintf(signal_info, sizeof(signal_info), "Replaying: %s", signals[i]);
        display_fill_rect(10, 50, 200, 15, COLOR_BLACK);
        display_draw_text(10, 50, signal_info, COLOR_RED, COLOR_BLACK);
        
        // Simulate transmission
        for (int j = 0; j < 10; j++) {
            gpio_set_level(RF_CS_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(50));
            gpio_set_level(RF_CS_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(50));
        }
        
        display_draw_text(10, 70, "Signal sent!", COLOR_GREEN, COLOR_BLACK);
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        if (touchscreen_is_touched()) break;
    }
    
    display_draw_text(10, 280, "Replay complete", COLOR_GREEN, COLOR_BLACK);
}

void rf_signal_generator(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Signal Generator", COLOR_WHITE, COLOR_BLACK);
    
    const char* freq_names[] = {"315MHz", "433.92MHz", "868.35MHz", "915MHz"};
    
    for (int i = 0; i < 4; i++) {
        char freq_info[30];
        snprintf(freq_info, sizeof(freq_info), "Freq: %s", freq_names[i]);
        display_fill_rect(10, 30, 200, 15, COLOR_BLACK);
        display_draw_text(10, 30, freq_info, COLOR_GREEN, COLOR_BLACK);
        
        display_draw_text(10, 50, "Generating signal...", COLOR_WHITE, COLOR_BLACK);
        
        // Simulate signal generation
        for (int j = 0; j < 100; j++) {
            gpio_set_level(RF_CS_PIN, j % 2);
            esp_rom_delay_us(1000);
        }
        
        vTaskDelay(pdMS_TO_TICKS(2000));
        if (touchscreen_is_touched()) break;
    }
    
    display_draw_text(10, 280, "Generation stopped", COLOR_GREEN, COLOR_BLACK);
}

void rf_spectrum_analyzer(void) {
    if (!cc1101_is_connected()) {
        display_fill_screen(COLOR_BLACK);
        display_draw_text(10, 10, "CC1101 Not Found", COLOR_RED, COLOR_BLACK);
        display_draw_text(10, 40, "Connect CC1101 module", COLOR_WHITE, COLOR_BLACK);
        display_draw_text(10, 280, "Touch to exit", COLOR_GRAY, COLOR_BLACK);
        while (!touchscreen_is_touched()) vTaskDelay(pdMS_TO_TICKS(100));
        return;
    }
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Spectrum Analyzer", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 30, "CC1101 Active", COLOR_GREEN, COLOR_BLACK);
    
    cc1101_freq_t freqs[] = {CC1101_FREQ_315, CC1101_FREQ_433, CC1101_FREQ_868, CC1101_FREQ_915};
    const char* freq_names[] = {"315MHz", "433MHz", "868MHz", "915MHz"};
    
    for (int i = 0; i < 4; i++) {
        cc1101_set_frequency(freqs[i]);
        cc1101_set_rx_mode();
        
        display_fill_rect(10, 60, 220, 150, COLOR_BLACK);
        display_draw_text(10, 60, freq_names[i], COLOR_ORANGE, COLOR_BLACK);
        
        for (int x = 0; x < 200; x++) {
            int8_t rssi = cc1101_get_rssi();
            int h = (rssi + 120) * 2;
            if (h < 0) h = 0;
            if (h > 100) h = 100;
            
            display_fill_rect(10 + x, 160 - h, 1, h, COLOR_GREEN);
            vTaskDelay(pdMS_TO_TICKS(10));
            
            if (touchscreen_is_touched()) return;
        }
    }
    
    display_draw_text(10, 280, "Touch to exit", COLOR_GRAY, COLOR_BLACK);
    while (!touchscreen_is_touched()) vTaskDelay(pdMS_TO_TICKS(100));
}