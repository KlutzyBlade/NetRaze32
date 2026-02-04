#ifndef SUBGHZ_PROTOCOLS_H
#define SUBGHZ_PROTOCOLS_H

#include "esp_err.h"
#include <stdint.h>

typedef struct {
    uint32_t frequency;
    uint32_t data;
    uint8_t bit_count;
    uint16_t te;
    uint8_t repeat;
} subghz_signal_t;

// Protocol functions
esp_err_t subghz_protocols_init(void);
void subghz_send_princeton(uint32_t data, uint8_t bit_count, uint16_t te, uint8_t repeat);
void subghz_send_came(uint32_t data, uint8_t bit_count);
void subghz_send_nice(uint32_t data, uint8_t bit_count);
void subghz_send_linear(uint32_t data, uint8_t bit_count);
void subghz_send_chamberlain(uint32_t data, uint8_t bit_count);
void subghz_send_keeloq(uint64_t data, uint32_t serial, uint16_t counter);
void subghz_capture_raw(uint32_t frequency, uint32_t duration_ms);

#endif // SUBGHZ_PROTOCOLS_H