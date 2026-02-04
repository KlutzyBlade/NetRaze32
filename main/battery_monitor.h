#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

float battery_get_percentage(void);
int battery_estimate_runtime(const char* attack_type);
void battery_draw_widget(int x, int y);
void battery_show_details(void);

#endif
