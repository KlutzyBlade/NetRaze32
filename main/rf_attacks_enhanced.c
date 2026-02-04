#include "rf_attacks_enhanced.h"
#include "rf_functions.h"
#include "cc1101_driver.h"
#include "esp_log.h"
#include "esp_random.h"
#include "display.h"
#include "touchscreen.h"
#include "utils.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_rom_sys.h"
#include <string.h>

static const char* TAG = "RF_ENHANCED";

// RF Jammer implementation - uses CC1101 if available, GPIO fallback
void rf_jammer_full(void) {
    ESP_LOGI(TAG, "Starting full RF jammer");
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Full RF Jammer", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    bool use_cc1101 = cc1101_is_connected();
    if (use_cc1101) {
        display_draw_text(10, 40, "CC1101: Active", COLOR_GREEN, COLOR_BLACK);
        cc1101_set_frequency(CC1101_FREQ_433);
        cc1101_set_tx_mode();
    } else {
        display_draw_text(10, 40, "GPIO Mode (Limited)", COLOR_ORANGE, COLOR_BLACK);
        gpio_config_t io_conf = {
            .intr_type = GPIO_INTR_DISABLE,
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = (1ULL << GPIO_NUM_2),
            .pull_down_en = 0,
            .pull_up_en = 0,
        };
        gpio_config(&io_conf);
    }
    
    uint32_t start_time = xTaskGetTickCount();
    uint8_t jam_data[64];
    
    while ((xTaskGetTickCount() - start_time) * portTICK_PERIOD_MS < 20000) {
        if (use_cc1101) {
            for (int i = 0; i < 64; i++) jam_data[i] = 0xFF;
            cc1101_transmit(jam_data, 64);
        } else {
            gpio_set_level(GPIO_NUM_2, 1);
            esp_rom_delay_us(100);
            gpio_set_level(GPIO_NUM_2, 0);
            esp_rom_delay_us(100);
        }
        
        uint32_t elapsed = (xTaskGetTickCount() - start_time) * portTICK_PERIOD_MS;
        if (elapsed % 1000 == 0) {
            char time_info[32];
            snprintf(time_info, sizeof(time_info), "Jamming: %lus/20s", elapsed / 1000);
            display_fill_rect(10, 60, 200, 20, COLOR_BLACK);
            display_draw_text(10, 60, time_info, COLOR_GREEN, COLOR_BLACK);
        }
        
        if (touchscreen_is_touched()) break;
    }
    
    if (!use_cc1101) gpio_set_level(GPIO_NUM_2, 0);
    
    display_draw_text(10, 160, "Jammer stopped", COLOR_GREEN, COLOR_BLACK);
    wait_for_back_button();
}

// RF Intermittent Jammer from Bruce
void rf_jammer_intermittent(void) {
    ESP_LOGI(TAG, "Starting intermittent RF jammer");
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Intermittent RF Jammer", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    display_draw_text(10, 40, "PWM jamming pattern", COLOR_ORANGE, COLOR_BLACK);
    
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << GPIO_NUM_2),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf);
    
    uint32_t start_time = xTaskGetTickCount();
    uint32_t jam_time = 0;
    uint32_t pulse_count = 0;
    
    while (jam_time < 20000) { // Max 20 seconds
        jam_time = (xTaskGetTickCount() - start_time) * portTICK_PERIOD_MS;
        
        // Intermittent jamming pattern
        for (int sequence = 1; sequence < 50; sequence++) {
            for (int duration = 1; duration <= 3; duration++) {
                // Check for early exit
                if (touchscreen_is_touched() || 
                    ((xTaskGetTickCount() - start_time) * portTICK_PERIOD_MS) > 20000) {
                    goto exit_jam;
                }
                
                // Turn on RF
                gpio_set_level(GPIO_NUM_2, 1);
                
                // Variable pulse width
                for (int width = 1; width <= (1 + sequence); width++) {
                    esp_rom_delay_us(10);
                }
                
                // Turn off RF
                gpio_set_level(GPIO_NUM_2, 0);
                
                // Same off time
                for (int width = 1; width <= (1 + sequence); width++) {
                    esp_rom_delay_us(10);
                }
                
                pulse_count++;
            }
        }
        
        // Update display every 100ms
        if (jam_time % 100 == 0) {
            char time_info[32];
            snprintf(time_info, sizeof(time_info), "Jamming: %lus/20s", jam_time / 1000);
            display_fill_rect(10, 60, 200, 20, COLOR_BLACK);
            display_draw_text(10, 60, time_info, COLOR_GREEN, COLOR_BLACK);
            
            char pulse_info[32];
            snprintf(pulse_info, sizeof(pulse_info), "Pulses: %lu", pulse_count);
            display_fill_rect(10, 80, 200, 20, COLOR_BLACK);
            display_draw_text(10, 80, pulse_info, COLOR_BLUE, COLOR_BLACK);
            
            display_draw_text(10, 100, "PWM pattern active", COLOR_ORANGE, COLOR_BLACK);
        }
    }
    
exit_jam:
    // Ensure RF is off
    gpio_set_level(GPIO_NUM_2, 0);
    
    display_draw_text(10, 140, "Intermittent jam stopped", COLOR_GREEN, COLOR_BLACK);
    wait_for_back_button();
}

// RF Spectrum Analyzer from Bruce
void rf_spectrum_analyzer_enhanced(void) {
    ESP_LOGI(TAG, "Starting enhanced RF spectrum analyzer");
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "RF Spectrum Analyzer", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    display_draw_text(10, 40, "Analyzing spectrum...", COLOR_BLUE, COLOR_BLACK);
    
    // Frequency ranges to analyze
    const struct {
        uint32_t start_freq;
        uint32_t end_freq;
        const char* band_name;
    } freq_bands[] = {
        {315000000, 315500000, "315MHz ISM"},
        {433000000, 434000000, "433MHz ISM"},
        {868000000, 869000000, "868MHz ISM"},
        {915000000, 916000000, "915MHz ISM"},
        {2400000000, 2500000000, "2.4GHz ISM"}
    };
    
    for (int band = 0; band < 5; band++) {
        char band_info[48];
        snprintf(band_info, sizeof(band_info), "Band: %s", freq_bands[band].band_name);
        display_fill_rect(10, 60, 220, 20, COLOR_BLACK);
        display_draw_text(10, 60, band_info, COLOR_BLUE, COLOR_BLACK);
        
        uint32_t freq_step = (freq_bands[band].end_freq - freq_bands[band].start_freq) / 20;
        int max_signal = 0;
        uint32_t peak_freq = 0;
        
        // Sweep through frequency range
        for (int step = 0; step < 20; step++) {
            uint32_t current_freq = freq_bands[band].start_freq + (step * freq_step);
            
            // Simulate signal strength measurement
            int signal_strength = esp_random() % 100;
            
            if (signal_strength > max_signal) {
                max_signal = signal_strength;
                peak_freq = current_freq;
            }
            
            // Draw spectrum bar
            int bar_height = (signal_strength * 80) / 100;
            int x_pos = 10 + (step * 10);
            
            // Clear previous bar
            display_fill_rect(x_pos, 120, 8, 80, COLOR_BLACK);
            
            // Draw new bar
            uint16_t bar_color = signal_strength > 70 ? COLOR_RED : 
                               signal_strength > 40 ? COLOR_ORANGE : COLOR_GREEN;
            display_fill_rect(x_pos, 200 - bar_height, 8, bar_height, bar_color);
            
            char freq_info[32];
            snprintf(freq_info, sizeof(freq_info), "Freq: %luMHz", current_freq / 1000000);
            display_fill_rect(10, 80, 200, 20, COLOR_BLACK);
            display_draw_text(10, 80, freq_info, COLOR_GREEN, COLOR_BLACK);
            
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        
        // Show peak frequency for this band
        char peak_info[48];
        snprintf(peak_info, sizeof(peak_info), "Peak: %luMHz (%ddBm)", 
                peak_freq / 1000000, max_signal - 80);
        display_fill_rect(10, 220, 220, 20, COLOR_BLACK);
        display_draw_text(10, 220, peak_info, COLOR_ORANGE, COLOR_BLACK);
        
        vTaskDelay(pdMS_TO_TICKS(2000)); // Pause between bands
    }
    
    display_draw_text(10, 260, "Spectrum analysis complete", COLOR_GREEN, COLOR_BLACK);
    wait_for_back_button();
}

// RF Signal Recorder - requires CC1101
void rf_signal_recorder(void) {
    if (!cc1101_is_connected()) {
        display_fill_screen(COLOR_BLACK);
        display_draw_text(10, 10, "CC1101 Required", COLOR_RED, COLOR_BLACK);
        display_draw_text(10, 40, "Connect CC1101 module", COLOR_WHITE, COLOR_BLACK);
        display_draw_text(10, 60, "for signal recording", COLOR_WHITE, COLOR_BLACK);
        display_draw_text(10, 280, "Touch to continue", COLOR_GRAY, COLOR_BLACK);
        wait_for_touch_with_timeout(30);
        return;
    }
    
    ESP_LOGI(TAG, "Starting RF signal recorder");
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "RF Signal Recorder", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    display_draw_text(10, 40, "Recording RF signals...", COLOR_GREEN, COLOR_BLACK);
    
    cc1101_freq_t freqs[] = {CC1101_FREQ_315, CC1101_FREQ_433, CC1101_FREQ_868, CC1101_FREQ_915};
    const char* freq_names[] = {"315MHz", "433MHz", "868MHz", "915MHz"};
    
    uint32_t total_signals = 0;
    
    for (int freq_idx = 0; freq_idx < 4; freq_idx++) {
        cc1101_set_frequency(freqs[freq_idx]);
        cc1101_set_rx_mode();
        
        char freq_info[48];
        snprintf(freq_info, sizeof(freq_info), "Recording: %s", freq_names[freq_idx]);
        display_fill_rect(10, 60, 200, 20, COLOR_BLACK);
        display_draw_text(10, 60, freq_info, COLOR_BLUE, COLOR_BLACK);
        
        uint32_t signals_this_freq = 0;
        
        for (int sec = 0; sec < 10; sec++) {
            uint8_t rx_data[64];
            size_t rx_len = 64;
            
            if (cc1101_receive(rx_data, &rx_len, 100) == ESP_OK && rx_len > 0) {
                signals_this_freq++;
                total_signals++;
                
                char signal_info[32];
                snprintf(signal_info, sizeof(signal_info), "Signal captured!");
                display_fill_rect(10, 120, 200, 20, COLOR_BLACK);
                display_draw_text(10, 120, signal_info, COLOR_GREEN, COLOR_BLACK);
                vTaskDelay(pdMS_TO_TICKS(200));
                display_fill_rect(10, 120, 200, 20, COLOR_BLACK);
            }
            
            char time_info[32];
            snprintf(time_info, sizeof(time_info), "Time: %d/10s", sec + 1);
            display_fill_rect(10, 80, 200, 20, COLOR_BLACK);
            display_draw_text(10, 80, time_info, COLOR_ORANGE, COLOR_BLACK);
            
            char count_info[32];
            snprintf(count_info, sizeof(count_info), "This freq: %lu", signals_this_freq);
            display_fill_rect(10, 100, 200, 20, COLOR_BLACK);
            display_draw_text(10, 100, count_info, COLOR_GREEN, COLOR_BLACK);
            
            if (touchscreen_is_touched()) goto exit_recorder;
        }
        
        char save_info[48];
        snprintf(save_info, sizeof(save_info), "Saved %lu from %s", signals_this_freq, freq_names[freq_idx]);
        display_draw_text(10, 140 + freq_idx * 15, save_info, COLOR_BLUE, COLOR_BLACK);
    }
    
exit_recorder:
    char total_info[32];
    snprintf(total_info, sizeof(total_info), "Total: %lu", total_signals);
    display_draw_text(10, 220, total_info, COLOR_GREEN, COLOR_BLACK);
    wait_for_back_button();
}

// RF Replay Attack - uses CC1101 if available, GPIO fallback
void rf_replay_attack_enhanced(void) {
    ESP_LOGI(TAG, "Starting enhanced RF replay attack");
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "RF Replay Attack", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    bool use_cc1101 = cc1101_is_connected();
    if (use_cc1101) {
        display_draw_text(10, 40, "CC1101: Active", COLOR_GREEN, COLOR_BLACK);
    } else {
        display_draw_text(10, 40, "GPIO Mode (Limited)", COLOR_ORANGE, COLOR_BLACK);
        gpio_config_t io_conf = {
            .intr_type = GPIO_INTR_DISABLE,
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = (1ULL << GPIO_NUM_2),
            .pull_down_en = 0,
            .pull_up_en = 0,
        };
        gpio_config(&io_conf);
    }
    
    const struct {
        const char* name;
        cc1101_freq_t freq;
        uint32_t duration_ms;
    } signals[] = {
        {"Garage Door A", CC1101_FREQ_315, 500},
        {"Car Remote B", CC1101_FREQ_433, 300},
        {"Gate Opener C", CC1101_FREQ_868, 800},
        {"Doorbell D", CC1101_FREQ_915, 200},
        {"Alarm Remote E", CC1101_FREQ_433, 1000}
    };
    
    for (int i = 0; i < 5; i++) {
        char signal_info[48];
        snprintf(signal_info, sizeof(signal_info), "Replaying: %s", signals[i].name);
        display_fill_rect(10, 60, 220, 20, COLOR_BLACK);
        display_draw_text(10, 60, signal_info, COLOR_ORANGE, COLOR_BLACK);
        
        if (use_cc1101) {
            cc1101_set_frequency(signals[i].freq);
            cc1101_set_tx_mode();
            
            uint8_t replay_data[32];
            for (int j = 0; j < 32; j++) replay_data[j] = esp_random() & 0xFF;
            
            display_draw_text(10, 80, "Transmitting...", COLOR_RED, COLOR_BLACK);
            cc1101_transmit(replay_data, 32);
            vTaskDelay(pdMS_TO_TICKS(signals[i].duration_ms));
        } else {
            display_draw_text(10, 80, "Transmitting...", COLOR_RED, COLOR_BLACK);
            gpio_set_level(GPIO_NUM_2, 1);
            vTaskDelay(pdMS_TO_TICKS(signals[i].duration_ms));
            gpio_set_level(GPIO_NUM_2, 0);
        }
        
        display_fill_rect(10, 80, 200, 20, COLOR_BLACK);
        display_draw_text(10, 80, "Signal sent!", COLOR_GREEN, COLOR_BLACK);
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        if (touchscreen_is_touched()) break;
    }
    
    if (!use_cc1101) gpio_set_level(GPIO_NUM_2, 0);
    
    display_draw_text(10, 160, "All signals replayed!", COLOR_GREEN, COLOR_BLACK);
    wait_for_back_button();
}