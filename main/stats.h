#ifndef STATS_H
#define STATS_H

#include <stdint.h>

void stats_increment_attacks(void);
void stats_increment_scans(void);
void stats_increment_captures(void);
uint32_t stats_get_attacks(void);
uint32_t stats_get_scans(void);
uint32_t stats_get_captures(void);

#endif
