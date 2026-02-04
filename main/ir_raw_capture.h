#ifndef IR_RAW_CAPTURE_H
#define IR_RAW_CAPTURE_H

#include "esp_err.h"
#include <stdint.h>

typedef struct {
    uint16_t* timings;
    uint16_t count;
    uint32_t frequency;
    char name[32];
} ir_signal_t;

esp_err_t ir_raw_init(void);
void ir_raw_capture_signal(void);
void ir_raw_replay_signal(void);
void ir_raw_signal_library(void);
void ir_raw_analyze_signal(void);

#endif // IR_RAW_CAPTURE_H