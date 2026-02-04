#ifndef SIGNAL_DISPLAY_H
#define SIGNAL_DISPLAY_H

#include <stdint.h>

// Draw signal strength as bars (like cell phone)
void draw_signal_bars(int x, int y, int rssi);

// Draw RSSI as horizontal meter with dBm value
void draw_rssi_meter(int x, int y, int rssi);

// Draw text quality indicator (EXCELLENT/GOOD/FAIR/POOR)
void draw_signal_quality_indicator(int x, int y, int rssi);

// Draw channel activity graph
void draw_channel_graph(int x, int y, int width, int height, int* channel_data, int max_channels);

// Draw signal direction compass
void draw_signal_compass(int x, int y, int radius, int direction, int strength);

#endif