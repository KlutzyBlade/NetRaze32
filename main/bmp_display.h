#ifndef BMP_DISPLAY_H
#define BMP_DISPLAY_H

#include "esp_err.h"
#include <stdint.h>
#include <stddef.h>

esp_err_t bmp_display_init(void);
void display_bmp_from_buffer(const uint8_t* bmp_data, size_t data_size, int x, int y);
void display_bmp_from_file(const char* filename, int x, int y);

#endif