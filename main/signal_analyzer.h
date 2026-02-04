#ifndef SIGNAL_ANALYZER_H
#define SIGNAL_ANALYZER_H

#include "esp_err.h"
#include <stdint.h>

typedef struct {
    float frequency;
    float strength;
    uint8_t protocol_type;
    char protocol_name[16];
} signal_data_t;

typedef struct {
    signal_data_t signals[64];
    uint8_t count;
    float max_strength;
    uint32_t scan_time;
} spectrum_data_t;

esp_err_t signal_analyzer_init(void);
void signal_analyzer_dashboard(void);
void signal_analyzer_waterfall(void);
void signal_analyzer_heatmap(void);
void signal_analyzer_protocol_detect(void);

#endif // SIGNAL_ANALYZER_H