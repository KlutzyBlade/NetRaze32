#ifndef SETTINGS_MENU_H
#define SETTINGS_MENU_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t brightness;
    uint8_t attack_power;
    bool auto_save;
    bool stealth_mode;
    uint8_t default_channel;
    char device_name[32];
    bool show_battery;
    bool sound_enabled;
} app_settings_t;

// Initialize settings menu
void settings_menu_init(void);

// Show settings UI
void settings_menu_ui(void);

// Get current settings
app_settings_t* settings_menu_get(void);

// Save settings to SD card
bool settings_menu_save(void);

// Load settings from SD card
bool settings_menu_load(void);

#endif // SETTINGS_MENU_H
