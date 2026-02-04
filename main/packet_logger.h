#ifndef PACKET_LOGGER_H
#define PACKET_LOGGER_H

#include <stdint.h>
#include <stdbool.h>

bool packet_logger_init(void);
void packet_logger_close(void);

void packet_log_wifi_scan(const char* ssid, int rssi, const char* security);
void packet_log_deauth_attack(const char* target_mac, int count);
void packet_log_beacon_spam(const char* ssid, int count);
void packet_log_ble_scan(const char* name, const char* mac, int rssi);
void packet_log_wps_attempt(const char* ssid, uint32_t pin);
void packet_log_wps_success(const char* ssid, uint32_t pin);
void packet_log_handshake_capture(const char* ssid, const char* client_mac);
void packet_log_custom(const char* type, const char* data);

const char* packet_logger_get_filename(void);

#endif