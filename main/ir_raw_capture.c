#include "ir_raw_capture.h"
#include "display.h"
#include "touchscreen.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "IR_RAW";
static ir_signal_t captured_signals[5];
static int signal_count = 0;

esp_err_t ir_raw_init(void) {
    // Initialize IR pins
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << 16),
        .pull_down_en = 0,
        .pull_up_en = 1,
    };
    gpio_config(&io_conf);
    
    ESP_LOGI(TAG, "IR raw capture initialized");
    return ESP_OK;
}

void ir_raw_capture_signal(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "IR Raw Capture", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    display_draw_text(10, 40, "Point remote at device", COLOR_ORANGE, COLOR_BLACK);
    display_draw_text(10, 60, "Press button to capture", COLOR_BLUE, COLOR_BLACK);
    
    display_draw_text(10, 90, "Waiting for signal...", COLOR_GREEN, COLOR_BLACK);
    
    // Simulate signal detection
    uint16_t sample_timings[] = {9000, 4500, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 560};
    int timing_count = sizeof(sample_timings) / sizeof(sample_timings[0]);
    
    // Wait for signal simulation
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    display_draw_text(10, 110, "Signal detected!", COLOR_GREEN, COLOR_BLACK);
    display_draw_text(10, 130, "Capturing timings...", COLOR_BLUE, COLOR_BLACK);
    
    // Show capture progress
    for (int i = 0; i < timing_count; i++) {
        char timing_info[32];
        snprintf(timing_info, sizeof(timing_info), "T%d: %d us", i, sample_timings[i]);
        
        if (i < 8) {
            display_draw_text(10, 150 + i * 12, timing_info, COLOR_GRAY, COLOR_BLACK);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    // Store signal
    if (signal_count < 5) {
        captured_signals[signal_count].timings = malloc(timing_count * sizeof(uint16_t));
        memcpy(captured_signals[signal_count].timings, sample_timings, timing_count * sizeof(uint16_t));
        captured_signals[signal_count].count = timing_count;
        captured_signals[signal_count].frequency = 38000;
        snprintf(captured_signals[signal_count].name, sizeof(captured_signals[signal_count].name), "Signal_%d", signal_count + 1);
        signal_count++;
    }
    
    display_draw_text(10, 260, "Capture complete!", COLOR_GREEN, COLOR_BLACK);
    char signal_info[32];
    snprintf(signal_info, sizeof(signal_info), "Stored as Signal_%d", signal_count);
    display_draw_text(10, 275, signal_info, COLOR_BLUE, COLOR_BLACK);
    
    display_draw_text(10, 300, "Touch to continue", COLOR_GRAY, COLOR_BLACK);
    
    while (true) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.pressed) break;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void ir_raw_replay_signal(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "IR Signal Replay", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    if (signal_count == 0) {
        display_draw_text(10, 40, "No signals captured", COLOR_RED, COLOR_BLACK);
        display_draw_text(10, 60, "Capture a signal first", COLOR_GRAY, COLOR_BLACK);
        vTaskDelay(pdMS_TO_TICKS(2000));
        return;
    }
    
    display_draw_text(10, 40, "Select signal to replay:", COLOR_BLUE, COLOR_BLACK);
    
    // Show available signals
    for (int i = 0; i < signal_count; i++) {
        display_fill_rect(10, 60 + i * 30, 220, 25, COLOR_DARKBLUE);
        display_draw_rect(10, 60 + i * 30, 220, 25, COLOR_GRAY);
        
        char signal_info[64];
        snprintf(signal_info, sizeof(signal_info), "%s (%d timings)", captured_signals[i].name, captured_signals[i].count);
        display_draw_text(15, 70 + i * 30, signal_info, COLOR_WHITE, COLOR_DARKBLUE);
    }
    
    display_fill_rect(10, 280, 80, 30, COLOR_DARKBLUE);
    display_draw_rect(10, 280, 80, 30, COLOR_WHITE);
    display_draw_text(35, 290, "BACK", COLOR_WHITE, COLOR_DARKBLUE);
    
    while (true) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.pressed) {
                // Check signal selection
                for (int i = 0; i < signal_count; i++) {
                    if (point.y >= 60 + i * 30 && point.y <= 85 + i * 30) {
                        // Replay selected signal
                        display_fill_screen(COLOR_BLACK);
                        display_draw_text(10, 10, "Replaying Signal", COLOR_WHITE, COLOR_BLACK);
                        
                        char replay_info[64];
                        snprintf(replay_info, sizeof(replay_info), "Signal: %s", captured_signals[i].name);
                        display_draw_text(10, 40, replay_info, COLOR_GREEN, COLOR_BLACK);
                        
                        display_draw_text(10, 60, "Transmitting...", COLOR_RED, COLOR_BLACK);
                        
                        // Simulate IR transmission
                        for (int t = 0; t < captured_signals[i].count; t++) {
                            if (t % 2 == 0) {
                                gpio_set_level(4, 1); // IR LED on
                            } else {
                                gpio_set_level(4, 0); // IR LED off
                            }
                            esp_rom_delay_us(captured_signals[i].timings[t]);
                        }
                        gpio_set_level(4, 0); // Ensure off
                        
                        display_draw_text(10, 80, "Transmission complete!", COLOR_GREEN, COLOR_BLACK);
                        vTaskDelay(pdMS_TO_TICKS(2000));
                        return;
                    }
                }
                
                if (point.x >= 10 && point.x <= 90 && point.y >= 280 && point.y <= 310) {
                    break;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void ir_raw_signal_library(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "IR Signal Library", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    display_fill_rect(10, 40, 220, 25, COLOR_DARKBLUE);
    display_draw_rect(10, 40, 220, 25, COLOR_GRAY);
    display_draw_text(15, 50, "TV Power (NEC)", COLOR_GREEN, COLOR_DARKBLUE);
    
    display_fill_rect(10, 70, 220, 25, COLOR_DARKBLUE);
    display_draw_rect(10, 70, 220, 25, COLOR_GRAY);
    display_draw_text(15, 80, "Samsung TV Vol+", COLOR_BLUE, COLOR_DARKBLUE);
    
    display_fill_rect(10, 100, 220, 25, COLOR_DARKBLUE);
    display_draw_rect(10, 100, 220, 25, COLOR_GRAY);
    display_draw_text(15, 110, "Sony TV Ch+", COLOR_ORANGE, COLOR_DARKBLUE);
    
    display_fill_rect(10, 130, 220, 25, COLOR_DARKBLUE);
    display_draw_rect(10, 130, 220, 25, COLOR_GRAY);
    display_draw_text(15, 140, "AC Power Toggle", COLOR_PURPLE, COLOR_DARKBLUE);
    
    display_fill_rect(10, 160, 220, 25, COLOR_DARKBLUE);
    display_draw_rect(10, 160, 220, 25, COLOR_GRAY);
    display_draw_text(15, 170, "Load from SD", COLOR_RED, COLOR_DARKBLUE);
    
    display_fill_rect(10, 280, 80, 30, COLOR_DARKBLUE);
    display_draw_rect(10, 280, 80, 30, COLOR_WHITE);
    display_draw_text(35, 290, "BACK", COLOR_WHITE, COLOR_DARKBLUE);
    
    while (true) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.pressed) {
                if (point.y >= 40 && point.y <= 65) {
                    display_draw_text(10, 200, "Sending TV Power (NEC)", COLOR_GREEN, COLOR_BLACK);
                    // Simulate NEC power command
                    vTaskDelay(pdMS_TO_TICKS(1000));
                } else if (point.y >= 70 && point.y <= 95) {
                    display_draw_text(10, 200, "Sending Samsung Vol+", COLOR_BLUE, COLOR_BLACK);
                    vTaskDelay(pdMS_TO_TICKS(1000));
                } else if (point.x >= 10 && point.x <= 90 && point.y >= 280 && point.y <= 310) {
                    break;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void ir_raw_analyze_signal(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Signal Analysis", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    if (signal_count == 0) {
        display_draw_text(10, 40, "No signals to analyze", COLOR_RED, COLOR_BLACK);
        vTaskDelay(pdMS_TO_TICKS(2000));
        return;
    }
    
    // Analyze first captured signal
    ir_signal_t* signal = &captured_signals[0];
    
    display_draw_text(10, 40, "Analyzing Signal_1:", COLOR_GREEN, COLOR_BLACK);
    
    char info[64];
    snprintf(info, sizeof(info), "Timings: %d", signal->count);
    display_draw_text(10, 60, info, COLOR_BLUE, COLOR_BLACK);
    
    snprintf(info, sizeof(info), "Frequency: %lu Hz", signal->frequency);
    display_draw_text(10, 80, info, COLOR_BLUE, COLOR_BLACK);
    
    // Protocol detection simulation
    display_draw_text(10, 110, "Protocol Detection:", COLOR_ORANGE, COLOR_BLACK);
    
    if (signal->timings[0] > 8000 && signal->timings[1] > 4000) {
        display_draw_text(10, 130, "Detected: NEC Protocol", COLOR_GREEN, COLOR_BLACK);
        display_draw_text(10, 150, "Address: 0x00FF", COLOR_GRAY, COLOR_BLACK);
        display_draw_text(10, 170, "Command: 0x12ED", COLOR_GRAY, COLOR_BLACK);
    } else {
        display_draw_text(10, 130, "Unknown Protocol", COLOR_RED, COLOR_BLACK);
        display_draw_text(10, 150, "Raw timing data", COLOR_GRAY, COLOR_BLACK);
    }
    
    display_draw_text(10, 200, "Bit pattern:", COLOR_BLUE, COLOR_BLACK);
    display_draw_text(10, 220, "10110010 11001101", COLOR_GRAY, COLOR_BLACK);
    
    display_draw_text(10, 280, "Touch to continue", COLOR_GRAY, COLOR_BLACK);
    
    while (true) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.pressed) break;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}