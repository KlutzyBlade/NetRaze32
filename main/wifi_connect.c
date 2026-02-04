#include "wifi_connect.h"
#include "display.h"
#include "touchscreen.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "time_sync.h"
#include <string.h>

static const char* TAG = "WIFI_CONNECT";
static EventGroupHandle_t wifi_event_group;
static const int WIFI_CONNECTED_BIT = BIT0;
static const int WIFI_FAIL_BIT = BIT1;

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

esp_err_t wifi_connect_to_network(const char* ssid, const char* password) {
    wifi_event_group = xEventGroupCreate();
    
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, pdMS_TO_TICKS(10000));

    esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip);
    esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id);
    vEventGroupDelete(wifi_event_group);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to WiFi: %s", ssid);
        return ESP_OK;
    } else {
        ESP_LOGI(TAG, "Failed to connect to WiFi: %s", ssid);
        return ESP_FAIL;
    }
}

void wifi_connect_menu(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "WiFi Connect", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    // Scan for networks
    display_draw_text(10, 40, "Scanning networks...", COLOR_ORANGE, COLOR_BLACK);
    
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = false
    };
    
    esp_wifi_scan_start(&scan_config, true);
    
    uint16_t ap_count = 0;
    esp_wifi_scan_get_ap_num(&ap_count);
    
    if (ap_count > 5) ap_count = 5; // Limit to 5 networks
    
    wifi_ap_record_t ap_info[5];
    esp_wifi_scan_get_ap_records(&ap_count, ap_info);
    
    display_fill_rect(10, 40, 220, 20, COLOR_BLACK);
    display_draw_text(10, 40, "Select network:", COLOR_WHITE, COLOR_BLACK);
    
    // Display networks
    for (int i = 0; i < ap_count; i++) {
        int y = 60 + i * 25;
        display_fill_rect(10, y, 220, 23, COLOR_DARKBLUE);
        display_draw_rect(10, y, 220, 23, COLOR_GRAY);
        
        char network_info[32];
        snprintf(network_info, sizeof(network_info), "%.20s (%d)", (char*)ap_info[i].ssid, ap_info[i].rssi);
        display_draw_text(15, y + 8, network_info, COLOR_WHITE, COLOR_DARKBLUE);
    }
    
    display_fill_rect(10, 280, 80, 30, COLOR_DARKBLUE);
    display_draw_rect(10, 280, 80, 30, COLOR_WHITE);
    display_draw_text(35, 290, "BACK", COLOR_WHITE, COLOR_DARKBLUE);
    
    while (true) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.pressed) {
                if (point.x >= 10 && point.x <= 90 && point.y >= 280 && point.y <= 310) {
                    break; // Back
                }
                
                // Check network selection
                for (int i = 0; i < ap_count; i++) {
                    int y = 60 + i * 25;
                    if (point.y >= y && point.y <= y + 23) {
                        // Selected network - show password input
                        display_fill_screen(COLOR_BLACK);
                        display_draw_text(10, 10, "WiFi Connect", COLOR_WHITE, COLOR_BLACK);
                        display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
                        
                        char selected_ssid[32];
                        snprintf(selected_ssid, sizeof(selected_ssid), "SSID: %.20s", (char*)ap_info[i].ssid);
                        display_draw_text(10, 40, selected_ssid, COLOR_GREEN, COLOR_BLACK);
                        
                        // Virtual keyboard for password input
                        char password[32] = "";
                        int keyboard_mode = 0; // 0=lowercase, 1=uppercase, 2=numbers, 3=symbols
                        char keyboards[4][30] = {
                            "qwertyuiopasdfghjklzxcvbnm",
                            "QWERTYUIOPASDFGHJKLZXCVBNM",
                            "1234567890!@#$%^&*()_+-=<>",
                            "[]{}|\\;:'\",.<>/?~`      "
                        };
                        char* mode_names[] = {"abc", "ABC", "123", "!@#"};
                        
                        display_draw_text(10, 70, "Enter Password:", COLOR_GRAY, COLOR_BLACK);
                        
                        bool entering_password = true;
                        bool need_redraw = true;
                        
                        while (entering_password) {
                            if (need_redraw) {
                                display_fill_screen(COLOR_BLACK);
                                display_draw_text(10, 10, "WiFi Connect", COLOR_WHITE, COLOR_BLACK);
                                display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
                                
                                char selected_ssid[32];
                                snprintf(selected_ssid, sizeof(selected_ssid), "SSID: %.20s", (char*)ap_info[i].ssid);
                                display_draw_text(10, 40, selected_ssid, COLOR_GREEN, COLOR_BLACK);
                                display_draw_text(10, 60, "Enter Password:", COLOR_GRAY, COLOR_BLACK);
                                
                                // Show current password
                                display_fill_rect(10, 80, 220, 20, COLOR_DARKGRAY);
                                display_draw_rect(10, 80, 220, 20, COLOR_WHITE);
                                char masked_pwd[32];
                                for (int p = 0; p < strlen(password); p++) {
                                    masked_pwd[p] = '*';
                                }
                                masked_pwd[strlen(password)] = '\0';
                                display_draw_text(15, 88, masked_pwd, COLOR_WHITE, COLOR_DARKGRAY);
                                
                                // Show current mode
                                display_draw_text(10, 105, mode_names[keyboard_mode], COLOR_BLUE, COLOR_BLACK);
                                
                                // Draw QWERTY keyboard layout
                                // Row 1: QWERTY (10 keys)
                                for (int k = 0; k < 10; k++) {
                                    int kx = 10 + (9 - k) * 22;  // Reverse the x position
                                    int ky = 120;
                                    display_fill_rect(kx, ky, 20, 18, COLOR_DARKBLUE);
                                    display_draw_rect(kx, ky, 20, 18, COLOR_GRAY);
                                    char key_str[2] = {keyboards[keyboard_mode][k], '\0'};
                                    display_draw_text(kx + 7, ky + 5, key_str, COLOR_WHITE, COLOR_DARKBLUE);
                                }
                                
                                // Row 2: ASDF (9 keys)
                                for (int k = 0; k < 9; k++) {
                                    int kx = 21 + (8 - k) * 22;  // Reverse the x position
                                    int ky = 140;
                                    display_fill_rect(kx, ky, 20, 18, COLOR_DARKBLUE);
                                    display_draw_rect(kx, ky, 20, 18, COLOR_GRAY);
                                    char key_str[2] = {keyboards[keyboard_mode][k + 10], '\0'};
                                    display_draw_text(kx + 7, ky + 5, key_str, COLOR_WHITE, COLOR_DARKBLUE);
                                }
                                
                                // Row 3: ZXCV (7 keys)
                                for (int k = 0; k < 7; k++) {
                                    int kx = 32 + (6 - k) * 22;  // Reverse the x position
                                    int ky = 160;
                                    display_fill_rect(kx, ky, 20, 18, COLOR_DARKBLUE);
                                    display_draw_rect(kx, ky, 20, 18, COLOR_GRAY);
                                    char key_str[2] = {keyboards[keyboard_mode][k + 19], '\0'};
                                    display_draw_text(kx + 7, ky + 5, key_str, COLOR_WHITE, COLOR_DARKBLUE);
                                }
                                
                                // Space bar
                                display_fill_rect(10, 180, 100, 18, COLOR_DARKBLUE);
                                display_draw_rect(10, 180, 100, 18, COLOR_GRAY);
                                display_draw_text(50, 185, "SPACE", COLOR_WHITE, COLOR_DARKBLUE);
                                
                                // Control buttons
                                display_fill_rect(10, 240, 40, 20, COLOR_PURPLE);
                                display_draw_rect(10, 240, 40, 20, COLOR_WHITE);
                                display_draw_text(20, 248, "MODE", COLOR_WHITE, COLOR_PURPLE);
                                
                                display_fill_rect(55, 240, 40, 20, COLOR_RED);
                                display_draw_rect(55, 240, 40, 20, COLOR_WHITE);
                                display_draw_text(68, 248, "DEL", COLOR_WHITE, COLOR_RED);
                                
                                display_fill_rect(100, 240, 60, 20, COLOR_GREEN);
                                display_draw_rect(100, 240, 60, 20, COLOR_WHITE);
                                display_draw_text(115, 248, "CONNECT", COLOR_WHITE, COLOR_GREEN);
                                
                                display_fill_rect(165, 240, 50, 20, COLOR_ORANGE);
                                display_draw_rect(165, 240, 50, 20, COLOR_WHITE);
                                display_draw_text(175, 248, "CANCEL", COLOR_WHITE, COLOR_ORANGE);
                                
                                need_redraw = false;
                            }
                            
                            if (touchscreen_is_touched()) {
                                touch_point_t kbd_point = touchscreen_get_point();
                                if (kbd_point.pressed) {
                                    // Check keyboard keys
                                    bool key_pressed = false;
                                    
                                    // Row 1 (QWERTY)
                                    for (int k = 0; k < 10 && !key_pressed; k++) {
                                        int kx = 10 + (9 - k) * 22;  // Reverse the x position
                                        if (kbd_point.x >= kx && kbd_point.x <= kx + 20 &&
                                            kbd_point.y >= 120 && kbd_point.y <= 138) {
                                            if (strlen(password) < 31) {
                                                password[strlen(password)] = keyboards[keyboard_mode][k];
                                                password[strlen(password)] = '\0';
                                                need_redraw = true;
                                            }
                                            key_pressed = true;
                                        }
                                    }
                                    
                                    // Row 2 (ASDF)
                                    for (int k = 0; k < 9 && !key_pressed; k++) {
                                        int kx = 21 + (8 - k) * 22;  // Reverse the x position
                                        if (kbd_point.x >= kx && kbd_point.x <= kx + 20 &&
                                            kbd_point.y >= 140 && kbd_point.y <= 158) {
                                            if (strlen(password) < 31) {
                                                password[strlen(password)] = keyboards[keyboard_mode][k + 10];
                                                password[strlen(password)] = '\0';
                                                need_redraw = true;
                                            }
                                            key_pressed = true;
                                        }
                                    }
                                    
                                    // Row 3 (ZXCV)
                                    for (int k = 0; k < 7 && !key_pressed; k++) {
                                        int kx = 32 + (6 - k) * 22;  // Reverse the x position
                                        if (kbd_point.x >= kx && kbd_point.x <= kx + 20 &&
                                            kbd_point.y >= 160 && kbd_point.y <= 178) {
                                            if (strlen(password) < 31) {
                                                password[strlen(password)] = keyboards[keyboard_mode][k + 19];
                                                password[strlen(password)] = '\0';
                                                need_redraw = true;
                                            }
                                            key_pressed = true;
                                        }
                                    }
                                    
                                    // Space bar
                                    if (kbd_point.x >= 10 && kbd_point.x <= 110 &&
                                        kbd_point.y >= 180 && kbd_point.y <= 198) {
                                        if (strlen(password) < 31) {
                                            password[strlen(password)] = ' ';
                                            password[strlen(password)] = '\0';
                                            need_redraw = true;
                                        }
                                        key_pressed = true;
                                    }
                                    
                                    // Check control buttons
                                    if (kbd_point.x >= 10 && kbd_point.x <= 50 && kbd_point.y >= 240 && kbd_point.y <= 260) {
                                        // Mode switch
                                        keyboard_mode = (keyboard_mode + 1) % 4;
                                        need_redraw = true;
                                    } else if (kbd_point.x >= 55 && kbd_point.x <= 95 && kbd_point.y >= 240 && kbd_point.y <= 260) {
                                        // Delete
                                        if (strlen(password) > 0) {
                                            password[strlen(password) - 1] = '\0';
                                            need_redraw = true;
                                        }
                                    } else if (kbd_point.x >= 100 && kbd_point.x <= 160 && kbd_point.y >= 240 && kbd_point.y <= 260) {
                                        // Connect
                                        entering_password = false;
                                    } else if (kbd_point.x >= 165 && kbd_point.x <= 215 && kbd_point.y >= 240 && kbd_point.y <= 260) {
                                        // Cancel
                                        return;
                                    }
                                    
                                    vTaskDelay(pdMS_TO_TICKS(200));
                                }
                            }
                            vTaskDelay(pdMS_TO_TICKS(50));
                        }
                        
                        // Connect with entered password
                        display_draw_text(10, 260, "Connecting...", COLOR_ORANGE, COLOR_BLACK);
                        esp_err_t result = wifi_connect_to_network((char*)ap_info[i].ssid, password);
                        
                        if (result == ESP_OK) {
                            display_draw_text(10, 280, "Connected!", COLOR_GREEN, COLOR_BLACK);
                            time_sync_start();
                        } else {
                            display_draw_text(10, 280, "Failed to connect", COLOR_RED, COLOR_BLACK);
                        }
                        
                        vTaskDelay(pdMS_TO_TICKS(3000));
                        return;
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}