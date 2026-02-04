#ifndef UI_EFFECTS_H
#define UI_EFFECTS_H

#include <stdint.h>
#include <stdbool.h>

// Matrix rain effect
void ui_matrix_rain(uint16_t duration_ms);

// Glitch effect for transitions
void ui_glitch_transition(void);

// Animated progress bars
void ui_animated_progress(int x, int y, int width, int progress, uint16_t color);

// Pulsing elements
void ui_pulse_element(int x, int y, int width, int height, uint16_t color);

// Scan line effects
void ui_scan_lines(void);

// Terminal-style text with typing effect
void ui_terminal_text(int x, int y, const char* text, uint16_t color, uint16_t delay_ms);

// Hexadecimal data stream
void ui_hex_stream(int x, int y, int width, int height);

#endif // UI_EFFECTS_H