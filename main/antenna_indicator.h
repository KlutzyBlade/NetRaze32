#ifndef ANTENNA_INDICATOR_H
#define ANTENNA_INDICATOR_H

#include "esp_err.h"

typedef enum {
    ANTENNA_INTERNAL,
    ANTENNA_EXTERNAL
} antenna_mode_t;

void antenna_set_mode(antenna_mode_t mode);
antenna_mode_t antenna_get_mode(void);
void antenna_draw_indicator(int x, int y);
esp_err_t antenna_toggle_ui(void);

#endif
