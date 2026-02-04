#include "stats_tracker.h"
#include "display.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static stats_t stats = {0};
static uint32_t boot_time = 0;

void stats_init(void) {
    boot_time = xTaskGetTickCount();
}

void stats_increment_attacks(void) {
    stats.total_attacks++;
}

void stats_increment_captures(void) {
    stats.total_captures++;
}

void stats_increment_scans(void) {
    stats.total_scans++;
}

stats_t stats_get(void) {
    stats.uptime_sec = (xTaskGetTickCount() - boot_time) / 1000;
    return stats;
}

void stats_draw_home_widget(int x, int y) {
    stats_t s = stats_get();
    
    display_draw_text(x, y, "Quick Stats:", COLOR_ORANGE, COLOR_BLACK);
    
    char buf[32];
    snprintf(buf, sizeof(buf), "Attacks: %lu", s.total_attacks);
    display_draw_text(x, y + 15, buf, COLOR_RED, COLOR_BLACK);
    
    snprintf(buf, sizeof(buf), "Captures: %lu", s.total_captures);
    display_draw_text(x, y + 30, buf, COLOR_GREEN, COLOR_BLACK);
    
    snprintf(buf, sizeof(buf), "Scans: %lu", s.total_scans);
    display_draw_text(x, y + 45, buf, COLOR_BLUE, COLOR_BLACK);
    
    uint32_t h = s.uptime_sec / 3600;
    uint32_t m = (s.uptime_sec % 3600) / 60;
    snprintf(buf, sizeof(buf), "Uptime: %luh %lum", h, m);
    display_draw_text(x, y + 60, buf, COLOR_GRAY, COLOR_BLACK);
}
