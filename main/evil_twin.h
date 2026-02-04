#ifndef EVIL_TWIN_H
#define EVIL_TWIN_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char ssid[33];
    uint8_t bssid[6];
    int8_t rssi;
    uint8_t channel;
    bool is_open;
} target_ap_t;

void evil_twin_init(void);
void evil_twin_start_attack(const char* target_ssid, uint8_t channel);
void evil_twin_stop_attack(void);
bool evil_twin_is_running(void);
void evil_twin_captive_portal(void);
int evil_twin_get_client_count(void);

#endif