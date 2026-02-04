#include "rssi_display.h"
#include "display.h"

void rssi_draw_bars(int x, int y, int8_t rssi) {
    int bars = 0;
    if (rssi >= -50) bars = 4;
    else if (rssi >= -60) bars = 3;
    else if (rssi >= -70) bars = 2;
    else if (rssi >= -80) bars = 1;
    
    for (int i = 0; i < 4; i++) {
        int h = 3 + i * 3;
        uint16_t color = (i < bars) ? COLOR_GREEN : COLOR_DARKGRAY;
        display_fill_rect(x + i * 4, y + 12 - h, 3, h, color);
    }
}

void rssi_draw_text(int x, int y, int8_t rssi) {
    char buf[8];
    snprintf(buf, sizeof(buf), "%ddBm", rssi);
    uint16_t color = (rssi >= -60) ? COLOR_GREEN : (rssi >= -75) ? COLOR_ORANGE : COLOR_RED;
    display_draw_text(x, y, buf, color, COLOR_BLACK);
}
