#ifndef UTILS_H
#define UTILS_H

#include "touchscreen.h"

void utils_init(void);
void draw_status_bar(float battery_voltage);
float read_battery_voltage(void);
void log_system_stats(void);
void enter_safe_mode(void);
void wait_for_touch_with_timeout(int timeout_seconds);
void wait_for_back_button(void);

#endif