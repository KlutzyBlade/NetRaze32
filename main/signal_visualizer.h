#ifndef SIGNAL_VISUALIZER_H
#define SIGNAL_VISUALIZER_H

#include <stdint.h>
#include "esp_wifi_types.h"

// Draw WiFi channel heatmap
void draw_wifi_heatmap(wifi_ap_record_t* ap_records, uint16_t ap_count);

// Draw BLE RSSI bars for devices
void draw_ble_rssi_bars(int8_t* rssi_values, uint16_t device_count, int start_y);

// Draw signal strength meter
void draw_signal_meter(int8_t rssi, int x, int y, int width, int height);

// Convert RSSI to color (red=weak, yellow=medium, green=strong)
uint16_t rssi_to_color(int8_t rssi);

#endif // SIGNAL_VISUALIZER_H
