#include "handshake_capture.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "display.h"
#include "touchscreen.h"
#include "packet_logger.h"
#include <string.h>

static const char* TAG = "HANDSHAKE";

void handshake_log_info(const char* msg) {
    ESP_LOGI(TAG, "%s", msg);
}

typedef struct {
    uint8_t bssid[6];
    uint8_t client[6];
    char ssid[32];
    bool has_beacon;
    bool has_auth;
    bool has_assoc_req;
    bool has_assoc_resp;
    int eapol_count;
} handshake_info_t;

static handshake_info_t handshakes[5];
static int handshake_count = 0;

static void handshake_promiscuous_cb(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_MGMT) return;
    
    wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t*)buf;
    uint8_t *frame = pkt->payload;
    
    // Check frame type
    uint8_t frame_type = frame[0] & 0xFC;
    
    if (frame_type == 0x80) { // Beacon
        if (handshake_count < 5) {
            memcpy(handshakes[handshake_count].bssid, &frame[10], 6);
            
            // Extract SSID from beacon
            uint8_t *ssid_ptr = &frame[36]; // Skip fixed parameters
            if (ssid_ptr[0] == 0x00 && ssid_ptr[1] <= 32) { // SSID element
                memcpy(handshakes[handshake_count].ssid, &ssid_ptr[2], ssid_ptr[1]);
                handshakes[handshake_count].ssid[ssid_ptr[1]] = '\0';
                handshakes[handshake_count].has_beacon = true;
                handshake_count++;
            }
        }
    }
    else if (frame_type == 0xB0) { // Authentication
        for (int i = 0; i < handshake_count; i++) {
            if (memcmp(handshakes[i].bssid, &frame[4], 6) == 0) {
                memcpy(handshakes[i].client, &frame[10], 6);
                handshakes[i].has_auth = true;
                break;
            }
        }
    }
    else if (frame_type == 0x00) { // Association Request
        for (int i = 0; i < handshake_count; i++) {
            if (memcmp(handshakes[i].bssid, &frame[4], 6) == 0) {
                handshakes[i].has_assoc_req = true;
                break;
            }
        }
    }
}

void handshake_capture_start(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Handshake Capture", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    display_draw_text(10, 40, "Monitoring for handshakes...", COLOR_BLUE, COLOR_BLACK);
    
    // Reset data
    memset(handshakes, 0, sizeof(handshakes));
    handshake_count = 0;
    
    // Set promiscuous mode
    esp_wifi_set_mode(WIFI_MODE_NULL);
    esp_wifi_set_promiscuous_rx_cb(handshake_promiscuous_cb);
    esp_wifi_set_promiscuous(true);
    
    // Cancel button
    display_fill_rect(160, 280, 70, 30, COLOR_RED);
    display_draw_rect(160, 280, 70, 30, COLOR_WHITE);
    display_draw_text(180, 290, "CANCEL", COLOR_WHITE, COLOR_RED);
    
    bool cancelled = false;
    int captured = 0;
    
    // Monitor for 60 seconds
    for (int sec = 0; sec < 60 && !cancelled; sec++) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.x >= 160 && point.x <= 230 && point.y >= 280 && point.y <= 310) {
                cancelled = true;
                break;
            }
        }
        
        char time_info[32];
        snprintf(time_info, sizeof(time_info), "Time: %d/60s", sec + 1);
        display_fill_rect(10, 60, 200, 15, COLOR_BLACK);
        display_draw_text(10, 60, time_info, COLOR_BLUE, COLOR_BLACK);
        
        // Count complete handshakes
        captured = 0;
        for (int i = 0; i < handshake_count; i++) {
            if (handshakes[i].has_beacon && handshakes[i].has_auth && 
                handshakes[i].has_assoc_req && handshakes[i].eapol_count >= 2) {
                captured++;
            }
        }
        
        char capture_info[64];
        snprintf(capture_info, sizeof(capture_info), "Net: %d Cap: %d", handshake_count, captured);
        display_fill_rect(10, 80, 220, 15, COLOR_BLACK);
        display_draw_text(10, 80, capture_info, COLOR_GREEN, COLOR_BLACK);
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    esp_wifi_set_promiscuous(false);
    esp_wifi_set_mode(WIFI_MODE_STA);
    
    // Save captured handshakes
    for (int i = 0; i < captured; i++) {
        packet_log_handshake_capture(handshakes[i].ssid, 
                                    "XX:XX:XX:XX:XX:XX"); // Simplified MAC
    }
    
    // Results screen
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Handshake Results", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    if (cancelled) {
        display_draw_text(10, 40, "Capture cancelled", COLOR_RED, COLOR_BLACK);
    } else {
        display_draw_text(10, 40, "Capture completed", COLOR_GREEN, COLOR_BLACK);
    }
    
    char final_stats[32];
    snprintf(final_stats, sizeof(final_stats), "Handshakes: %d", captured);
    display_draw_text(10, 60, final_stats, COLOR_BLUE, COLOR_BLACK);
    
    display_fill_rect(10, 280, 80, 30, COLOR_DARKBLUE);
    display_draw_rect(10, 280, 80, 30, COLOR_WHITE);
    display_draw_text(35, 290, "BACK", COLOR_WHITE, COLOR_DARKBLUE);
    
    while (true) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.x >= 10 && point.x <= 90 && point.y >= 280 && point.y <= 310) {
                return;
            }
            vTaskDelay(pdMS_TO_TICKS(300));
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}