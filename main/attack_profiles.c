#include "attack_profiles.h"
#include "esp_log.h"
#include "display.h"
#include "touchscreen.h"
#include "packet_logger.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static const char* TAG = "PROFILES";

typedef struct {
    char name[32];
    uint8_t attack_type; // 0=WiFi, 1=BLE, 2=Mixed
    uint32_t duration;
    uint16_t channel;
    char target_ssid[32];
    bool enabled;
} attack_profile_t;

static attack_profile_t profiles[8];
static int profile_count = 0;

bool attack_profiles_init(void) {
    struct stat st = {0};
    if (stat("/sdcard/profiles", &st) == -1) {
        if (mkdir("/sdcard/profiles", 0700) != 0) {
            ESP_LOGE(TAG, "Failed to create profiles directory");
            return false;
        }
    }
    
    // Load existing profiles
    FILE* file = fopen("/sdcard/profiles/attacks.dat", "rb");
    if (file) {
        fread(&profile_count, sizeof(int), 1, file);
        if (profile_count > 8) profile_count = 8;
        fread(profiles, sizeof(attack_profile_t), profile_count, file);
        fclose(file);
    } else {
        // Create default profiles
        strcpy(profiles[0].name, "Quick WiFi Scan");
        profiles[0].attack_type = 0;
        profiles[0].duration = 30;
        profiles[0].channel = 0;
        profiles[0].enabled = true;
        
        strcpy(profiles[1].name, "BLE Discovery");
        profiles[1].attack_type = 1;
        profiles[1].duration = 60;
        profiles[1].channel = 0;
        profiles[1].enabled = true;
        
        profile_count = 2;
        attack_profiles_save();
    }
    
    ESP_LOGI(TAG, "Loaded %d attack profiles", profile_count);
    return true;
}

void attack_profiles_save(void) {
    FILE* file = fopen("/sdcard/profiles/attacks.dat", "wb");
    if (file) {
        fwrite(&profile_count, sizeof(int), 1, file);
        fwrite(profiles, sizeof(attack_profile_t), profile_count, file);
        fclose(file);
    }
}

void attack_profiles_menu(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Attack Profiles", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    int selected = 0;
    bool need_redraw = true;
    
    while (true) {
        if (need_redraw) {
            display_fill_rect(10, 35, 220, 200, COLOR_BLACK);
            
            // Draw profiles
            for (int i = 0; i < profile_count && i < 6; i++) {
                int y = 40 + i * 30;
                uint16_t color = (i == selected) ? COLOR_ORANGE : COLOR_GRAY;
                
                display_fill_rect(10, y, 200, 25, COLOR_DARKBLUE);
                display_draw_rect(10, y, 200, 25, color);
                
                char profile_info[48];
                snprintf(profile_info, sizeof(profile_info), "%.20s (%ds)", 
                        profiles[i].name, (int)profiles[i].duration);
                display_draw_text(15, y + 8, profile_info, color, COLOR_DARKBLUE);
            }
            
            // Buttons
            display_fill_rect(10, 250, 60, 25, COLOR_GREEN);
            display_draw_rect(10, 250, 60, 25, COLOR_WHITE);
            display_draw_text(25, 258, "RUN", COLOR_WHITE, COLOR_GREEN);
            
            display_fill_rect(80, 250, 60, 25, COLOR_BLUE);
            display_draw_rect(80, 250, 60, 25, COLOR_WHITE);
            display_draw_text(95, 258, "EDIT", COLOR_WHITE, COLOR_BLUE);
            
            display_fill_rect(150, 250, 60, 25, COLOR_RED);
            display_draw_rect(150, 250, 60, 25, COLOR_WHITE);
            display_draw_text(165, 258, "BACK", COLOR_WHITE, COLOR_RED);
            
            need_redraw = false;
        }
        
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            
            // Profile selection
            if (point.x >= 10 && point.x <= 210 && point.y >= 40 && point.y <= 220) {
                int new_selected = (point.y - 40) / 30;
                if (new_selected < profile_count && new_selected != selected) {
                    selected = new_selected;
                    need_redraw = true;
                }
            }
            // Run button
            else if (point.x >= 10 && point.x <= 70 && point.y >= 250 && point.y <= 275) {
                attack_profiles_run(selected);
                need_redraw = true;
            }
            // Edit button
            else if (point.x >= 80 && point.x <= 140 && point.y >= 250 && point.y <= 275) {
                attack_profiles_edit(selected);
                need_redraw = true;
            }
            // Back button
            else if (point.x >= 150 && point.x <= 210 && point.y >= 250 && point.y <= 275) {
                return;
            }
            
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void attack_profiles_run(int profile_id) {
    if (profile_id >= profile_count) return;
    
    attack_profile_t *profile = &profiles[profile_id];
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Running Profile", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    char profile_info[48];
    snprintf(profile_info, sizeof(profile_info), "Profile: %.20s", profile->name);
    display_draw_text(10, 40, profile_info, COLOR_GREEN, COLOR_BLACK);
    
    // Execute based on attack type
    for (int sec = 0; sec < profile->duration; sec++) {
        char progress[32];
        snprintf(progress, sizeof(progress), "Time: %d/%ds", sec + 1, (int)profile->duration);
        display_fill_rect(10, 60, 200, 15, COLOR_BLACK);
        display_draw_text(10, 60, progress, COLOR_BLUE, COLOR_BLACK);
        
        if (profile->attack_type == 0) {
            display_draw_text(10, 80, "WiFi Attack Active", COLOR_RED, COLOR_BLACK);
        } else if (profile->attack_type == 1) {
            display_draw_text(10, 80, "BLE Attack Active", COLOR_RED, COLOR_BLACK);
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    packet_log_custom("PROFILE_RUN", profile->name);
    
    display_draw_text(10, 120, "Profile completed!", COLOR_GREEN, COLOR_BLACK);
    display_draw_text(10, 280, "Touch to continue", COLOR_GRAY, COLOR_BLACK);
    
    while (!touchscreen_is_touched()) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void attack_profiles_edit(int profile_id) {
    if (profile_id >= profile_count) return;
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Edit Profile", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    display_draw_text(10, 40, "Profile editing not", COLOR_GRAY, COLOR_BLACK);
    display_draw_text(10, 60, "implemented yet", COLOR_GRAY, COLOR_BLACK);
    display_draw_text(10, 280, "Touch to return", COLOR_GRAY, COLOR_BLACK);
    
    while (!touchscreen_is_touched()) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}