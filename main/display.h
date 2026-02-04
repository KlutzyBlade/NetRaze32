#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/spi_master.h"
#include "board_config.h"

// Color definitions (RGB565)
#define COLOR_BLACK     0x0000
#define COLOR_WHITE     0xFFFF
#define COLOR_RED       0xF800
#define COLOR_GREEN     0x07E0
#define COLOR_BLUE      0x001F
#define COLOR_ORANGE    0xFBE4
#define COLOR_GRAY      0x8410
#define COLOR_DARKGRAY  0x4208
#define COLOR_DARKBLUE  0x3166
#define COLOR_PURPLE    0x780F

// Display functions
esp_err_t display_init(void);
void display_fill_screen(uint16_t color);
void display_draw_pixel(int16_t x, int16_t y, uint16_t color);
void display_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void display_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void display_draw_line(int x0, int y0, int x1, int y1, uint16_t color);
void display_draw_circle(int cx, int cy, int radius, uint16_t color);
void display_draw_text(int16_t x, int16_t y, const char* text, uint16_t color, uint16_t bg_color);
void display_draw_text_2x(int16_t x, int16_t y, const char* text, uint16_t color, uint16_t bg_color);
void display_set_rotation(uint8_t rotation);
void set_addr_window(int16_t x, int16_t y, int16_t w, int16_t h);

// LCD low-level functions
esp_err_t lcd_cmd(uint8_t cmd);
esp_err_t lcd_data(uint8_t data);

extern spi_device_handle_t spi_device;

// ILI9341 Commands
#define ILI9341_MADCTL      0x36

#endif // DISPLAY_H