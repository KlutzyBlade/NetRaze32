#ifndef BLUETOOTH_FUNCTIONS_H
#define BLUETOOTH_FUNCTIONS_H

#include "esp_bt.h"

// Enhanced Bluetooth function prototypes
esp_err_t bluetooth_init(void);
void ble_scan_start(void);
void ble_jammer_start(void);
void bluetooth_targeted_attack_enhanced(void);
void ble_spoofer_start(void);
void ble_sour_apple(void);
void ble_sniffer_start(void);

// New enhanced functions from ESP32Marauder
void ble_apple_spam(void);
void ble_samsung_spam(void);
void ble_microsoft_spam(void);
void ble_airtag_scanner(void);
void ble_flipper_detector(void);
void ble_wardriving_scan(void);
void ble_swiftpair_spam(void);
void ble_fastpair_spam(void);
void ble_beacon_flood(void);
void ble_targeted_attack(void);

// Enhanced attacks imported from Bruce
void ble_apple_juice_attack(void);
void ble_sour_apple_attack(void);
void ble_samsung_watch_spam_enhanced(void);
void ble_google_fastpair_spam(void);
void ble_beacon_flood_attack(void);

#endif // BLUETOOTH_FUNCTIONS_H