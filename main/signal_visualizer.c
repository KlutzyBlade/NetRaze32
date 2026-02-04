#include "signal_visualizer.h"
#include "display.h"
#include "board_config.h"
#include <string.h>

uint16_t rssi_to_color(int8_t rssi) {
    if (rssi >= -50) return COLOR_GREEN;      // Strong
    if (rssi >= -70) return COLOR_ORANGE;     // Medium
    return COLOR_RED;                         // Weak
}

void draw_signal_meter(int8_t rssi, int x, int y, int width, int height) {
    // Draw border
    display_draw_rect(x, y, width, height, COLOR_GRAY);
    
    // Calculate fill percentage (RSSI -100 to -30)
    int percentage = ((rssi + 100) * 100) / 70;
    if (percentage < 0) percentage = 0;
    if (percentage > 100) percentage = 100;
    
    int fill_width = (width - 4) * percentage / 100;
    uint16_t color = rssi_to_color(rssi);
    
    // Draw filled bar
    display_fill_rect(x + 2, y + 2, fill_width, height - 4, color);
}

void draw_wifi_heatmap(wifi_ap_record_t* ap_records, uint16_t ap_count) {
    // Channel distribution (1-14)
    int channel_count[14] = {0};
    int channel_rssi[14] = {0};
    
    // Aggregate data by channel
    for (int i = 0; i < ap_count; i++) {
        int ch = ap_records[i].primary;
        if (ch >= 1 && ch <= 14) {
            channel_count[ch - 1]++;
            if (ap_records[i].rssi > channel_rssi[ch - 1]) {
                channel_rssi[ch - 1] = ap_records[i].rssi;
            }
        }
    }
    
    // Draw heatmap
    int bar_width = 220 / 14;
    int max_height = 150;
    
    for (int i = 0; i < 14; i++) {
        if (channel_count[i] > 0) {
            // Height based on AP count
            int height = (channel_count[i] * max_height) / 10;
            if (height > max_height) height = max_height;
            
            // Color based on RSSI
            uint16_t color = rssi_to_color(channel_rssi[i]);
            
            int x = 10 + i * bar_width;
            int y = 200 - height;
            
            display_fill_rect(x, y, bar_width - 2, height, color);
            
            // Draw channel number
            if (i % 2 == 0) {
                char ch_str[4];
                snprintf(ch_str, sizeof(ch_str), "%d", i + 1);
                display_draw_text(x, 205, ch_str, COLOR_WHITE, COLOR_BLACK);
            }
        }
    }
    
    // Draw axis
    display_draw_rect(10, 50, 220, 150, COLOR_WHITE);
    display_draw_text(10, 35, "WiFi Channel Heatmap", COLOR_WHITE, COLOR_BLACK);
}

void draw_ble_rssi_bars(int8_t* rssi_values, uint16_t device_count, int start_y) {
    for (int i = 0; i < device_count && i < 8; i++) {
        int y = start_y + i * 20;
        
        // Draw device number
        char dev_num[8];
        snprintf(dev_num, sizeof(dev_num), "%d:", i + 1);
        display_draw_text(10, y, dev_num, COLOR_WHITE, COLOR_BLACK);
        
        // Draw RSSI bar
        draw_signal_meter(rssi_values[i], 30, y, 180, 12);
        
        // Draw RSSI value
        char rssi_str[8];
        snprintf(rssi_str, sizeof(rssi_str), "%d", rssi_values[i]);
        display_draw_text(215, y, rssi_str, COLOR_GRAY, COLOR_BLACK);
    }
}
