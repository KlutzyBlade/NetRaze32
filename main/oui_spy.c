#include "oui_spy.h"
#include "display.h"
#include "touchscreen.h"
#include "utils.h"
#include "settings.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_bt_main.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include <string.h>
#include <stdlib.h>

static const char* TAG = "OUI_SPY";

// Built-in OUI database - Surveillance/Security devices
typedef struct {
    const char* oui;
    const char* vendor;
} oui_entry_t;

static const oui_entry_t oui_database[] = {
    // Ring (Doorbell/Security)
    {"18:7f:88", "Ring"},
    {"24:2b:d6", "Ring"},
    {"34:3e:a4", "Ring"},
    {"54:e0:19", "Ring"},
    {"5c:47:5e", "Ring"},
    {"64:9a:63", "Ring"},
    {"90:48:6c", "Ring"},
    {"9c:76:13", "Ring"},
    {"ac:9f:c3", "Ring"},
    {"c4:db:ad", "Ring"},
    {"cc:3b:fb", "Ring"},
    // Law Enforcement
    {"00:25:df", "Axon Body Cam"},
    // Surveillance
    {"b4:1e:52", "Flock Safety"},
    // Drones - DJI
    {"0c:9a:e6", "DJI"},
    {"8c:58:23", "DJI"},
    {"04:a8:5a", "DJI"},
    {"58:b8:58", "DJI"},
    {"e4:7a:2c", "DJI"},
    {"60:60:1f", "DJI"},
    {"48:1c:b9", "DJI"},
    {"34:d2:62", "DJI"},
    // Drones - Parrot
    {"00:12:1c", "Parrot"},
    {"00:26:7e", "Parrot"},
    {"90:03:b7", "Parrot"},
    {"90:3a:e6", "Parrot"},
    {"a0:14:3d", "Parrot"},
    // Drones - Skydio
    {"38:1d:14", "Skydio"},
    // Common devices
    {"00:13:37", "Hak5"},
    {"00:c0:ca", "Alfa Networks"},
    {"00:1c:b3", "Apple"},
    {"00:1e:c2", "Apple"},
    {"00:1f:5b", "Apple"},
    {"28:cf:e9", "Apple"},
    {"3c:15:c2", "Apple"},
    {"f0:db:e2", "Apple"},
};

#define OUI_DB_SIZE (sizeof(oui_database) / sizeof(oui_entry_t))

typedef struct {
    char identifier[18];
    char description[32];
    bool is_full_mac;
} oui_filter_t;

typedef struct {
    char mac[18];
    int8_t rssi;
    uint32_t last_seen;
    uint32_t cooldown_until;
} device_info_t;

static oui_filter_t filters[10];
static int filter_count = 0;
static device_info_t devices[50];
static int device_count = 0;
static bool scanning = false;

static void normalize_mac(char* mac) {
    for (int i = 0; mac[i]; i++) {
        if (mac[i] >= 'A' && mac[i] <= 'F') mac[i] += 32;
    }
}

static bool matches_filter(const char* mac, char* matched_desc) {
    char normalized[18];
    strncpy(normalized, mac, sizeof(normalized));
    normalize_mac(normalized);
    
    // Check user filters first
    for (int i = 0; i < filter_count; i++) {
        char filter_id[18];
        strncpy(filter_id, filters[i].identifier, sizeof(filter_id));
        normalize_mac(filter_id);
        
        if (filters[i].is_full_mac) {
            if (strcmp(normalized, filter_id) == 0) {
                strcpy(matched_desc, filters[i].description);
                return true;
            }
        } else {
            if (strncmp(normalized, filter_id, 8) == 0) {
                strcpy(matched_desc, filters[i].description);
                return true;
            }
        }
    }
    
    // Check built-in OUI database
    for (int i = 0; i < OUI_DB_SIZE; i++) {
        if (strncmp(normalized, oui_database[i].oui, 8) == 0) {
            strcpy(matched_desc, oui_database[i].vendor);
            return true;
        }
    }
    
    return false;
}

static void gap_callback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    if (event == ESP_GAP_BLE_SCAN_RESULT_EVT && param->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_RES_EVT) {
        char mac[18];
        snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x",
                 param->scan_rst.bda[0], param->scan_rst.bda[1], param->scan_rst.bda[2],
                 param->scan_rst.bda[3], param->scan_rst.bda[4], param->scan_rst.bda[5]);
        
        char matched_desc[32];
        if (matches_filter(mac, matched_desc)) {
            uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
            
            bool found = false;
            for (int i = 0; i < device_count; i++) {
                if (strcmp(devices[i].mac, mac) == 0) {
                    found = true;
                    if (now >= devices[i].cooldown_until) {
                        devices[i].rssi = param->scan_rst.rssi;
                        devices[i].last_seen = now;
                        devices[i].cooldown_until = now + 3000;
                        ESP_LOGI(TAG, "Re-detected: %s (%s) RSSI: %d", mac, matched_desc, param->scan_rst.rssi);
                    }
                    break;
                }
            }
            
            if (!found && device_count < 50) {
                strncpy(devices[device_count].mac, mac, sizeof(devices[device_count].mac));
                devices[device_count].rssi = param->scan_rst.rssi;
                devices[device_count].last_seen = now;
                devices[device_count].cooldown_until = now + 3000;
                device_count++;
                ESP_LOGI(TAG, "NEW: %s (%s) RSSI: %d", mac, matched_desc, param->scan_rst.rssi);
            }
        }
    }
}

esp_err_t oui_spy_init(void) {
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_bt_controller_init(&bt_cfg);
    esp_bt_controller_enable(ESP_BT_MODE_BLE);
    esp_bluedroid_init();
    esp_bluedroid_enable();
    esp_ble_gap_register_callback(gap_callback);
    
    ESP_LOGI(TAG, "OUI-Spy initialized");
    return ESP_OK;
}

void oui_spy_add_filter(const char* mac_or_oui, const char* description) {
    if (filter_count >= 10) return;
    
    strncpy(filters[filter_count].identifier, mac_or_oui, sizeof(filters[filter_count].identifier) - 1);
    strncpy(filters[filter_count].description, description, sizeof(filters[filter_count].description) - 1);
    filters[filter_count].is_full_mac = (strlen(mac_or_oui) == 17);
    filter_count++;
    
    ESP_LOGI(TAG, "Added filter: %s (%s)", mac_or_oui, description);
}

void oui_spy_clear_filters(void) {
    filter_count = 0;
    device_count = 0;
}

int oui_spy_get_device_count(void) {
    return device_count;
}

void oui_spy_scan(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "OUI-Spy Scanner", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_ORANGE);
    
    char info[64];
    snprintf(info, sizeof(info), "DB: %d OUIs | Filters: %d", OUI_DB_SIZE, filter_count);
    display_draw_text(10, 40, info, COLOR_BLUE, COLOR_BLACK);
    display_draw_text(10, 60, "Scanning...", COLOR_GREEN, COLOR_BLACK);
    
    esp_ble_scan_params_t scan_params = {
        .scan_type = BLE_SCAN_TYPE_ACTIVE,
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
        .scan_interval = 0x50,
        .scan_window = 0x30,
        .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE
    };
    
    esp_ble_gap_set_scan_params(&scan_params);
    esp_ble_gap_start_scanning(30);
    scanning = true;
    
    uint32_t start = xTaskGetTickCount();
    int y = 80;
    
    while ((xTaskGetTickCount() - start) < pdMS_TO_TICKS(30000)) {
        snprintf(info, sizeof(info), "Detected: %d devices", device_count);
        display_fill_rect(10, 80, 220, 20, COLOR_BLACK);
        display_draw_text(10, 80, info, COLOR_GREEN, COLOR_BLACK);
        
        y = 100;
        for (int i = 0; i < device_count && y < 260; i++) {
            char dev_info[48];
            snprintf(dev_info, sizeof(dev_info), "%.17s %d", devices[i].mac, devices[i].rssi);
            display_draw_text(10, y, dev_info, COLOR_WHITE, COLOR_BLACK);
            y += 15;
        }
        
        touch_point_t p = touchscreen_get_point();
        if (p.pressed) break;
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    esp_ble_gap_stop_scanning();
    scanning = false;
    
    display_draw_text(10, 280, "Scan complete", COLOR_GREEN, COLOR_BLACK);
    vTaskDelay(pdMS_TO_TICKS(2000));
}


// Flock Safety detector - WiFi + BLE combined
static int flock_wifi_count = 0;
static int flock_ble_count = 0;

static void wifi_sniffer_callback(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_MGMT) return;
    
    wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t*)buf;
    if (pkt->rx_ctrl.sig_len < 24) return; // Ensure minimum frame size
    
    uint8_t *payload = pkt->payload;
    
    // Check for Flock Safety MAC (B4:1E:52) in all MAC address fields
    // Destination MAC (offset 4-9), Source MAC (offset 10-15), BSSID (offset 16-21)
    bool is_flock = false;
    
    // Check destination address
    if (payload[4] == 0xb4 && payload[5] == 0x1e && payload[6] == 0x52) {
        is_flock = true;
    }
    // Check source address
    else if (payload[10] == 0xb4 && payload[11] == 0x1e && payload[12] == 0x52) {
        is_flock = true;
    }
    // Check BSSID
    else if (payload[16] == 0xb4 && payload[17] == 0x1e && payload[18] == 0x52) {
        is_flock = true;
    }
    
    if (is_flock) {
        flock_wifi_count++;
        ESP_LOGI(TAG, "Flock WiFi detected!");
    }
}

static void flock_ble_callback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    if (event == ESP_GAP_BLE_SCAN_RESULT_EVT && param->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_RES_EVT) {
        // Check for Flock Safety BLE MAC (B4:1E:52)
        if (param->scan_rst.bda[0] == 0xb4 && 
            param->scan_rst.bda[1] == 0x1e && 
            param->scan_rst.bda[2] == 0x52) {
            flock_ble_count++;
            ESP_LOGI(TAG, "Flock BLE detected!");
        }
    }
}

void oui_spy_flock_detector(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Flock Safety Detector", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_RED);
    display_draw_text(10, 40, "WiFi + BLE Scanning", COLOR_ORANGE, COLOR_BLACK);
    display_draw_text(10, 60, "Detecting cameras...", COLOR_GREEN, COLOR_BLACK);
    
    flock_wifi_count = 0;
    flock_ble_count = 0;
    
    // Initialize WiFi if needed
    esp_wifi_start();
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Start WiFi promiscuous mode
    esp_wifi_set_promiscuous_rx_cb(wifi_sniffer_callback);
    esp_wifi_set_promiscuous(true);
    
    // Register BLE callback for Flock detection
    esp_ble_gap_register_callback(flock_ble_callback);
    
    // Start BLE scan
    esp_ble_scan_params_t scan_params = {
        .scan_type = BLE_SCAN_TYPE_ACTIVE,
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
        .scan_interval = 0x50,
        .scan_window = 0x30,
        .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE
    };
    esp_ble_gap_set_scan_params(&scan_params);
    esp_ble_gap_start_scanning(30);
    
    uint32_t start = xTaskGetTickCount();
    
    while ((xTaskGetTickCount() - start) < pdMS_TO_TICKS(30000)) {
        char info[64];
        snprintf(info, sizeof(info), "WiFi: %d | BLE: %d", flock_wifi_count, flock_ble_count);
        display_fill_rect(10, 80, 220, 20, COLOR_BLACK);
        display_draw_text(10, 80, info, flock_wifi_count > 0 || flock_ble_count > 0 ? COLOR_RED : COLOR_GREEN, COLOR_BLACK);
        
        if (flock_wifi_count > 0 || flock_ble_count > 0) {
            display_draw_text(10, 110, "FLOCK DETECTED!", COLOR_RED, COLOR_BLACK);
            display_draw_text(10, 130, "Surveillance camera", COLOR_ORANGE, COLOR_BLACK);
            display_draw_text(10, 150, "in range", COLOR_ORANGE, COLOR_BLACK);
        } else {
            display_draw_text(10, 110, "No Flock cameras", COLOR_GREEN, COLOR_BLACK);
            display_draw_text(10, 130, "detected", COLOR_GREEN, COLOR_BLACK);
        }
        
        touch_point_t p = touchscreen_get_point();
        if (p.pressed) break;
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    // Cleanup
    esp_wifi_set_promiscuous(false);
    esp_wifi_set_promiscuous_rx_cb(NULL);
    esp_ble_gap_stop_scanning();
    esp_ble_gap_register_callback(gap_callback); // Restore original callback
    
    display_draw_text(10, 280, "Scan complete", COLOR_GREEN, COLOR_BLACK);
    vTaskDelay(pdMS_TO_TICKS(2000));
}
