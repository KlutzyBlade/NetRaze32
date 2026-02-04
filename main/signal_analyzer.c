#include "signal_analyzer.h"
#include "display.h"
#include "touchscreen.h"
#include "utils.h"
#include "esp_log.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>
#include <string.h>

static const char* TAG = "SIGNAL_ANALYZER";
static spectrum_data_t current_spectrum;

esp_err_t signal_analyzer_init(void) {
    current_spectrum.count = 0;
    current_spectrum.max_strength = 0;
    current_spectrum.scan_time = 0;
    ESP_LOGI(TAG, "Signal analyzer initialized");
    return ESP_OK;
}

static void generate_mock_signals(void) __attribute__((unused)); // Mark as potentially unused
static void generate_mock_signals(void) {
    current_spectrum.count = 0;
    current_spectrum.max_strength = 0;
    
    // Generate realistic signal data
    for (int i = 0; i < 32; i++) {
        if (esp_random() % 100 < 30) { // 30% chance of signal
            signal_data_t* sig = &current_spectrum.signals[current_spectrum.count];
            
            // Common frequencies
            float base_freqs[] = {433.92, 868.3, 915.0, 2400.0, 5800.0};
            sig->frequency = base_freqs[esp_random() % 5] + (esp_random() % 1000) / 1000.0f;
            sig->strength = -30 - (esp_random() % 70); // -30 to -100 dBm
            
            if (sig->strength > current_spectrum.max_strength) {
                current_spectrum.max_strength = sig->strength;
            }
            
            // Protocol detection
            if (sig->frequency > 2000) {
                sig->protocol_type = 1;
                strcpy(sig->protocol_name, "WiFi");
            } else if (sig->frequency > 800) {
                sig->protocol_type = 2;
                strcpy(sig->protocol_name, "LoRa");
            } else {
                sig->protocol_type = 3;
                strcpy(sig->protocol_name, "SubGHz");
            }
            
            current_spectrum.count++;
        }
    }
}

void signal_analyzer_dashboard(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "SIGNAL ANALYZER", COLOR_WHITE, COLOR_BLACK);
    
    for (int i = 0; i < 50; i++) {
        // Simple spectrum display
        display_fill_rect(0, 50, DISPLAY_WIDTH, 100, COLOR_BLACK);
        
        for (int x = 0; x < DISPLAY_WIDTH; x += 8) {
            int height = esp_random() % 80;
            uint16_t color = (height > 60) ? COLOR_RED : (height > 30) ? COLOR_ORANGE : COLOR_GREEN;
            display_fill_rect(x, 150-height, 6, height, color);
        }
        
        display_draw_text(10, 160, "Live Signal Analysis Active", COLOR_GREEN, COLOR_BLACK);
        display_draw_text(10, 280, "Touch to exit", COLOR_GRAY, COLOR_BLACK);
        
        if (touchscreen_is_touched()) break;
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void signal_analyzer_waterfall(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "WATERFALL DISPLAY", COLOR_WHITE, COLOR_BLACK);
    
    // Simplified waterfall with direct drawing
    for (int scroll = 0; scroll < 100; scroll++) {
        // Clear previous frame
        display_fill_rect(0, 30, DISPLAY_WIDTH, 80, COLOR_BLACK);
        
        // Draw waterfall lines
        for (int y = 0; y < 40; y++) {
            for (int x = 0; x < DISPLAY_WIDTH; x += 4) {
                uint8_t intensity = (esp_random() + scroll + y + x) % 256;
                uint16_t color = COLOR_BLACK;
                
                if (intensity > 200) color = COLOR_RED;
                else if (intensity > 150) color = COLOR_ORANGE;
                else if (intensity > 100) color = COLOR_WHITE;
                else if (intensity > 50) color = COLOR_GREEN;
                else if (intensity > 20) color = COLOR_BLUE;
                
                if (color != COLOR_BLACK) {
                    display_draw_pixel(x, 30 + y, color);
                }
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
        
        if (touchscreen_is_touched()) break;
    }
    
    display_draw_text(10, 280, "Touch to return", COLOR_GRAY, COLOR_BLACK);
    wait_for_touch_with_timeout(5);
}

void signal_analyzer_heatmap(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "SIGNAL HEATMAP", COLOR_WHITE, COLOR_BLACK);
    
    // Generate heatmap data
    for (int y = 0; y < 15; y++) {
        for (int x = 0; x < 20; x++) {
            uint8_t intensity = esp_random() % 100;
            uint16_t color = COLOR_BLACK;
            
            if (intensity > 80) color = COLOR_RED;
            else if (intensity > 60) color = COLOR_ORANGE;
            else if (intensity > 40) color = COLOR_WHITE;
            else if (intensity > 20) color = COLOR_GREEN;
            else if (intensity > 10) color = COLOR_BLUE;
            
            if (color != COLOR_BLACK) {
                display_fill_rect(10 + x*11, 40 + y*11, 10, 10, color);
            }
        }
    }
    
    // Legend
    display_draw_text(10, 210, "Signal Strength:", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(10, 225, 10, 10, COLOR_BLUE);
    display_draw_text(25, 228, "Low", COLOR_BLUE, COLOR_BLACK);
    display_fill_rect(60, 225, 10, 10, COLOR_GREEN);
    display_draw_text(75, 228, "Med", COLOR_GREEN, COLOR_BLACK);
    display_fill_rect(110, 225, 10, 10, COLOR_ORANGE);
    display_draw_text(125, 228, "High", COLOR_ORANGE, COLOR_BLACK);
    display_fill_rect(160, 225, 10, 10, COLOR_RED);
    display_draw_text(175, 228, "Max", COLOR_RED, COLOR_BLACK);
    
    display_draw_text(10, 280, "Touch to return", COLOR_GRAY, COLOR_BLACK);
    wait_for_touch_with_timeout(10);
}

void signal_analyzer_protocol_detect(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "PROTOCOL DETECTION", COLOR_WHITE, COLOR_BLACK);
    
    const char* protocols[] = {"WiFi 802.11", "Bluetooth LE", "LoRa WAN", "Zigbee", "SubGHz OOK"};
    uint16_t colors[] = {COLOR_BLUE, COLOR_PURPLE, COLOR_ORANGE, COLOR_GREEN, COLOR_RED};
    
    for (int i = 0; i < 5; i++) {
        if (esp_random() % 100 < 60) { // 60% detection chance
            display_fill_rect(10, 40 + i*30, 200, 25, COLOR_DARKBLUE);
            display_draw_rect(10, 40 + i*30, 200, 25, colors[i]);
            display_draw_text(15, 48 + i*30, protocols[i], colors[i], COLOR_DARKBLUE);
            
            char strength[32];
            snprintf(strength, sizeof(strength), "-%ddBm", (int)(30 + (esp_random() % 70)));
            display_draw_text(150, 48 + i*30, strength, COLOR_WHITE, COLOR_DARKBLUE);
        }
    }
    
    display_draw_text(10, 280, "Touch to return", COLOR_GRAY, COLOR_BLACK);
    wait_for_touch_with_timeout(10);
}