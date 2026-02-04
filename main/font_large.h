#ifndef FONT_LARGE_H
#define FONT_LARGE_H

#include <stdint.h>

// Large 8x16 font for better readability
extern const uint8_t font_large_8x16[256][16];

void display_draw_text_large(int x, int y, const char* text, uint16_t color, uint16_t bg_color);
void display_draw_char_large(int x, int y, char c, uint16_t color, uint16_t bg_color);

#endif // FONT_LARGE_H