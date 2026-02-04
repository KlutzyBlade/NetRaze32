#include "stats.h"

static uint32_t attacks_count = 0;
static uint32_t scans_count = 0;
static uint32_t captures_count = 0;

void stats_increment_attacks(void) {
    attacks_count++;
}

void stats_increment_scans(void) {
    scans_count++;
}

void stats_increment_captures(void) {
    captures_count++;
}

uint32_t stats_get_attacks(void) {
    return attacks_count;
}

uint32_t stats_get_scans(void) {
    return scans_count;
}

uint32_t stats_get_captures(void) {
    return captures_count;
}
