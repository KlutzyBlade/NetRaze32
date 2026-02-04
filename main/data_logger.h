#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include "esp_err.h"
#include <stdint.h>

typedef struct {
    char ssid[33];
    uint8_t bssid[6];
    int8_t rssi;
    uint8_t channel;
    uint8_t authmode;
    uint32_t timestamp;
} wifi_log_entry_t;

typedef struct {
    uint8_t addr[6];
    int8_t rssi;
    uint8_t adv_type;
    uint32_t timestamp;
} ble_log_entry_t;

// Data logging functions
void data_logger_init(void);
void log_scan_results(void);
esp_err_t log_wifi_scan_result(const wifi_log_entry_t* entry);
esp_err_t log_ble_scan_result(const ble_log_entry_t* entry);
esp_err_t export_logs_to_file(const char* filename);
void clear_all_logs(void);
uint32_t get_wifi_log_count(void);
uint32_t get_ble_log_count(void);

#endif // DATA_LOGGER_H