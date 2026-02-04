#include "ble_packet_capture.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "display.h"
#include "touchscreen.h"
#include "packet_logger.h"
#include <stdio.h>
#include <string.h>

static const char* TAG = "BLE_CAPTURE";

void ble_log_info(const char* msg) {
    ESP_LOGI(TAG, "%s", msg);
}

typedef struct {
    uint8_t addr[6];
    uint8_t data[31];
    uint8_t data_len;
    int8_t rssi;
    uint32_t timestamp;
} ble_packet_t;

static ble_packet_t captured_packets[100];
static int packet_count = 0;
static FILE* capture_file = NULL;

static void ble_capture_callback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    if (event == ESP_GAP_BLE_SCAN_RESULT_EVT) {
        esp_ble_gap_cb_param_t *scan_result = param;
        if (scan_result->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_RES_EVT) {
            
            if (packet_count < 100) {
                // Store packet
                memcpy(captured_packets[packet_count].addr, scan_result->scan_rst.bda, 6);
                captured_packets[packet_count].rssi = scan_result->scan_rst.rssi;
                captured_packets[packet_count].data_len = scan_result->scan_rst.adv_data_len;
                captured_packets[packet_count].timestamp = esp_timer_get_time() / 1000;
                
                if (scan_result->scan_rst.adv_data_len > 0) {
                    memcpy(captured_packets[packet_count].data, 
                           scan_result->scan_rst.ble_adv, 
                           scan_result->scan_rst.adv_data_len > 31 ? 31 : scan_result->scan_rst.adv_data_len);
                }
                
                // Write to file if open
                if (capture_file) {
                    fprintf(capture_file, "%lu,%02X:%02X:%02X:%02X:%02X:%02X,%d,%d,",
                            captured_packets[packet_count].timestamp,
                            scan_result->scan_rst.bda[0], scan_result->scan_rst.bda[1],
                            scan_result->scan_rst.bda[2], scan_result->scan_rst.bda[3],
                            scan_result->scan_rst.bda[4], scan_result->scan_rst.bda[5],
                            scan_result->scan_rst.rssi, scan_result->scan_rst.adv_data_len);
                    
                    // Write hex data
                    for (int i = 0; i < scan_result->scan_rst.adv_data_len && i < 31; i++) {
                        fprintf(capture_file, "%02X", scan_result->scan_rst.ble_adv[i]);
                    }
                    fprintf(capture_file, "\n");
                    fflush(capture_file);
                }
                
                packet_count++;
            }
        }
    }
}

void ble_packet_capture_start(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "BLE Packet Capture", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    display_draw_text(10, 40, "Capturing BLE packets...", COLOR_BLUE, COLOR_BLACK);
    
    // Reset counters
    packet_count = 0;
    
    // Open capture file
    char filename[64];
    snprintf(filename, sizeof(filename), "/sdcard/logs/ble_capture_%u.csv", 
             (unsigned int)(esp_timer_get_time() / 1000000));
    
    capture_file = fopen(filename, "w");
    if (capture_file) {
        fprintf(capture_file, "timestamp,mac_address,rssi,data_length,raw_data\n");
        fflush(capture_file);
    }
    
    // Initialize BLE
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_bt_controller_init(&bt_cfg);
    esp_bt_controller_enable(ESP_BT_MODE_BLE);
    esp_bluedroid_init();
    esp_bluedroid_enable();
    
    esp_ble_gap_register_callback(ble_capture_callback);
    
    esp_ble_scan_params_t scan_params = {
        .scan_type = BLE_SCAN_TYPE_PASSIVE,
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
        .scan_interval = 0x50,
        .scan_window = 0x30,
        .scan_duplicate = BLE_SCAN_DUPLICATE_ENABLE
    };
    
    esp_ble_gap_set_scan_params(&scan_params);
    esp_ble_gap_start_scanning(0); // Continuous scan
    
    // Cancel button
    display_fill_rect(160, 280, 70, 30, COLOR_RED);
    display_draw_rect(160, 280, 70, 30, COLOR_WHITE);
    display_draw_text(180, 290, "STOP", COLOR_WHITE, COLOR_RED);
    
    bool cancelled = false;
    int seconds = 0;
    
    while (!cancelled) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.x >= 160 && point.x <= 230 && point.y >= 280 && point.y <= 310) {
                cancelled = true;
                break;
            }
        }
        
        seconds++;
        char time_info[32];
        snprintf(time_info, sizeof(time_info), "Time: %ds", seconds);
        display_fill_rect(10, 60, 200, 15, COLOR_BLACK);
        display_draw_text(10, 60, time_info, COLOR_BLUE, COLOR_BLACK);
        
        char packet_info[32];
        snprintf(packet_info, sizeof(packet_info), "Packets: %d/100", packet_count);
        display_fill_rect(10, 80, 200, 15, COLOR_BLACK);
        display_draw_text(10, 80, packet_info, COLOR_GREEN, COLOR_BLACK);
        
        if (packet_count >= 100) {
            display_draw_text(10, 100, "Buffer full!", COLOR_RED, COLOR_BLACK);
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    esp_ble_gap_stop_scanning();
    
    if (capture_file) {
        fclose(capture_file);
        capture_file = NULL;
    }
    
    // Results screen
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Capture Results", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    char final_stats[48];
    snprintf(final_stats, sizeof(final_stats), "Captured %d packets in %ds", packet_count, seconds);
    display_draw_text(10, 40, final_stats, COLOR_GREEN, COLOR_BLACK);
    
    display_draw_text(10, 60, "Saved to SD card", COLOR_BLUE, COLOR_BLACK);
    
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