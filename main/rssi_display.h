#ifndef RSSI_DISPLAY_H
#define RSSI_DISPLAY_H

#include <stdint.h>

void rssi_draw_bars(int x, int y, int8_t rssi);
void rssi_draw_text(int x, int y, int8_t rssi);

#endif
