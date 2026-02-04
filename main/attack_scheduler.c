#include "attack_scheduler.h"
#include "wifi_functions.h"
#include "wifi_attacks_enhanced.h"
#include "bluetooth_functions.h"
#include "ble_attacks_enhanced.h"
#include "packet_capture.h"
#include "display.h"
#include "touchscreen.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char* TAG = "SCHEDULER";
static scheduled_attack_t attack_queue[10];
static uint8_t queue_count = 0;
static bool scheduler_running = false;
static TaskHandle_t scheduler_task_handle = NULL;

static const char* attack_names[] = {
    "WiFi Deauth",
    "Beacon Spam",
    "Karma Attack",
    "Apple Spam",
    "Samsung Spam",
    "BLE Jammer",
    "Packet Capture"
};

void attack_scheduler_init(void) {
    memset(attack_queue, 0, sizeof(attack_queue));
    queue_count = 0;
    scheduler_running = false;
    ESP_LOGI(TAG, "Scheduler initialized");
}

bool attack_scheduler_add(attack_type_t type, schedule_mode_t mode, 
                          uint32_t duration, uint32_t interval, uint32_t repeats) {
    if (queue_count >= 10) {
        ESP_LOGW(TAG, "Queue full");
        return false;
    }
    
    attack_queue[queue_count].type = type;
    attack_queue[queue_count].mode = mode;
    attack_queue[queue_count].duration_sec = duration;
    attack_queue[queue_count].interval_sec = interval;
    attack_queue[queue_count].repeat_count = repeats;
    attack_queue[queue_count].active = true;
    attack_queue[queue_count].executions = 0;
    
    queue_count++;
    ESP_LOGI(TAG, "Added attack: %s", attack_names[type]);
    return true;
}

static void execute_attack(attack_type_t type, uint32_t duration) {
    ESP_LOGI(TAG, "Executing: %s for %lus", attack_names[type], duration);
    
    switch (type) {
        case ATTACK_WIFI_DEAUTH:
            wifi_deauth_attack();
            break;
        case ATTACK_WIFI_BEACON_SPAM:
            wifi_beacon_spam_enhanced();
            break;
        case ATTACK_WIFI_KARMA:
            wifi_karma_attack();
            break;
        case ATTACK_BLE_APPLE_SPAM:
            ble_apple_juice_attack();
            break;
        case ATTACK_BLE_SAMSUNG_SPAM:
            ble_samsung_watch_spam_enhanced();
            break;
        case ATTACK_BLE_JAMMER:
            ble_jammer_start();
            break;
        case ATTACK_PACKET_CAPTURE:
            wifi_packet_monitor();
            break;
        default:
            break;
    }
}

static void scheduler_task(void* param) {
    while (scheduler_running) {
        for (int i = 0; i < queue_count; i++) {
            if (!attack_queue[i].active) continue;
            
            scheduled_attack_t* attack = &attack_queue[i];
            
            // Execute attack
            execute_attack(attack->type, attack->duration_sec);
            attack->executions++;
            
            // Check if should continue
            if (attack->mode == SCHED_ONCE) {
                attack->active = false;
            } else if (attack->mode == SCHED_REPEAT) {
                if (attack->executions >= attack->repeat_count) {
                    attack->active = false;
                }
            }
            
            // Wait interval before next attack
            if (attack->active && attack->interval_sec > 0) {
                vTaskDelay(pdMS_TO_TICKS(attack->interval_sec * 1000));
            }
            
            if (!scheduler_running) break;
        }
        
        // Check if all attacks done
        bool any_active = false;
        for (int i = 0; i < queue_count; i++) {
            if (attack_queue[i].active) {
                any_active = true;
                break;
            }
        }
        
        if (!any_active) {
            scheduler_running = false;
            break;
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    ESP_LOGI(TAG, "Scheduler stopped");
    scheduler_task_handle = NULL;
    vTaskDelete(NULL);
}

void attack_scheduler_start(void) {
    if (scheduler_running) {
        ESP_LOGW(TAG, "Already running");
        return;
    }
    
    if (queue_count == 0) {
        ESP_LOGW(TAG, "No attacks queued");
        return;
    }
    
    scheduler_running = true;
    xTaskCreate(scheduler_task, "scheduler", 4096, NULL, 5, &scheduler_task_handle);
    ESP_LOGI(TAG, "Scheduler started");
}

void attack_scheduler_stop(void) {
    scheduler_running = false;
    if (scheduler_task_handle) {
        vTaskDelete(scheduler_task_handle);
        scheduler_task_handle = NULL;
    }
    ESP_LOGI(TAG, "Scheduler stopped");
}

bool attack_scheduler_is_running(void) {
    return scheduler_running;
}

uint8_t attack_scheduler_get_queue_count(void) {
    return queue_count;
}

void attack_scheduler_clear(void) {
    attack_scheduler_stop();
    memset(attack_queue, 0, sizeof(attack_queue));
    queue_count = 0;
    ESP_LOGI(TAG, "Queue cleared");
}

void attack_scheduler_ui(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Attack Scheduler", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    if (queue_count == 0) {
        display_draw_text(10, 40, "No attacks queued", COLOR_GRAY, COLOR_BLACK);
        display_draw_text(10, 60, "Add attacks first", COLOR_ORANGE, COLOR_BLACK);
    } else {
        display_draw_text(10, 40, "Queued Attacks:", COLOR_GREEN, COLOR_BLACK);
        
        for (int i = 0; i < queue_count && i < 6; i++) {
            char info[48];
            snprintf(info, sizeof(info), "%d: %s", i + 1, attack_names[attack_queue[i].type]);
            display_draw_text(10, 60 + i * 25, info, COLOR_WHITE, COLOR_BLACK);
            
            char details[48];
            if (attack_queue[i].mode == SCHED_ONCE) {
                snprintf(details, sizeof(details), "   Once, %lus", attack_queue[i].duration_sec);
            } else if (attack_queue[i].mode == SCHED_REPEAT) {
                snprintf(details, sizeof(details), "   %lux, %lus", 
                        attack_queue[i].repeat_count, attack_queue[i].duration_sec);
            } else {
                snprintf(details, sizeof(details), "   Every %lus", attack_queue[i].interval_sec);
            }
            display_draw_text(10, 60 + i * 25 + 12, details, COLOR_GRAY, COLOR_BLACK);
        }
    }
    
    // Buttons
    if (queue_count > 0 && !scheduler_running) {
        display_fill_rect(10, 240, 70, 25, COLOR_GREEN);
        display_draw_text(25, 248, "START", COLOR_WHITE, COLOR_GREEN);
    }
    
    if (queue_count > 0) {
        display_fill_rect(90, 240, 70, 25, COLOR_RED);
        display_draw_text(105, 248, "CLEAR", COLOR_WHITE, COLOR_RED);
    }
    
    display_fill_rect(170, 240, 60, 25, COLOR_BLUE);
    display_draw_text(185, 248, "BACK", COLOR_WHITE, COLOR_BLUE);
    
    // Handle input
    while (true) {
        touch_point_t point = touchscreen_get_point();
        if (point.pressed && point.y >= 240 && point.y <= 265) {
            if (point.x >= 10 && point.x <= 80 && queue_count > 0 && !scheduler_running) {
                attack_scheduler_start();
                
                // Show running status
                display_fill_screen(COLOR_BLACK);
                display_draw_text(10, 10, "Scheduler Running", COLOR_GREEN, COLOR_BLACK);
                display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
                display_draw_text(10, 40, "Executing attacks...", COLOR_ORANGE, COLOR_BLACK);
                
                // Monitor progress
                while (scheduler_running) {
                    char status[32];
                    snprintf(status, sizeof(status), "Queue: %d attacks", queue_count);
                    display_fill_rect(10, 60, 220, 20, COLOR_BLACK);
                    display_draw_text(10, 60, status, COLOR_WHITE, COLOR_BLACK);
                    
                    display_fill_rect(60, 240, 120, 25, COLOR_RED);
                    display_draw_text(90, 248, "STOP", COLOR_WHITE, COLOR_RED);
                    
                    touch_point_t p = touchscreen_get_point();
                    if (p.pressed && p.y >= 240 && p.y <= 265) {
                        attack_scheduler_stop();
                        break;
                    }
                    
                    vTaskDelay(pdMS_TO_TICKS(500));
                }
                
                display_draw_text(10, 100, "Scheduler stopped", COLOR_GREEN, COLOR_BLACK);
                vTaskDelay(pdMS_TO_TICKS(2000));
                return;
                
            } else if (point.x >= 90 && point.x <= 160 && queue_count > 0) {
                attack_scheduler_clear();
                return;
            } else if (point.x >= 170 && point.x <= 230) {
                return;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
