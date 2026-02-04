#ifndef BRIGHTNESS_CONTROL_H
#define BRIGHTNESS_CONTROL_H

#include <stdint.h>
#include "esp_err.h"

void brightness_init(void);
void brightness_set(uint8_t level);
uint8_t brightness_get(void);
esp_err_t brightness_control_ui(void);

#endif
