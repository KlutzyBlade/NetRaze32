#ifndef WIFI_FUNCTIONS_H
#define WIFI_FUNCTIONS_H

#include "esp_wifi.h"

// Enhanced WiFi function prototypes
esp_err_t wifi_init(void);
void wifi_scan_start(void);
void wifi_deauth_attack(void);
void wifi_beacon_spam(void);
void wifi_packet_monitor(void);
void wifi_deauth_detector(void);
void wifi_captive_portal(void);

// New enhanced functions from ESP32Marauder
void wifi_channel_analyzer(void);
void wifi_pineapple_detector(void);
void wifi_probe_sniffer(void);
void wifi_eapol_sniffer(void);
void wifi_wardriving_mode(void);
void wifi_evil_twin_attack(void);
void wifi_karma_attack(void);
void wifi_pmkid_capture(void);
void wifi_rickroll_attack(void);

// Enhanced attacks imported from Bruce
void wifi_evil_portal_attack(void);
void wifi_karma_attack(void);
void wifi_beacon_spam_enhanced(void);
void wifi_pineapple_detector(void);
void wifi_rickroll_attack(void);

#endif // WIFI_FUNCTIONS_H