#include "ducky_script.h"
#include "display.h"
#include "touchscreen.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char* TAG = "DUCKY";

esp_err_t ducky_script_init(void) {
    ESP_LOGI(TAG, "Ducky Script interpreter initialized");
    return ESP_OK;
}

void ducky_execute_command(const char* command, const char* parameter) {
    if (strcmp(command, "STRING") == 0) {
        // Type string
        ESP_LOGI(TAG, "Typing: %s", parameter);
    } else if (strcmp(command, "DELAY") == 0) {
        int delay_ms = atoi(parameter);
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    } else if (strcmp(command, "ENTER") == 0) {
        ESP_LOGI(TAG, "Pressing ENTER");
    } else if (strcmp(command, "CTRL") == 0) {
        ESP_LOGI(TAG, "Pressing CTRL+%s", parameter);
    } else if (strcmp(command, "ALT") == 0) {
        ESP_LOGI(TAG, "Pressing ALT+%s", parameter);
    } else if (strcmp(command, "GUI") == 0) {
        ESP_LOGI(TAG, "Pressing WIN+%s", parameter);
    } else if (strcmp(command, "TAB") == 0) {
        ESP_LOGI(TAG, "Pressing TAB");
    } else if (strcmp(command, "ESCAPE") == 0) {
        ESP_LOGI(TAG, "Pressing ESCAPE");
    }
}

void ducky_script_interpreter(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Ducky Script Engine", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    display_fill_rect(10, 40, 220, 25, COLOR_DARKBLUE);
    display_draw_rect(10, 40, 220, 25, COLOR_GRAY);
    display_draw_text(15, 50, "Run from SD Card", COLOR_GREEN, COLOR_DARKBLUE);
    
    display_fill_rect(10, 70, 220, 25, COLOR_DARKBLUE);
    display_draw_rect(10, 70, 220, 25, COLOR_GRAY);
    display_draw_text(15, 80, "Built-in Scripts", COLOR_BLUE, COLOR_DARKBLUE);
    
    display_fill_rect(10, 100, 220, 25, COLOR_DARKBLUE);
    display_draw_rect(10, 100, 220, 25, COLOR_GRAY);
    display_draw_text(15, 110, "Script Editor", COLOR_ORANGE, COLOR_DARKBLUE);
    
    display_fill_rect(10, 130, 220, 25, COLOR_DARKBLUE);
    display_draw_rect(10, 130, 220, 25, COLOR_GRAY);
    display_draw_text(15, 140, "Test Script", COLOR_PURPLE, COLOR_DARKBLUE);
    
    display_fill_rect(10, 280, 80, 30, COLOR_DARKBLUE);
    display_draw_rect(10, 280, 80, 30, COLOR_WHITE);
    display_draw_text(35, 290, "BACK", COLOR_WHITE, COLOR_DARKBLUE);
    
    while (true) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.pressed) {
                if (point.y >= 40 && point.y <= 65) {
                    ducky_run_script_from_sd("payload.txt");
                    return;
                } else if (point.y >= 70 && point.y <= 95) {
                    // Built-in scripts menu
                    display_fill_screen(COLOR_BLACK);
                    display_draw_text(10, 10, "Built-in Scripts", COLOR_WHITE, COLOR_BLACK);
                    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
                    
                    display_draw_text(10, 40, "1. WiFi Password Grab", COLOR_GREEN, COLOR_BLACK);
                    display_draw_text(10, 60, "2. Reverse Shell", COLOR_RED, COLOR_BLACK);
                    display_draw_text(10, 80, "3. System Info Dump", COLOR_BLUE, COLOR_BLACK);
                    display_draw_text(10, 100, "4. Disable Defender", COLOR_ORANGE, COLOR_BLACK);
                    display_draw_text(10, 120, "5. RickRoll Prank", COLOR_PURPLE, COLOR_BLACK);
                    
                    vTaskDelay(pdMS_TO_TICKS(3000));
                    return;
                } else if (point.y >= 100 && point.y <= 125) {
                    ducky_script_editor();
                    return;
                } else if (point.y >= 130 && point.y <= 155) {
                    // Test script
                    display_fill_screen(COLOR_BLACK);
                    display_draw_text(10, 10, "Test Script Running", COLOR_WHITE, COLOR_BLACK);
                    display_draw_text(10, 40, "Executing commands:", COLOR_GREEN, COLOR_BLACK);
                    
                    display_draw_text(10, 60, "GUI r", COLOR_BLUE, COLOR_BLACK);
                    ducky_execute_command("GUI", "r");
                    vTaskDelay(pdMS_TO_TICKS(500));
                    
                    display_draw_text(10, 80, "STRING notepad", COLOR_BLUE, COLOR_BLACK);
                    ducky_execute_command("STRING", "notepad");
                    vTaskDelay(pdMS_TO_TICKS(500));
                    
                    display_draw_text(10, 100, "ENTER", COLOR_BLUE, COLOR_BLACK);
                    ducky_execute_command("ENTER", "");
                    vTaskDelay(pdMS_TO_TICKS(1000));
                    
                    display_draw_text(10, 120, "STRING Hello World!", COLOR_BLUE, COLOR_BLACK);
                    ducky_execute_command("STRING", "Hello from NetRaze32!");
                    
                    display_draw_text(10, 160, "Test complete!", COLOR_GREEN, COLOR_BLACK);
                    vTaskDelay(pdMS_TO_TICKS(2000));
                    return;
                } else if (point.x >= 10 && point.x <= 90 && point.y >= 280 && point.y <= 310) {
                    break;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void ducky_run_script_from_sd(const char* filename) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Running Script", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    char filepath[64];
    snprintf(filepath, sizeof(filepath), "File: %s", filename);
    display_draw_text(10, 40, filepath, COLOR_BLUE, COLOR_BLACK);
    
    display_draw_text(10, 70, "Loading from SD card...", COLOR_ORANGE, COLOR_BLACK);
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Simulate script execution
    char* sample_commands[] = {
        "DELAY 1000",
        "GUI r", 
        "DELAY 500",
        "STRING cmd",
        "ENTER",
        "DELAY 1000",
        "STRING echo NetRaze32",
        "ENTER"
    };
    
    for (int i = 0; i < 8; i++) {
        char line_info[64];
        snprintf(line_info, sizeof(line_info), "Line %d: %s", i + 1, sample_commands[i]);
        display_draw_text(10, 90 + i * 15, line_info, COLOR_GREEN, COLOR_BLACK);
        
        // Parse and execute command
        char* space = strchr(sample_commands[i], ' ');
        if (space) {
            *space = '\0';
            ducky_execute_command(sample_commands[i], space + 1);
            *space = ' ';
        } else {
            ducky_execute_command(sample_commands[i], "");
        }
        
        vTaskDelay(pdMS_TO_TICKS(300));
    }
    
    display_draw_text(10, 250, "Script execution complete!", COLOR_GREEN, COLOR_BLACK);
    display_draw_text(10, 280, "Touch to continue", COLOR_GRAY, COLOR_BLACK);
    
    while (true) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.pressed) break;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void ducky_script_editor(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Script Editor", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    display_draw_text(10, 40, "Basic script editor", COLOR_BLUE, COLOR_BLACK);
    display_draw_text(10, 60, "Commands supported:", COLOR_GREEN, COLOR_BLACK);
    
    display_draw_text(10, 80, "STRING <text>", COLOR_GRAY, COLOR_BLACK);
    display_draw_text(10, 95, "DELAY <ms>", COLOR_GRAY, COLOR_BLACK);
    display_draw_text(10, 110, "ENTER, TAB, ESCAPE", COLOR_GRAY, COLOR_BLACK);
    display_draw_text(10, 125, "CTRL <key>", COLOR_GRAY, COLOR_BLACK);
    display_draw_text(10, 140, "ALT <key>", COLOR_GRAY, COLOR_BLACK);
    display_draw_text(10, 155, "GUI <key>", COLOR_GRAY, COLOR_BLACK);
    
    display_draw_text(10, 180, "Save to: /sdcard/script.txt", COLOR_ORANGE, COLOR_BLACK);
    
    display_draw_text(10, 220, "Editor functionality", COLOR_BLUE, COLOR_BLACK);
    display_draw_text(10, 240, "coming in next update", COLOR_BLUE, COLOR_BLACK);
    
    display_draw_text(10, 280, "Touch to continue", COLOR_GRAY, COLOR_BLACK);
    
    while (true) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.pressed) break;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}