#ifndef TARGET_MANAGER_H
#define TARGET_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_wifi_types.h"

#define MAX_SAVED_TARGETS 20

typedef enum {
    TARGET_WIFI,
    TARGET_BLE
} target_type_t;

typedef struct {
    target_type_t type;
    char name[32];
    uint8_t mac[6];
    int8_t rssi;
    uint8_t channel;
    bool favorite;
    uint32_t last_seen;
} saved_target_t;

// Initialize target manager
void target_manager_init(void);

// Save target
bool target_manager_save(target_type_t type, const char* name, const uint8_t* mac, 
                         int8_t rssi, uint8_t channel);

// Delete target
bool target_manager_delete(uint8_t index);

// Get target by index
saved_target_t* target_manager_get(uint8_t index);

// Get target count
uint8_t target_manager_count(void);

// Toggle favorite
void target_manager_toggle_favorite(uint8_t index);

// Load from SD card
bool target_manager_load(void);

// Save to SD card
bool target_manager_save_to_file(void);

// UI for managing targets
void target_manager_ui(void);

// Quick save from scan results
void target_manager_quick_save_wifi(wifi_ap_record_t* ap);
void target_manager_quick_save_ble(const uint8_t* mac, const char* name, int8_t rssi);

#endif // TARGET_MANAGER_H
