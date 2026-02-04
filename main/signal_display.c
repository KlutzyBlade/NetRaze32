#include "signal_display.h"
#include "display.h"
#include <stdio.h>
#include <math.h>

void draw_signal_bars(int x, int y, int rssi) {
    // Convert RSSI to signal strength (0-4 bars)
    int bars = 0;
    if (rssi >= -50) bars = 4;      // Excellent
    else if (rssi >= -60) bars = 3; // Good  
    else if (rssi >= -70) bars = 2; // Fair
    else if (rssi >= -80) bars = 1; // Poor
    else bars = 0;                  // No signal
    
    // Draw 4 bars with different heights
    for (int i = 0; i < 4; i++) {
        int bar_height = 3 + (i * 2); // Heights: 3, 5, 7, 9
        int bar_x = x + (i * 3);
        int bar_y = y + (9 - bar_height);
        
        uint16_t color = (i < bars) ? COLOR_GREEN : COLOR_DARKGRAY;
        if (bars == 1 && i == 0) color = COLOR_RED;
        else if (bars == 2 && i < 2) color = COLOR_ORANGE;
        
        display_fill_rect(bar_x, bar_y, 2, bar_height, color);
    }
}

void draw_rssi_meter(int x, int y, int rssi) {
    // Draw background
    display_draw_rect(x, y, 60, 12, COLOR_WHITE);
    display_fill_rect(x + 1, y + 1, 58, 10, COLOR_BLACK);
    
    // Calculate fill width based on RSSI (-100 to -30 dBm range)
    int fill_width = 0;
    if (rssi > -30) fill_width = 58;
    else if (rssi < -100) fill_width = 0;
    else fill_width = (rssi + 100) * 58 / 70;
    
    // Choose color based on signal strength
    uint16_t color = COLOR_RED;
    if (rssi >= -50) color = COLOR_GREEN;
    else if (rssi >= -70) color = COLOR_ORANGE;
    
    if (fill_width > 0) {
        display_fill_rect(x + 1, y + 1, fill_width, 10, color);
    }
    
    // Draw RSSI value
    char rssi_text[8];
    snprintf(rssi_text, sizeof(rssi_text), "%ddBm", rssi);
    display_draw_text(x + 65, y + 2, rssi_text, COLOR_WHITE, COLOR_BLACK);
}

void draw_signal_quality_indicator(int x, int y, int rssi) {
    const char* quality;
    uint16_t color;
    
    if (rssi >= -50) {
        quality = "EXCELLENT";
        color = COLOR_GREEN;
    } else if (rssi >= -60) {
        quality = "GOOD";
        color = COLOR_GREEN;
    } else if (rssi >= -70) {
        quality = "FAIR";
        color = COLOR_ORANGE;
    } else if (rssi >= -80) {
        quality = "POOR";
        color = COLOR_RED;
    } else {
        quality = "NO SIGNAL";
        color = COLOR_RED;
    }
    
    display_draw_text(x, y, quality, color, COLOR_BLACK);
}

void draw_channel_graph(int x, int y, int width, int height, int* channel_data, int max_channels) {
    // Draw axes using proper line function
    display_draw_line(x, y + height, x + width, y + height, COLOR_WHITE); // X-axis
    display_draw_line(x, y, x, y + height, COLOR_WHITE); // Y-axis
    
    // Find max value for scaling
    int max_value = 1;
    for (int i = 0; i < max_channels; i++) {
        if (channel_data[i] > max_value) {
            max_value = channel_data[i];
        }
    }
    
    // Draw bars for each channel
    int bar_width = width / max_channels;
    for (int i = 0; i < max_channels; i++) {
        int bar_height = (channel_data[i] * height) / max_value;
        int bar_x = x + (i * bar_width);
        int bar_y = y + height - bar_height;
        
        uint16_t color = COLOR_BLUE;
        if (channel_data[i] > max_value * 0.7) color = COLOR_RED;
        else if (channel_data[i] > max_value * 0.4) color = COLOR_ORANGE;
        
        display_fill_rect(bar_x, bar_y, bar_width - 1, bar_height, color);
        
        // Draw channel number
        if (bar_width >= 8 && i < 99) {
            char ch_text[8];
            snprintf(ch_text, sizeof(ch_text), "%d", i + 1);
            display_draw_text(bar_x + 1, y + height + 2, ch_text, COLOR_WHITE, COLOR_BLACK);
        }
    }
}

void draw_signal_compass(int x, int y, int radius, int direction, int strength) {
    // Draw compass circle using proper circle function
    display_draw_circle(x, y, radius, COLOR_WHITE);
    
    // Draw cardinal directions
    display_draw_text(x - 2, y - radius - 8, "N", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(x + radius + 2, y - 2, "E", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(x - 2, y + radius + 2, "S", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(x - radius - 8, y - 2, "W", COLOR_WHITE, COLOR_BLACK);
    
    // Draw signal direction arrow
    int arrow_length = (radius * strength) / 100;
    if (arrow_length < 5) arrow_length = 5;
    
    // Convert direction to radians
    float angle = (direction * 3.14159f) / 180.0f;
    int end_x = x + (int)(arrow_length * sinf(angle));
    int end_y = y - (int)(arrow_length * cosf(angle));
    
    uint16_t color = COLOR_GREEN;
    if (strength < 30) color = COLOR_RED;
    else if (strength < 60) color = COLOR_ORANGE;
    
    // Draw line using proper line function
    display_draw_line(x, y, end_x, end_y, color);
    
    // Draw arrowhead
    display_fill_rect(end_x - 1, end_y - 1, 3, 3, color);
}