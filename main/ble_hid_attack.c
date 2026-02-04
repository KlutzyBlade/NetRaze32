#include "ble_hid_attack.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_log.h"
#include "display.h"
#include "touchscreen.h"
#include <string.h>

static const char* TAG = "BLE_HID";

// HID Report Map for keyboard
static const uint8_t __attribute__((unused)) hid_report_map[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
    0x19, 0xE0,        //   Usage Minimum (0xE0)
    0x29, 0xE7,        //   Usage Maximum (0xE7)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x08,        //   Report Count (8)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x06,        //   Report Count (6)
    0x75, 0x08,        //   Report Size (8)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x65,        //   Logical Maximum (101)
    0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
    0x19, 0x00,        //   Usage Minimum (0x00)
    0x29, 0x65,        //   Usage Maximum (0x65)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
};

static bool hid_connected = false;
static uint16_t hid_conn_id = 0;

static void ble_hid_gatts_cb(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    switch (event) {
        case ESP_GATTS_CONNECT_EVT:
            hid_connected = true;
            hid_conn_id = param->connect.conn_id;
            ESP_LOGI(TAG, "HID device connected");
            break;
        case ESP_GATTS_DISCONNECT_EVT:
            hid_connected = false;
            ESP_LOGI(TAG, "HID device disconnected");
            break;
        default:
            break;
    }
}

static void send_key_report(uint8_t modifier, uint8_t keycode) {
    if (!hid_connected) return;
    
    uint8_t report[8] = {0};
    report[0] = modifier;  // Modifier keys
    report[2] = keycode;   // Key code
    
    // Send HID report (simplified - would need proper GATT service setup)
    ESP_LOGI(TAG, "Sending key: modifier=0x%02X, key=0x%02X", modifier, keycode);
    (void)report; // Suppress unused variable warning
}

void ble_hid_keyboard_attack(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "BLE HID Keyboard", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    display_draw_text(10, 40, "Setting up HID device...", COLOR_BLUE, COLOR_BLACK);
    
    // Initialize BLE
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_bt_controller_init(&bt_cfg);
    esp_bt_controller_enable(ESP_BT_MODE_BLE);
    esp_bluedroid_init();
    esp_bluedroid_enable();
    
    // Register GATT server callback
    esp_ble_gatts_register_callback(ble_hid_gatts_cb);
    
    // Set device name
    esp_ble_gap_set_device_name("Wireless Keyboard");
    
    // Configure advertising
    esp_ble_adv_data_t adv_data = {
        .set_scan_rsp = false,
        .include_name = true,
        .include_txpower = false,
        .min_interval = 0x0006,
        .max_interval = 0x0010,
        .appearance = 0x03C1, // HID Keyboard
        .manufacturer_len = 0,
        .p_manufacturer_data = NULL,
        .service_data_len = 0,
        .p_service_data = NULL,
        .service_uuid_len = 0,
        .p_service_uuid = NULL,
        .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
    };
    
    esp_ble_gap_config_adv_data(&adv_data);
    
    esp_ble_adv_params_t adv_params = {
        .adv_int_min = 0x20,
        .adv_int_max = 0x40,
        .adv_type = ADV_TYPE_IND,
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .channel_map = ADV_CHNL_ALL,
        .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
    };
    
    esp_ble_gap_start_advertising(&adv_params);
    
    display_draw_text(10, 60, "Advertising as keyboard...", COLOR_GREEN, COLOR_BLACK);
    display_draw_text(10, 80, "Waiting for connection...", COLOR_ORANGE, COLOR_BLACK);
    
    // Attack selection menu
    display_fill_rect(10, 120, 200, 25, COLOR_DARKBLUE);
    display_draw_rect(10, 120, 200, 25, COLOR_WHITE);
    display_draw_text(15, 130, "Send Test Keys", COLOR_WHITE, COLOR_DARKBLUE);
    
    display_fill_rect(10, 150, 200, 25, COLOR_DARKBLUE);
    display_draw_rect(10, 150, 200, 25, COLOR_WHITE);
    display_draw_text(15, 160, "Type Message", COLOR_WHITE, COLOR_DARKBLUE);
    
    display_fill_rect(10, 180, 200, 25, COLOR_DARKBLUE);
    display_draw_rect(10, 180, 200, 25, COLOR_WHITE);
    display_draw_text(15, 190, "Ctrl+Alt+Del", COLOR_WHITE, COLOR_DARKBLUE);
    
    display_fill_rect(10, 280, 80, 30, COLOR_RED);
    display_draw_rect(10, 280, 80, 30, COLOR_WHITE);
    display_draw_text(35, 290, "STOP", COLOR_WHITE, COLOR_RED);
    
    bool running = true;
    while (running) {
        // Update connection status
        if (hid_connected) {
            display_fill_rect(10, 100, 200, 15, COLOR_BLACK);
            display_draw_text(10, 100, "Connected! Ready to attack", COLOR_GREEN, COLOR_BLACK);
        } else {
            display_fill_rect(10, 100, 200, 15, COLOR_BLACK);
            display_draw_text(10, 100, "Not connected", COLOR_RED, COLOR_BLACK);
        }
        
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            
            if (point.x >= 10 && point.x <= 210) {
                if (point.y >= 120 && point.y <= 145 && hid_connected) {
                    // Send test keys
                    send_key_report(0x00, 0x04); // 'a'
                    vTaskDelay(pdMS_TO_TICKS(100));
                    send_key_report(0x00, 0x00); // Release
                    vTaskDelay(pdMS_TO_TICKS(100));
                    send_key_report(0x00, 0x05); // 'b'
                    vTaskDelay(pdMS_TO_TICKS(100));
                    send_key_report(0x00, 0x00); // Release
                }
                else if (point.y >= 150 && point.y <= 175 && hid_connected) {
                    // Type "HACKED"
                    uint8_t message[] = {0x0B, 0x04, 0x06, 0x0E, 0x08, 0x07}; // HACKED
                    for (int i = 0; i < 6; i++) {
                        send_key_report(0x00, message[i]);
                        vTaskDelay(pdMS_TO_TICKS(100));
                        send_key_report(0x00, 0x00);
                        vTaskDelay(pdMS_TO_TICKS(100));
                    }
                }
                else if (point.y >= 180 && point.y <= 205 && hid_connected) {
                    // Ctrl+Alt+Del
                    send_key_report(0x05, 0x4C); // Ctrl+Alt+Del
                    vTaskDelay(pdMS_TO_TICKS(100));
                    send_key_report(0x00, 0x00);
                }
            }
            
            if (point.x >= 10 && point.x <= 90 && point.y >= 280 && point.y <= 310) {
                running = false;
            }
            
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    esp_ble_gap_stop_advertising();
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "HID Attack Stopped", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    display_draw_text(10, 40, "Attack completed", COLOR_GREEN, COLOR_BLACK);
    display_draw_text(10, 280, "Touch to continue", COLOR_GRAY, COLOR_BLACK);
    
    while (!touchscreen_is_touched()) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}