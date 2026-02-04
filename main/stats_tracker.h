#ifndef STATS_TRACKER_H
#define STATS_TRACKER_H

#include <stdint.h>

typedef struct {
    uint32_t total_attacks;
    uint32_t total_captures;
    uint32_t total_scans;
    uint32_t uptime_sec;
} stats_t;

void stats_init(void);
void stats_increment_attacks(void);
void stats_increment_captures(void);
void stats_increment_scans(void);
stats_t stats_get(void);
void stats_draw_home_widget(int x, int y);

#endif
