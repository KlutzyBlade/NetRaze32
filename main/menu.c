#include "menu.h"
#include "ui_dialogs.h"
#include "stats.h"
#include "display.h"
#include "touchscreen.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifi_functions.h"
#include "wifi_attacks_enhanced.h"
#include "bluetooth_functions.h"
#include "ble_attacks_enhanced.h"
#include "attack_scheduler.h"
#include "target_manager.h"
#include "settings_menu.h"
#include "rf_functions.h"
#include "subghz_protocols.h"
#include "cc1101_driver.h"
#include "ir_functions.h"
#include "nfc_functions.h"
#include "gps_functions.h"
#include "badusb_functions.h"
#include "sd_browser.h"
#include "battery_monitor.h"
#include "usb_msc.h"
#include "antenna_indicator.h"
#include "confirmation_dialog.h"
#include "brightness_control.h"
#include "stats_tracker.h"
#include "attack_timer.h"
#include "oui_spy.h"
#include <string.h>

static const char* TAG = "MENU";
static menu_state_t menu_state = {0};
static int scroll_offset = 0;

static const char* menu_items[] = {
    "WiFi Attacks",
    "Bluetooth",
    "SubGHz RF",
    "IR Remote",
    "NFC/RFID",
    "GPS",
    "BadUSB",
    "Automation",
    "Tools",
    "Settings"
};

// WiFi submenu items
static const char* wifi_menu_items[] = {
    "WiFi Scanner",
    "Deauth Attack",
    "Beacon Spam",
    "Evil Twin",
    "Packet Capture",
    "Probe Sniffer",
    "EAPOL Sniffer",
    "Karma Attack",
    "Rickroll",
    "Pineapple Detect",
    "Back"
};

// Bluetooth submenu items
static const char* bt_menu_items[] = {
    "BLE Scanner",
    "Flock Detector",
    "Targeted Attack",
    "Apple Juice",
    "Sour Apple",
    "Samsung Watch",
    "FastPair Spam",
    "Beacon Flood",
    "BLE Jammer",
    "BLE Sniffer",
    "Back"
};

// SubGHz RF submenu items
static const char* rf_sub_menu_items[] = {
    "Spectrum Analyzer",
    "Signal Capture",
    "Signal Replay",
    "Garage Doors",
    "Car Keys",
    "Back"
};

// IR submenu items
static const char* ir_menu_items[] = {
    "TV Power",
    "Volume Control",
    "Channel Control",
    "Back"
};

// NFC submenu items
static const char* nfc_menu_items[] = {
    "Scan Cards",
    "Clone Card",
    "Emulate Card",
    "Back"
};

// Tools submenu items
static const char* tools_menu_items[] = {
    "Target Manager",
    "Packet Logger",
    "Signal Analyzer",
    "SD Browser",
    "USB Mass Storage",
    "Back"
};

// Settings submenu items
static const char* settings_menu_items[] = {
    "Display",
    "Network",
    "System",
    "Battery Info",
    "Antenna Mode",
    "Brightness",
    "Back"
};

void menu_init(void) {
    menu_state.current_index = 0;
    menu_state.in_submenu = false;
    menu_state.initialized = true;
    scroll_offset = 0;
    ESP_LOGI(TAG, "Menu initialized");
}

void draw_header(void) {
    // Header background
    display_fill_rect(0, 0, DISPLAY_WIDTH, 30, COLOR_DARKBLUE);
    display_draw_text(10, 8, "NetRaze32 v1.0", COLOR_WHITE, COLOR_DARKBLUE);
    display_draw_text(DISPLAY_WIDTH - 50, 8, "READY", COLOR_GREEN, COLOR_DARKBLUE);
}

static uint16_t get_menu_color(int index) {
    if (index == MENU_WIFI || index == MENU_BLUETOOTH || index == MENU_SUBGHZ || 
        index == MENU_IR_REMOTE || index == MENU_NFC_RFID) {
        return COLOR_RED;
    } else if (index == MENU_TOOLS || index == MENU_GPS) {
        return COLOR_GREEN;
    } else if (index == MENU_SETTING) {
        return COLOR_BLUE;
    } else if (index == MENU_BADUSB || index == MENU_AUTOMATION) {
        return COLOR_ORANGE;
    }
    return COLOR_GRAY;
}

void draw_menu_box(int x, int y, int w, int h, const char* text, bool selected, int index) {
    uint16_t base_color = get_menu_color(index);
    uint16_t bg_color = selected ? base_color : COLOR_DARKGRAY;
    uint16_t text_color = selected ? COLOR_WHITE : COLOR_GRAY;
    uint16_t border_color = selected ? COLOR_WHITE : base_color;
    
    display_fill_rect(x, y, w, h, bg_color);
    display_draw_rect(x, y, w, h, border_color);
    
    int text_x = x + (w - strlen(text) * 6) / 2;
    int text_y = y + (h - 7) / 2;
    display_draw_text(text_x, text_y, text, text_color, bg_color);
}

void menu_draw(void) {
    display_fill_screen(COLOR_BLACK);
    draw_header();
    
    if (!menu_state.in_submenu) {
        // Main menu - draw in grid layout
        int items_per_row = 2;
        int box_width = (DISPLAY_WIDTH - 30) / items_per_row;
        int box_height = 40;
        int start_y = 40;
        
        for (int i = 0; i < MENU_COUNT; i++) {
            int row = i / items_per_row;
            int col = i % items_per_row;
            int x = 10 + col * (box_width + 10);
            int y = start_y + row * (box_height + 10);
            
            // Only draw if it fits on screen
            if (y + box_height <= DISPLAY_HEIGHT - 10) {
                bool selected = (i == menu_state.current_index);
                draw_menu_box(x, y, box_width, box_height, menu_items[i], selected, i);
            }
        }
    } else {
        // Submenu - draw as list
        display_draw_text(10, 35, "Select Option:", COLOR_WHITE, COLOR_BLACK);
        
        const char** submenu_items = NULL;
        int submenu_count = 0;
        
        switch (menu_state.current_index) {
            case MENU_WIFI:
                submenu_items = wifi_menu_items;
                submenu_count = 11;
                break;
            case MENU_BLUETOOTH:
                submenu_items = bt_menu_items;
                submenu_count = 11;
                break;
            case MENU_SUBGHZ:
                submenu_items = rf_sub_menu_items;
                submenu_count = 6;
                break;
            case MENU_IR_REMOTE:
                submenu_items = ir_menu_items;
                submenu_count = 4;
                break;
            case MENU_NFC_RFID:
                submenu_items = nfc_menu_items;
                submenu_count = 4;
                break;
            case MENU_TOOLS:
                submenu_items = tools_menu_items;
                submenu_count = 6;
                break;
            case MENU_SETTING:
                submenu_items = settings_menu_items;
                submenu_count = 7;
                break;
            default:
                menu_state.in_submenu = false;
                menu_draw();
                return;
        }
        
        int items_per_page = 9;
        int start_idx = scroll_offset * items_per_page;
        int end_idx = start_idx + items_per_page;
        if (end_idx > submenu_count) end_idx = submenu_count;
        
        for (int i = start_idx; i < end_idx; i++) {
            int y = 60 + (i - start_idx) * 22;
            draw_menu_box(10, y, DISPLAY_WIDTH - 20, 18, submenu_items[i], false, -1);
        }
        
        // Page indicator and scroll buttons
        int total_pages = (submenu_count + items_per_page - 1) / items_per_page;
        if (total_pages > 1) {
            char page_str[20];
            snprintf(page_str, sizeof(page_str), "Page %d/%d", scroll_offset + 1, total_pages);
            display_draw_text(80, 260, page_str, COLOR_BLUE, COLOR_BLACK);
            
            // Scroll buttons
            if (scroll_offset > 0) {
                display_fill_rect(10, 255, 50, 20, COLOR_ORANGE);
                display_draw_text(15, 260, "PREV", COLOR_WHITE, COLOR_ORANGE);
            }
            if (end_idx < submenu_count) {
                display_fill_rect(180, 255, 50, 20, COLOR_ORANGE);
                display_draw_text(185, 260, "NEXT", COLOR_WHITE, COLOR_ORANGE);
            }
        }
    }
}

static void show_schedule_prompt(attack_type_t attack_type) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Schedule Attack?", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    display_draw_text(10, 40, "Add to automation queue?", COLOR_ORANGE, COLOR_BLACK);
    
    display_fill_rect(20, 100, 90, 30, COLOR_GREEN);
    display_draw_text(35, 110, "ONCE", COLOR_WHITE, COLOR_GREEN);
    
    display_fill_rect(130, 100, 90, 30, COLOR_BLUE);
    display_draw_text(145, 110, "REPEAT", COLOR_WHITE, COLOR_BLUE);
    
    display_fill_rect(60, 150, 120, 30, COLOR_RED);
    display_draw_text(95, 160, "RUN NOW", COLOR_WHITE, COLOR_RED);
    
    while (true) {
        touch_point_t point = touchscreen_get_point();
        if (point.pressed) {
            if (point.y >= 100 && point.y <= 130) {
                if (point.x >= 20 && point.x <= 110) {
                    if (attack_scheduler_add(attack_type, SCHED_ONCE, 30, 0, 1) == ESP_OK) {
                        display_draw_text(10, 200, "Added to queue!", COLOR_GREEN, COLOR_BLACK);
                    } else {
                        display_draw_text(10, 200, "Failed to add", COLOR_RED, COLOR_BLACK);
                    }
                    vTaskDelay(pdMS_TO_TICKS(1500));
                    return;
                } else if (point.x >= 130 && point.x <= 220) {
                    if (attack_scheduler_add(attack_type, SCHED_REPEAT, 30, 60, 5) == ESP_OK) {
                        display_draw_text(10, 200, "Added 5x repeats!", COLOR_GREEN, COLOR_BLACK);
                    } else {
                        display_draw_text(10, 200, "Failed to add", COLOR_RED, COLOR_BLACK);
                    }
                    vTaskDelay(pdMS_TO_TICKS(1500));
                    return;
                }
            } else if (point.y >= 150 && point.y <= 180 && point.x >= 60 && point.x <= 180) {
                return;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void execute_wifi_function(int index) {
    attack_type_t attack_type = ATTACK_WIFI_DEAUTH;
    bool show_prompt = false;
    esp_err_t ret = ESP_OK;
    
    switch (index) {
        case 0: 
            stats_increment_scans();
            wifi_scan_start(); 
            break;
        case 1: attack_type = ATTACK_WIFI_DEAUTH; show_prompt = true; break;
        case 2: attack_type = ATTACK_WIFI_BEACON_SPAM; show_prompt = true; break;
        case 3: wifi_evil_portal_attack(); break;
        case 4: attack_type = ATTACK_PACKET_CAPTURE; show_prompt = true; break;
        case 5: wifi_probe_sniffer(); break;
        case 6: wifi_eapol_sniffer(); break;
        case 7: attack_type = ATTACK_WIFI_KARMA; show_prompt = true; break;
        case 8: wifi_rickroll_attack(); break;
        case 9: wifi_pineapple_detector(); break;
        case 10: 
            menu_state.in_submenu = false;
            scroll_offset = 0;
            menu_draw();
            return;
    }
    
    if (show_prompt) {
        show_schedule_prompt(attack_type);
    }
    
    if (index == 1) {
        if (confirm_dialog("WiFi Deauth", "Start attack?")) {
            stats_increment_attacks();
            attack_timer_start(60);
            wifi_deauth_attack();
            attack_timer_stop();
        }
    } else if (index == 2) {
        if (confirm_dialog("Beacon Spam", "Start attack?")) {
            stats_increment_attacks();
            wifi_beacon_spam_enhanced();
        }
    } else if (index == 4) {
        stats_increment_captures();
        wifi_packet_monitor();
    } else if (index == 7) {
        if (confirm_dialog("Karma Attack", "Start attack?")) {
            stats_increment_attacks();
            wifi_karma_attack();
        }
    }
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WiFi function failed: %d", ret);
    }
    
    vTaskDelay(pdMS_TO_TICKS(2000));
    menu_draw();
}

void execute_bt_function(int index) {
    esp_err_t ret = ESP_OK;
    
    switch (index) {
        case 0: 
            stats_increment_scans();
            ble_scan_start(); 
            break;
        case 1:
            oui_spy_flock_detector();
            break;
        case 2: 
            if (confirm_dialog("BLE Attack", "Target device?")) {
                stats_increment_attacks();
                ble_targeted_attack();
            }
            break;
        case 3: 
        case 4: 
        case 5: 
        case 6: 
        case 7: 
        case 8:
            stats_increment_attacks();
            if (index == 3) ble_apple_juice_attack();
            else if (index == 4) ble_sour_apple_attack();
            else if (index == 5) ble_samsung_watch_spam_enhanced();
            else if (index == 6) ble_google_fastpair_spam();
            else if (index == 7) ble_beacon_flood_attack();
            else if (index == 8) ble_jammer_start();
            break;
        case 9: ble_sniffer_start(); break;
        case 10:
            menu_state.in_submenu = false;
            scroll_offset = 0;
            menu_draw();
            return;
    }
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "BT function failed: %d", ret);
    }
    
    vTaskDelay(pdMS_TO_TICKS(2000));
    menu_draw();
}

void execute_rf_sub_function(int index) {
    if (!cc1101_is_connected() && index < 5) {
        display_fill_screen(COLOR_BLACK);
        display_draw_text(10, 10, "CC1101 Not Found", COLOR_RED, COLOR_BLACK);
        display_draw_text(10, 40, "Connect CC1101 module", COLOR_WHITE, COLOR_BLACK);
        display_draw_text(10, 60, "to use SubGHz features", COLOR_WHITE, COLOR_BLACK);
        display_draw_text(10, 280, "Touch to continue", COLOR_GRAY, COLOR_BLACK);
        while (!touchscreen_is_touched()) vTaskDelay(pdMS_TO_TICKS(100));
        menu_draw();
        return;
    }
    
    switch (index) {
        case 0: rf_spectrum_analyzer(); break;
        case 1: subghz_capture_raw(433920, 5000); break;
        case 2: /* Signal replay */ break;
        case 3: /* Garage doors */ break;
        case 4: /* Car keys */ break;
        case 5:
            menu_state.in_submenu = false;
            scroll_offset = 0;
            menu_draw();
            return;
    }
    
    vTaskDelay(pdMS_TO_TICKS(2000));
    menu_draw();
}

void execute_ir_function(int index) {
    esp_err_t ret = ESP_OK;
    
    switch (index) {
        case 0: ir_tv_power_attack(); break;
        case 1: //ir_functions_volume(); break;
        case 2: //ir_functions_channel(); break;
        case 3:
            menu_state.in_submenu = false;
            scroll_offset = 0;
            menu_draw();
            return;
    }
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "IR function failed: %d", ret);
    }
    
    vTaskDelay(pdMS_TO_TICKS(2000));
    menu_draw();
}

void execute_nfc_function(int index) {
    esp_err_t ret = ESP_OK;
    
    switch (index) {
        case 0: nfc_scan_cards(); break;
        case 1: //nfc_clone_card(); break;
        case 2: //nfc_emulate_card(); break;
        case 3:
            menu_state.in_submenu = false;
            scroll_offset = 0;
            menu_draw();
            return;
    }
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NFC function failed: %d", ret);
    }
    
    vTaskDelay(pdMS_TO_TICKS(2000));
    menu_draw();
}

void execute_tools_function(int index) {
    esp_err_t ret = ESP_OK;
    
    switch (index) {
        case 0: target_manager_ui(); break;
        case 1: //packet_logger_ui(); break;
        case 2: //signal_analyzer_ui(); break;
        case 3: sd_browser_ui(); break;
        case 4: usb_msc_mode_ui(); break;
        case 5:
            menu_state.in_submenu = false;
            scroll_offset = 0;
            menu_draw();
            return;
    }
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Tools function failed: %d", ret);
    }
    
    // Only redraw if still in submenu
    if (menu_state.in_submenu) {
        menu_draw();
    }
}

void execute_settings_function(int index) {
    esp_err_t ret = ESP_OK;
    
    switch (index) {
        case 0: //settings_display_ui(); break;
        case 1: //settings_network_ui(); break;
        case 2: //settings_system_ui(); break;
        case 3:
            battery_show_details();
            while (!touchscreen_is_touched()) vTaskDelay(pdMS_TO_TICKS(100));
            break;
        case 4: antenna_toggle_ui(); break;
        case 5: brightness_control_ui(); break;
        case 6:
            menu_state.in_submenu = false;
            scroll_offset = 0;
            menu_draw();
            return;
    }
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Settings function failed: %d", ret);
    }
    
    // Only redraw if still in submenu
    if (menu_state.in_submenu) {
        menu_draw();
    }
}

void menu_handle_input(void) {
    static uint32_t last_touch_time = 0;
    uint32_t current_time = xTaskGetTickCount();
    
    if (current_time - last_touch_time > pdMS_TO_TICKS(200)) {
        touch_point_t point = touchscreen_get_point();
        if (point.pressed) {
            ESP_LOGI(TAG, "Touch: X=%d Y=%d", point.x, point.y);
                if (!menu_state.in_submenu) {
                    // Main menu touch handling
                    int items_per_row = 2;
                    int box_width = (DISPLAY_WIDTH - 30) / items_per_row;
                    int box_height = 40;
                    int start_y = 40;
                    
                    for (int i = 0; i < MENU_COUNT; i++) {
                        int row = i / items_per_row;
                        int col = i % items_per_row;
                        int x = 10 + col * (box_width + 10);
                        int y = start_y + row * (box_height + 10);
                        
                        // Touch detection for all items regardless of visibility
                        if (point.x >= x && point.x <= x + box_width &&
                            point.y >= y && point.y <= y + box_height) {
                            menu_state.current_index = i;
                            
                            if (i == MENU_WIFI || i == MENU_BLUETOOTH || i == MENU_SUBGHZ || 
                                i == MENU_IR_REMOTE || i == MENU_NFC_RFID || i == MENU_TOOLS || i == MENU_SETTING) {
                                menu_state.in_submenu = true;
                                scroll_offset = 0;
                            } else {
                                esp_err_t ret = ESP_OK;
                                switch (i) {
                                    case MENU_GPS: gps_get_location(); break;
                                    case MENU_BADUSB: badusb_execute_payload(); break;
                                    case MENU_AUTOMATION: attack_scheduler_ui(); break;
                                }
                                if (ret != ESP_OK) {
                                    ESP_LOGE(TAG, "Menu function failed: %d", ret);
                                }
                                vTaskDelay(pdMS_TO_TICKS(2000));
                            }
                            menu_draw();
                            break;
                        }
                    }
                } else {
                    // Submenu touch handling
                    int submenu_count;
                    if (menu_state.current_index == MENU_WIFI) submenu_count = 11;
                    else if (menu_state.current_index == MENU_BLUETOOTH) submenu_count = 11;
                    else if (menu_state.current_index == MENU_SUBGHZ) submenu_count = 6;
                    else if (menu_state.current_index == MENU_IR_REMOTE) submenu_count = 4;
                    else if (menu_state.current_index == MENU_NFC_RFID) submenu_count = 4;
                    else if (menu_state.current_index == MENU_TOOLS) submenu_count = 6;
                    else if (menu_state.current_index == MENU_SETTING) submenu_count = 7;
                    else submenu_count = 0;
                    
                    int items_per_page = 9;
                    int total_pages = (submenu_count + items_per_page - 1) / items_per_page;
                    
                    // Check scroll buttons
                    if (point.y >= 255 && point.y <= 275) {
                        if (point.x >= 10 && point.x <= 60 && scroll_offset > 0) {
                            scroll_offset--;
                            menu_draw();
                        } else if (point.x >= 180 && point.x <= 230 && scroll_offset < total_pages - 1) {
                            scroll_offset++;
                            menu_draw();
                        }
                    } else {
                        // Item selection
                        int start_idx = scroll_offset * items_per_page;
                        for (int i = 0; i < items_per_page && (start_idx + i) < submenu_count; i++) {
                            int y = 60 + i * 22;
                            if (point.y >= y && point.y <= y + 18) {
                                int selected_idx = start_idx + i;
                                if (menu_state.current_index == MENU_WIFI) {
                                    execute_wifi_function(selected_idx);
                                } else if (menu_state.current_index == MENU_BLUETOOTH) {
                                    execute_bt_function(selected_idx);
                                } else if (menu_state.current_index == MENU_SUBGHZ) {
                                    execute_rf_sub_function(selected_idx);
                                } else if (menu_state.current_index == MENU_IR_REMOTE) {
                                    execute_ir_function(selected_idx);
                                } else if (menu_state.current_index == MENU_NFC_RFID) {
                                    execute_nfc_function(selected_idx);
                                } else if (menu_state.current_index == MENU_TOOLS) {
                                    execute_tools_function(selected_idx);
                                } else if (menu_state.current_index == MENU_SETTING) {
                                    execute_settings_function(selected_idx);
                                }
                                break;
                            }
                        }
                    }
                }
            last_touch_time = current_time;
        }
    }
}

void menu_navigate_up(void) {
    if (menu_state.current_index > 0) {
        menu_state.current_index--;
        menu_draw();
    }
}

void menu_navigate_down(void) {
    if (menu_state.current_index < MENU_COUNT - 1) {
        menu_state.current_index++;
        menu_draw();
    }
}

void menu_navigate_left(void) {}
void menu_navigate_right(void) {}
void menu_select(void) {}