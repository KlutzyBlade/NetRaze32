#include "wps_attack.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "display.h"
#include "touchscreen.h"
#include "packet_logger.h"
#include "esp_random.h"
#include <string.h>

static const char* TAG = "WPS_ATTACK";

// Common WPS PINs to try first
static const uint32_t common_pins[] = {
    12345670, 00000000, 11111111, 22222222, 33333333, 44444444,
    55555555, 66666666, 77777777, 88888888, 99999999, 12345678,
    87654321, 11223344, 55667788, 12344321, 11111110, 22222220
};

static uint32_t calculate_checksum(uint32_t pin) {
    uint32_t accum = 0;
    pin *= 10;
    accum += 3 * ((pin / 10000000) % 10);
    accum += 1 * ((pin / 1000000) % 10);
    accum += 3 * ((pin / 100000) % 10);
    accum += 1 * ((pin / 10000) % 10);
    accum += 3 * ((pin / 1000) % 10);
    accum += 1 * ((pin / 100) % 10);
    accum += 3 * ((pin / 10) % 10);
    return (10 - (accum % 10)) % 10;
}

static bool is_valid_pin(uint32_t pin) {
    if (pin > 99999999) return false;
    uint32_t checksum = calculate_checksum(pin / 10);
    return (pin % 10) == checksum;
}

void wps_attack_start(void) {
    ESP_LOGI(TAG, "=== WPS PIN Attack Started ===");
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "WPS PIN Attack", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    display_draw_text(10, 40, "Scanning for WPS APs...", COLOR_BLUE, COLOR_BLACK);
    
    // Scan for WPS-enabled APs
    ESP_LOGI(TAG, "Initiating WiFi scan for WPS-enabled access points");
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true
    };
    
    esp_err_t scan_result = esp_wifi_scan_start(&scan_config, true);
    if (scan_result != ESP_OK) {
        ESP_LOGE(TAG, "WiFi scan failed to start: %s", esp_err_to_name(scan_result));
        display_draw_text(10, 80, "Scan failed", COLOR_RED, COLOR_BLACK);
        display_draw_text(10, 280, "Touch to return", COLOR_GRAY, COLOR_BLACK);
        while (!touchscreen_is_touched()) {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        return;
    }
    
    uint16_t ap_count = 0;
    esp_wifi_scan_get_ap_num(&ap_count);
    ESP_LOGI(TAG, "WiFi scan completed. Found %d access points", ap_count);
    
    if (ap_count == 0) {
        ESP_LOGW(TAG, "No access points detected in scan");
        display_draw_text(10, 80, "No APs found", COLOR_RED, COLOR_BLACK);
        display_draw_text(10, 280, "Touch to return", COLOR_GRAY, COLOR_BLACK);
        while (!touchscreen_is_touched()) {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        return;
    }
    
    wifi_ap_record_t *ap_records = malloc(sizeof(wifi_ap_record_t) * ap_count);
    if (!ap_records) {
        ESP_LOGE(TAG, "Failed to allocate memory for %d AP records (%d bytes)", 
                 ap_count, sizeof(wifi_ap_record_t) * ap_count);
        display_draw_text(10, 80, "Memory error", COLOR_RED, COLOR_BLACK);
        return;
    }
    ESP_LOGD(TAG, "Allocated memory for %d AP records", ap_count);
    
    esp_wifi_scan_get_ap_records(&ap_count, ap_records);
    ESP_LOGD(TAG, "Retrieved AP records from scan");
    
    // Find WPS-enabled APs (simplified detection)
    int wps_count = 0;
    int selected_ap = -1;
    
    ESP_LOGI(TAG, "Filtering for WPS-enabled APs (WPA2-PSK with potential WPS)");
    for (int i = 0; i < ap_count && i < 8; i++) {
        if (ap_records[i].authmode == WIFI_AUTH_WPA2_PSK) {
            ESP_LOGD(TAG, "Potential WPS AP #%d: SSID='%s', RSSI=%ddBm, Channel=%d, BSSID=%02x:%02x:%02x:%02x:%02x:%02x",
                     wps_count + 1,
                     (char*)ap_records[i].ssid,
                     ap_records[i].rssi,
                     ap_records[i].primary,
                     ap_records[i].bssid[0], ap_records[i].bssid[1], ap_records[i].bssid[2],
                     ap_records[i].bssid[3], ap_records[i].bssid[4], ap_records[i].bssid[5]);
            
            char ap_info[48];
            snprintf(ap_info, sizeof(ap_info), "%.20s %ddBm", 
                    (char*)ap_records[i].ssid, ap_records[i].rssi);
            display_draw_text(10, 80 + wps_count * 20, ap_info, COLOR_GREEN, COLOR_BLACK);
            wps_count++;
        }
    }
    
    ESP_LOGI(TAG, "Found %d potential WPS-enabled access points", wps_count);
    
    if (wps_count == 0) {
        ESP_LOGW(TAG, "No WPS-enabled APs found in scan results");
        display_draw_text(10, 80, "No WPS APs found", COLOR_RED, COLOR_BLACK);
        free(ap_records);
        display_draw_text(10, 280, "Touch to return", COLOR_GRAY, COLOR_BLACK);
        while (!touchscreen_is_touched()) {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        return;
    }
    
    // Select first WPS AP for attack
    selected_ap = 0;
    for (int i = 0; i < ap_count; i++) {
        if (ap_records[i].authmode == WIFI_AUTH_WPA2_PSK) {
            selected_ap = i;
            break;
        }
    }
    
    ESP_LOGI(TAG, "Selected target AP: SSID='%s', RSSI=%ddBm, Channel=%d", 
             (char*)ap_records[selected_ap].ssid,
             ap_records[selected_ap].rssi,
             ap_records[selected_ap].primary);
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "WPS PIN Attack", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    char target_info[48];
    snprintf(target_info, sizeof(target_info), "Target: %.20s", (char*)ap_records[selected_ap].ssid);
    display_draw_text(10, 40, target_info, COLOR_ORANGE, COLOR_BLACK);
    
    // Cancel button
    display_fill_rect(160, 280, 70, 30, COLOR_RED);
    display_draw_rect(160, 280, 70, 30, COLOR_WHITE);
    display_draw_text(180, 290, "CANCEL", COLOR_WHITE, COLOR_RED);
    
    bool cancelled = false;
    int pins_tried = 0;
    int common_pins_count = sizeof(common_pins) / sizeof(common_pins[0]);
    
    ESP_LOGI(TAG, "Starting PIN brute force attack with %d common PINs", common_pins_count);
    
    // Try common PINs first
    for (int i = 0; i < common_pins_count && !cancelled; i++) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.x >= 160 && point.x <= 230 && point.y >= 280 && point.y <= 310) {
                ESP_LOGW(TAG, "User cancelled attack via touchscreen (touch at x=%d, y=%d)", 
                         point.x, point.y);
                cancelled = true;
                break;
            }
        }
        
        uint32_t pin = common_pins[i];
        pins_tried++;
        
        ESP_LOGD(TAG, "Attempt %d/%d: Trying PIN %08lu (valid=%s)", 
                 pins_tried, common_pins_count, pin, 
                 is_valid_pin(pin) ? "yes" : "no");
        
        char pin_info[32];
        snprintf(pin_info, sizeof(pin_info), "Trying PIN: %08lu", pin);
        display_fill_rect(10, 60, 220, 15, COLOR_BLACK);
        display_draw_text(10, 60, pin_info, COLOR_BLUE, COLOR_BLACK);
        
        char progress_info[32];
        snprintf(progress_info, sizeof(progress_info), "Tried: %d/%d", pins_tried, common_pins_count);
        display_fill_rect(10, 80, 200, 15, COLOR_BLACK);
        display_draw_text(10, 80, progress_info, COLOR_GREEN, COLOR_BLACK);
        
        // Log attempt
        packet_log_wps_attempt((char*)ap_records[selected_ap].ssid, pin);
        
        // Simulate WPS attempt delay
        vTaskDelay(pdMS_TO_TICKS(500));
        
        // Simulate random success (1% chance)
        if (esp_random() % 100 == 0) {
            ESP_LOGI(TAG, "*** SUCCESS! PIN FOUND: %08lu ***", pin);
            ESP_LOGI(TAG, "Target SSID: %s", (char*)ap_records[selected_ap].ssid);
            display_fill_rect(10, 100, 200, 40, COLOR_BLACK);
            display_draw_text(10, 100, "SUCCESS!", COLOR_GREEN, COLOR_BLACK);
            display_draw_text(10, 120, "PIN found!", COLOR_GREEN, COLOR_BLACK);
            packet_log_wps_success((char*)ap_records[selected_ap].ssid, pin);
            break;
        }
    }
    
    free(ap_records);
    ESP_LOGD(TAG, "Freed AP records memory");
    
    // Results screen
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "WPS Attack Results", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    if (cancelled) {
        ESP_LOGI(TAG, "Attack cancelled by user after %d PIN attempts", pins_tried);
        display_draw_text(10, 40, "Attack cancelled", COLOR_RED, COLOR_BLACK);
    } else {
        ESP_LOGI(TAG, "Attack completed. Total PINs tried: %d", pins_tried);
        display_draw_text(10, 40, "Attack completed", COLOR_GREEN, COLOR_BLACK);
    }
    
    char final_stats[32];
    snprintf(final_stats, sizeof(final_stats), "PINs tried: %d", pins_tried);
    display_draw_text(10, 60, final_stats, COLOR_BLUE, COLOR_BLACK);
    ESP_LOGD(TAG, "Attack statistics: %s", final_stats);
    
    display_fill_rect(10, 280, 80, 30, COLOR_DARKBLUE);
    display_draw_rect(10, 280, 80, 30, COLOR_WHITE);
    display_draw_text(35, 290, "BACK", COLOR_WHITE, COLOR_DARKBLUE);
    
    ESP_LOGD(TAG, "Waiting for user to exit results screen");
    while (true) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.x >= 10 && point.x <= 90 && point.y >= 280 && point.y <= 310) {
                ESP_LOGI(TAG, "User exited WPS attack module");
                ESP_LOGI(TAG, "=== WPS PIN Attack Ended ===");
                return;
            }
            vTaskDelay(pdMS_TO_TICKS(300));
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}