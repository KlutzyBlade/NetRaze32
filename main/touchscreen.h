#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "board_config.h"

typedef struct {
    int16_t x;
    int16_t y;
    bool pressed;
} touch_point_t;

esp_err_t touchscreen_init(void);
bool touchscreen_is_touched(void);
touch_point_t touchscreen_get_point(void);

#endif // TOUCHSCREEN_H