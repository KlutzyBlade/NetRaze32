#ifndef UI_SIMPLE_H
#define UI_SIMPLE_H

#include <stdint.h>
#include <stdbool.h>

// Stack-safe UI effects using minimal memory
void ui_loading_dots(int x, int y, uint16_t color);
void ui_progress_spinner(int x, int y, uint16_t color);
void ui_blink_text(int x, int y, const char* text, uint16_t color);
void ui_status_indicator(int x, int y, bool active);

#endif // UI_SIMPLE_H