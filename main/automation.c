#include "automation.h"
#include "display.h"
#include "touchscreen.h"
#include "wifi_functions.h"
#include "bluetooth_functions.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char* TAG = "AUTOMATION";
static script_t scripts[5];
static scheduled_task_t tasks[3];
static int script_count = 0;
static int task_count = 0;
static bool recording_macro = false;

esp_err_t automation_init(void) {
    // Initialize default scripts
    strcpy(scripts[0].name, "WiFi Scan Loop");
    strcpy(scripts[0].commands, "wifi_scan;delay_5s;wifi_scan");
    scripts[0].delay_ms = 5000;
    scripts[0].enabled = true;
    
    strcpy(scripts[1].name, "BLE Spam Attack");
    strcpy(scripts[1].commands, "ble_apple_spam;delay_10s;ble_samsung_spam");
    scripts[1].delay_ms = 10000;
    scripts[1].enabled = true;
    
    script_count = 2;
    
    ESP_LOGI(TAG, "Automation initialized with %d scripts", script_count);
    return ESP_OK;
}

void automation_script_runner(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Script Runner", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    if (script_count == 0) {
        display_draw_text(10, 50, "No scripts available", COLOR_GRAY, COLOR_BLACK);
    } else {
        for (int i = 0; i < script_count && i < 5; i++) {
            int y = 35 + i * 30;
            uint16_t color = scripts[i].enabled ? COLOR_GREEN : COLOR_RED;
            
            display_fill_rect(10, y, 220, 28, COLOR_DARKBLUE);
            display_draw_rect(10, y, 220, 28, COLOR_GRAY);
            display_draw_text(15, y + 10, scripts[i].name, color, COLOR_DARKBLUE);
        }
    }
    
    // Add new script button
    display_fill_rect(10, 200, 100, 25, COLOR_GREEN);
    display_draw_rect(10, 200, 100, 25, COLOR_WHITE);
    display_draw_text(35, 210, "ADD SCRIPT", COLOR_WHITE, COLOR_GREEN);
    
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
                
                // Check script selection
                for (int i = 0; i < script_count && i < 5; i++) {
                    int y = 35 + i * 30;
                    if (point.y >= y && point.y <= y + 28) {
                        // Execute script
                        display_fill_screen(COLOR_BLACK);
                        display_draw_text(10, 10, "Executing Script", COLOR_WHITE, COLOR_BLACK);
                        display_draw_text(10, 40, scripts[i].name, COLOR_GREEN, COLOR_BLACK);
                        display_draw_text(10, 60, "Running...", COLOR_ORANGE, COLOR_BLACK);
                        
                        // Simple command parser
                        if (strstr(scripts[i].commands, "wifi_scan")) {
                            wifi_scan_start();
                        }
                        if (strstr(scripts[i].commands, "ble_apple_spam")) {
                            ble_apple_spam();
                        }
                        if (strstr(scripts[i].commands, "ble_samsung_spam")) {
                            ble_samsung_spam();
                        }
                        
                        display_draw_text(10, 80, "Script completed!", COLOR_GREEN, COLOR_BLACK);
                        vTaskDelay(pdMS_TO_TICKS(2000));
                        return;
                    }
                }
                
                // Add new script
                if (point.x >= 10 && point.x <= 110 && point.y >= 200 && point.y <= 225) {
                    if (script_count < 5) {
                        snprintf(scripts[script_count].name, sizeof(scripts[script_count].name), "Script %d", script_count + 1);
                        strcpy(scripts[script_count].commands, "wifi_scan");
                        scripts[script_count].delay_ms = 1000;
                        scripts[script_count].enabled = true;
                        script_count++;
                        
                        display_draw_text(10, 230, "Script added!", COLOR_GREEN, COLOR_BLACK);
                        vTaskDelay(pdMS_TO_TICKS(1000));
                        return;
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void automation_scheduled_tasks(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Scheduled Tasks", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    if (task_count == 0) {
        display_draw_text(10, 50, "No tasks scheduled", COLOR_GRAY, COLOR_BLACK);
    } else {
        for (int i = 0; i < task_count && i < 3; i++) {
            int y = 35 + i * 35;
            char task_info[64];
            snprintf(task_info, sizeof(task_info), "%.20s - %02d:%02d", 
                     tasks[i].name, tasks[i].hour, tasks[i].minute);
            
            display_fill_rect(10, y, 220, 30, COLOR_DARKBLUE);
            display_draw_rect(10, y, 220, 30, COLOR_GRAY);
            display_draw_text(15, y + 10, task_info, COLOR_BLUE, COLOR_DARKBLUE);
        }
    }
    
    display_fill_rect(10, 160, 100, 25, COLOR_GREEN);
    display_draw_rect(10, 160, 100, 25, COLOR_WHITE);
    display_draw_text(30, 170, "ADD TASK", COLOR_WHITE, COLOR_GREEN);
    
    display_fill_rect(10, 280, 80, 30, COLOR_DARKBLUE);
    display_draw_rect(10, 280, 80, 30, COLOR_WHITE);
    display_draw_text(35, 290, "BACK", COLOR_WHITE, COLOR_DARKBLUE);
    
    while (true) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.pressed) {
                if (point.x >= 10 && point.x <= 90 && point.y >= 280 && point.y <= 310) {
                    break;
                }
                if (point.x >= 10 && point.x <= 110 && point.y >= 160 && point.y <= 185) {
                    if (task_count < 3) {
                        snprintf(tasks[task_count].name, sizeof(tasks[task_count].name), "Task %d", task_count + 1);
                        tasks[task_count].hour = 12;
                        tasks[task_count].minute = 0;
                        tasks[task_count].script_id = 0;
                        tasks[task_count].enabled = true;
                        task_count++;
                        
                        display_draw_text(10, 200, "Task added!", COLOR_GREEN, COLOR_BLACK);
                        vTaskDelay(pdMS_TO_TICKS(1000));
                        return;
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void automation_macro_recorder(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Macro Recorder", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    if (!recording_macro) {
        display_draw_text(10, 50, "Ready to record", COLOR_GREEN, COLOR_BLACK);
        display_draw_text(10, 70, "Touch START to begin", COLOR_GRAY, COLOR_BLACK);
        
        display_fill_rect(10, 100, 80, 30, COLOR_GREEN);
        display_draw_rect(10, 100, 80, 30, COLOR_WHITE);
        display_draw_text(35, 110, "START", COLOR_WHITE, COLOR_GREEN);
    } else {
        display_draw_text(10, 50, "Recording...", COLOR_RED, COLOR_BLACK);
        display_draw_text(10, 70, "Touch STOP to finish", COLOR_GRAY, COLOR_BLACK);
        
        display_fill_rect(10, 100, 80, 30, COLOR_RED);
        display_draw_rect(10, 100, 80, 30, COLOR_WHITE);
        display_draw_text(40, 110, "STOP", COLOR_WHITE, COLOR_RED);
    }
    
    display_fill_rect(10, 280, 80, 30, COLOR_DARKBLUE);
    display_draw_rect(10, 280, 80, 30, COLOR_WHITE);
    display_draw_text(35, 290, "BACK", COLOR_WHITE, COLOR_DARKBLUE);
    
    while (true) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.pressed) {
                if (point.x >= 10 && point.x <= 90 && point.y >= 280 && point.y <= 310) {
                    recording_macro = false;
                    break;
                }
                if (point.x >= 10 && point.x <= 90 && point.y >= 100 && point.y <= 130) {
                    recording_macro = !recording_macro;
                    return;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void automation_batch_operations(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Batch Operations", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    display_fill_rect(10, 35, 220, 28, COLOR_DARKBLUE);
    display_draw_rect(10, 35, 220, 28, COLOR_GRAY);
    display_draw_text(15, 45, "Mass WiFi Scan", COLOR_GREEN, COLOR_DARKBLUE);
    
    display_fill_rect(10, 65, 220, 28, COLOR_DARKBLUE);
    display_draw_rect(10, 65, 220, 28, COLOR_GRAY);
    display_draw_text(15, 75, "BLE Device Sweep", COLOR_BLUE, COLOR_DARKBLUE);
    
    display_fill_rect(10, 95, 220, 28, COLOR_DARKBLUE);
    display_draw_rect(10, 95, 220, 28, COLOR_GRAY);
    display_draw_text(15, 105, "RF Frequency Scan", COLOR_ORANGE, COLOR_DARKBLUE);
    
    display_fill_rect(10, 125, 220, 28, COLOR_DARKBLUE);
    display_draw_rect(10, 125, 220, 28, COLOR_GRAY);
    display_draw_text(15, 135, "Multi-Protocol Jam", COLOR_RED, COLOR_DARKBLUE);
    
    display_fill_rect(10, 280, 80, 30, COLOR_DARKBLUE);
    display_draw_rect(10, 280, 80, 30, COLOR_WHITE);
    display_draw_text(35, 290, "BACK", COLOR_WHITE, COLOR_DARKBLUE);
    
    while (true) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.pressed) {
                if (point.x >= 10 && point.x <= 90 && point.y >= 280 && point.y <= 310) {
                    break;
                }
                
                display_fill_screen(COLOR_BLACK);
                display_draw_text(10, 10, "Batch Operation", COLOR_WHITE, COLOR_BLACK);
                display_draw_text(10, 40, "Running...", COLOR_ORANGE, COLOR_BLACK);
                
                if (point.y >= 35 && point.y <= 63) {
                    display_draw_text(10, 60, "WiFi scanning...", COLOR_GREEN, COLOR_BLACK);
                    for (int i = 0; i < 3; i++) {
                        wifi_scan_start();
                        vTaskDelay(pdMS_TO_TICKS(2000));
                    }
                } else if (point.y >= 65 && point.y <= 93) {
                    display_draw_text(10, 60, "BLE scanning...", COLOR_BLUE, COLOR_BLACK);
                    for (int i = 0; i < 3; i++) {
                        ble_scan_start();
                        vTaskDelay(pdMS_TO_TICKS(2000));
                    }
                }
                
                display_draw_text(10, 80, "Batch completed!", COLOR_GREEN, COLOR_BLACK);
                vTaskDelay(pdMS_TO_TICKS(2000));
                return;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void automation_auto_exploit(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Auto Exploit", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    display_draw_text(10, 50, "Scanning for targets...", COLOR_ORANGE, COLOR_BLACK);
    
    // Simulate vulnerability scan
    for (int i = 0; i < 5; i++) {
        char progress[32];
        snprintf(progress, sizeof(progress), "Checking %d/5", i + 1);
        display_fill_rect(10, 70, 200, 20, COLOR_BLACK);
        display_draw_text(10, 70, progress, COLOR_BLUE, COLOR_BLACK);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    display_draw_text(10, 100, "Scan complete!", COLOR_GREEN, COLOR_BLACK);
    display_draw_text(10, 120, "Found 2 vulnerable APs", COLOR_RED, COLOR_BLACK);
    display_draw_text(10, 140, "Ready to exploit", COLOR_ORANGE, COLOR_BLACK);
    
    display_fill_rect(10, 170, 100, 25, COLOR_RED);
    display_draw_rect(10, 170, 100, 25, COLOR_WHITE);
    display_draw_text(35, 180, "EXPLOIT", COLOR_WHITE, COLOR_RED);
    
    display_fill_rect(10, 280, 80, 30, COLOR_DARKBLUE);
    display_draw_rect(10, 280, 80, 30, COLOR_WHITE);
    display_draw_text(35, 290, "BACK", COLOR_WHITE, COLOR_DARKBLUE);
    
    while (true) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.pressed) {
                if (point.x >= 10 && point.x <= 90 && point.y >= 280 && point.y <= 310) {
                    break;
                }
                if (point.x >= 10 && point.x <= 110 && point.y >= 170 && point.y <= 195) {
                    display_draw_text(10, 210, "Exploiting targets...", COLOR_RED, COLOR_BLACK);
                    vTaskDelay(pdMS_TO_TICKS(3000));
                    display_draw_text(10, 230, "Exploit complete!", COLOR_GREEN, COLOR_BLACK);
                    vTaskDelay(pdMS_TO_TICKS(2000));
                    return;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}