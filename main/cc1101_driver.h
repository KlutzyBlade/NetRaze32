#ifndef CC1101_DRIVER_H
#define CC1101_DRIVER_H

#include "esp_err.h"
#include <stdbool.h>

typedef enum {
    CC1101_FREQ_315 = 0,
    CC1101_FREQ_433,
    CC1101_FREQ_868,
    CC1101_FREQ_915
} cc1101_freq_t;

esp_err_t cc1101_init(void);
bool cc1101_is_connected(void);
esp_err_t cc1101_set_frequency(cc1101_freq_t freq);
esp_err_t cc1101_transmit(const uint8_t *data, size_t len);
esp_err_t cc1101_receive(uint8_t *data, size_t *len, uint32_t timeout_ms);
esp_err_t cc1101_set_rx_mode(void);
esp_err_t cc1101_set_tx_mode(void);
int8_t cc1101_get_rssi(void);

#endif
