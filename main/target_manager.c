#include "target_manager.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "display.h"
#include "touchscreen.h"
#include "esp_log.h"
#include <string.h>
#include <stdio.h>
#include <time.h>

static const char* TAG = "TARGET_MGR";
static const char* NVS_NAMESPACE = "targets";
static saved_target_t targets[MAX_SAVED_TARGETS];
static uint8_t target_count = 0;

void target_manager_init(void) {
    memset(targets, 0, sizeof(targets));
    target_count = 0;
    target_manager_load();
    ESP_LOGI(TAG, "Target manager initialized");
}

bool target_manager_save(target_type_t type, const char* name, const uint8_t* mac, 
                         int8_t rssi, uint8_t channel) {
    if (target_count >= MAX_SAVED_TARGETS) {
        ESP_LOGW(TAG, "Target list full");
        return false;
    }
    
    // Check for duplicates
    for (int i = 0; i < target_count; i++) {
        if (memcmp(targets[i].mac, mac, 6) == 0) {
            ESP_LOGW(TAG, "Target already saved");
            return false;
        }
    }
    
    targets[target_count].type = type;
    strncpy(targets[target_count].name, name, 31);
    targets[target_count].name[31] = '\0';
    memcpy(targets[target_count].mac, mac, 6);
    targets[target_count].rssi = rssi;
    targets[target_count].channel = channel;
    targets[target_count].favorite = false;
    targets[target_count].last_seen = time(NULL);
    
    target_count++;
    target_manager_save_to_file();
    
    ESP_LOGI(TAG, "Saved target: %s", name);
    return true;
}

bool target_manager_delete(uint8_t index) {
    if (index >= target_count) return false;
    
    for (int i = index; i < target_count - 1; i++) {
        targets[i] = targets[i + 1];
    }
    target_count--;
    target_manager_save_to_file();
    
    ESP_LOGI(TAG, "Deleted target at index %d", index);
    return true;
}

saved_target_t* target_manager_get(uint8_t index) {
    if (index >= target_count) return NULL;
    return &targets[index];
}

uint8_t target_manager_count(void) {
    return target_count;
}

void target_manager_toggle_favorite(uint8_t index) {
    if (index < target_count) {
        targets[index].favorite = !targets[index].favorite;
        target_manager_save_to_file();
    }
}

bool target_manager_load(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "No saved targets");
        return false;
    }
    
    nvs_get_u8(nvs_handle, "count", &target_count);
    size_t required_size = sizeof(saved_target_t) * target_count;
    nvs_get_blob(nvs_handle, "data", targets, &required_size);
    nvs_close(nvs_handle);
    
    ESP_LOGI(TAG, "Loaded %d targets from NVS", target_count);
    return true;
}

bool target_manager_save_to_file(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS");
        return false;
    }
    
    nvs_set_u8(nvs_handle, "count", target_count);
    nvs_set_blob(nvs_handle, "data", targets, sizeof(saved_target_t) * target_count);
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    
    ESP_LOGI(TAG, "Saved %d targets to NVS", target_count);
    return true;
}

void target_manager_quick_save_wifi(wifi_ap_record_t* ap) {
    target_manager_save(TARGET_WIFI, (char*)ap->ssid, ap->bssid, 
                       ap->rssi, ap->primary);
}

void target_manager_quick_save_ble(const uint8_t* mac, const char* name, int8_t rssi) {
    target_manager_save(TARGET_BLE, name, mac, rssi, 0);
}

void target_manager_ui(void) {
    int scroll_offset = 0;
    bool running = true;
    
    while (running) {
        display_fill_screen(COLOR_BLACK);
        display_draw_text(10, 10, "Saved Targets", COLOR_WHITE, COLOR_BLACK);
        display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
        
        if (target_count == 0) {
            display_draw_text(10, 40, "No saved targets", COLOR_GRAY, COLOR_BLACK);
            display_draw_text(10, 60, "Save from scan results", COLOR_ORANGE, COLOR_BLACK);
        } else {
            char count_str[32];
            snprintf(count_str, sizeof(count_str), "Total: %d targets", target_count);
            display_draw_text(10, 35, count_str, COLOR_GREEN, COLOR_BLACK);
            
            // Show targets
            int visible = 6;
            for (int i = 0; i < visible && (i + scroll_offset) < target_count; i++) {
                int idx = i + scroll_offset;
                saved_target_t* target = &targets[idx];
                
                int y = 55 + i * 28;
                
                // Draw favorite star
                if (target->favorite) {
                    display_draw_text(10, y, "*", COLOR_ORANGE, COLOR_BLACK);
                }
                
                // Draw name
                char name[32];
                snprintf(name, sizeof(name), "%.18s", target->name);
                display_draw_text(20, y, name, COLOR_WHITE, COLOR_BLACK);
                
                // Draw type and MAC
                char info[32];
                snprintf(info, sizeof(info), "%s %02X:%02X:%02X",
                        target->type == TARGET_WIFI ? "WiFi" : "BLE",
                        target->mac[0], target->mac[1], target->mac[2]);
                display_draw_text(20, y + 12, info, COLOR_GRAY, COLOR_BLACK);
            }
            
            // Scroll indicators
            if (scroll_offset > 0) {
                display_draw_text(220, 55, "^", COLOR_BLUE, COLOR_BLACK);
            }
            if (scroll_offset + visible < target_count) {
                display_draw_text(220, 210, "v", COLOR_BLUE, COLOR_BLACK);
            }
        }
        
        // Buttons
        if (target_count > 0) {
            display_fill_rect(10, 240, 70, 25, COLOR_RED);
            display_draw_text(20, 248, "DELETE", COLOR_WHITE, COLOR_RED);
            
            display_fill_rect(90, 240, 70, 25, COLOR_ORANGE);
            display_draw_text(105, 248, "CLEAR", COLOR_WHITE, COLOR_ORANGE);
        }
        
        display_fill_rect(170, 240, 60, 25, COLOR_BLUE);
        display_draw_text(185, 248, "BACK", COLOR_WHITE, COLOR_BLUE);
        
        // Handle input
        touch_point_t point = touchscreen_get_point();
        if (point.pressed) {
            // Scroll
            if (point.y >= 55 && point.y < 220) {
                if (point.x >= 220) {
                    if (point.y < 130 && scroll_offset > 0) {
                        scroll_offset--;
                    } else if (point.y >= 130 && scroll_offset + 6 < target_count) {
                        scroll_offset++;
                    }
                } else {
                    // Select target
                    int selected = (point.y - 55) / 28 + scroll_offset;
                    if (selected < target_count) {
                        target_manager_toggle_favorite(selected);
                    }
                }
            }
            
            // Buttons
            if (point.y >= 240 && point.y <= 265) {
                if (point.x >= 10 && point.x <= 80 && target_count > 0) {
                    // Delete first non-favorite
                    for (int i = 0; i < target_count; i++) {
                        if (!targets[i].favorite) {
                            target_manager_delete(i);
                            break;
                        }
                    }
                } else if (point.x >= 90 && point.x <= 160 && target_count > 0) {
                    // Clear all
                    target_count = 0;
                    target_manager_save_to_file();
                } else if (point.x >= 170 && point.x <= 230) {
                    running = false;
                }
            }
            
            vTaskDelay(pdMS_TO_TICKS(300));
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
