#include "tactile_browser.h"
#include "display.h"
#include "touchscreen.h"
#include "esp_http_client.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char* TAG = "BROWSER";
static bookmark_t bookmarks[3];
static history_entry_t history[5];
static int bookmark_count = 0;
static int history_count = 0;
static char current_url[64] = "example.com";

esp_err_t browser_init(void) {
    // Initialize default bookmarks
    strcpy(bookmarks[0].url, "google.com");
    strcpy(bookmarks[0].title, "Google");
    bookmark_count = 1;
    
    ESP_LOGI(TAG, "TactileBrowser initialized");
    return ESP_OK;
}

void browser_start(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "TactileBrowser", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    // Address bar
    display_fill_rect(10, 35, 220, 25, COLOR_DARKGRAY);
    display_draw_rect(10, 35, 220, 25, COLOR_WHITE);
    display_draw_text(15, 45, current_url, COLOR_WHITE, COLOR_DARKGRAY);
    
    // Navigation buttons
    display_fill_rect(10, 70, 50, 25, COLOR_DARKBLUE);
    display_draw_rect(10, 70, 50, 25, COLOR_WHITE);
    display_draw_text(25, 80, "GO", COLOR_WHITE, COLOR_DARKBLUE);
    
    display_fill_rect(70, 70, 50, 25, COLOR_DARKBLUE);
    display_draw_rect(70, 70, 50, 25, COLOR_WHITE);
    display_draw_text(85, 80, "BACK", COLOR_WHITE, COLOR_DARKBLUE);
    
    // Content area
    display_fill_rect(10, 105, 220, 150, COLOR_DARKGRAY);
    display_draw_rect(10, 105, 220, 150, COLOR_WHITE);
    display_draw_text(15, 115, "Loading page...", COLOR_WHITE, COLOR_DARKGRAY);
    display_draw_text(15, 135, "WiFi required", COLOR_ORANGE, COLOR_DARKGRAY);
    
    // Back button
    display_fill_rect(10, 280, 80, 30, COLOR_DARKBLUE);
    display_draw_rect(10, 280, 80, 30, COLOR_WHITE);
    display_draw_text(35, 290, "BACK", COLOR_WHITE, COLOR_DARKBLUE);
    
    // Wait for touch
    while (true) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.pressed) {
                if (point.x >= 10 && point.x <= 90 && point.y >= 280 && point.y <= 310) {
                    break; // Back button
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void browser_navigate(const char* url) {
    strncpy(current_url, url, sizeof(current_url) - 1);
    current_url[sizeof(current_url) - 1] = '\0';
    
    // Add to history
    if (history_count < 5) {
        strncpy(history[history_count].url, url, sizeof(history[history_count].url) - 1);
        strncpy(history[history_count].title, url, sizeof(history[history_count].title) - 1);
        history[history_count].timestamp = xTaskGetTickCount();
        history_count++;
    }
}

void browser_add_bookmark(const char* url, const char* title) {
    if (bookmark_count < 3) {
        strncpy(bookmarks[bookmark_count].url, url, sizeof(bookmarks[bookmark_count].url) - 1);
        strncpy(bookmarks[bookmark_count].title, title, sizeof(bookmarks[bookmark_count].title) - 1);
        bookmark_count++;
    }
}

void browser_show_bookmarks(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Bookmarks", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    if (bookmark_count == 0) {
        display_draw_text(10, 50, "No bookmarks", COLOR_GRAY, COLOR_BLACK);
    } else {
        for (int i = 0; i < bookmark_count && i < 8; i++) {
            int y = 35 + i * 25;
            display_fill_rect(10, y, 220, 23, COLOR_DARKBLUE);
            display_draw_rect(10, y, 220, 23, COLOR_GRAY);
            display_draw_text(15, y + 8, bookmarks[i].title, COLOR_WHITE, COLOR_DARKBLUE);
        }
    }
    
    display_fill_rect(10, 280, 80, 30, COLOR_DARKBLUE);
    display_draw_rect(10, 280, 80, 30, COLOR_WHITE);
    display_draw_text(35, 290, "BACK", COLOR_WHITE, COLOR_DARKBLUE);
    
    // Wait for touch
    while (true) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.pressed) {
                if (point.x >= 10 && point.x <= 90 && point.y >= 280 && point.y <= 310) {
                    break;
                }
                // Check bookmark selection
                for (int i = 0; i < bookmark_count && i < 8; i++) {
                    int y = 35 + i * 25;
                    if (point.y >= y && point.y <= y + 23) {
                        browser_navigate(bookmarks[i].url);
                        browser_start();
                        return;
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void browser_show_history(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "History", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    if (history_count == 0) {
        display_draw_text(10, 50, "No history", COLOR_GRAY, COLOR_BLACK);
    } else {
        for (int i = 0; i < history_count && i < 8; i++) {
            int y = 35 + i * 25;
            display_fill_rect(10, y, 220, 23, COLOR_DARKBLUE);
            display_draw_rect(10, y, 220, 23, COLOR_GRAY);
            display_draw_text(15, y + 8, history[i].title, COLOR_WHITE, COLOR_DARKBLUE);
        }
    }
    
    display_fill_rect(10, 280, 80, 30, COLOR_DARKBLUE);
    display_draw_rect(10, 280, 80, 30, COLOR_WHITE);
    display_draw_text(35, 290, "BACK", COLOR_WHITE, COLOR_DARKBLUE);
    
    // Wait for touch
    while (true) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.pressed) {
                if (point.x >= 10 && point.x <= 90 && point.y >= 280 && point.y <= 310) {
                    break;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void browser_settings(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Browser Settings", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    display_fill_rect(10, 35, 220, 23, COLOR_DARKBLUE);
    display_draw_rect(10, 35, 220, 23, COLOR_GRAY);
    display_draw_text(15, 43, "Clear History", COLOR_RED, COLOR_DARKBLUE);
    
    display_fill_rect(10, 60, 220, 23, COLOR_DARKBLUE);
    display_draw_rect(10, 60, 220, 23, COLOR_GRAY);
    display_draw_text(15, 68, "Clear Bookmarks", COLOR_RED, COLOR_DARKBLUE);
    
    display_fill_rect(10, 85, 220, 23, COLOR_DARKBLUE);
    display_draw_rect(10, 85, 220, 23, COLOR_GRAY);
    display_draw_text(15, 93, "User Agent", COLOR_BLUE, COLOR_DARKBLUE);
    
    display_fill_rect(10, 280, 80, 30, COLOR_DARKBLUE);
    display_draw_rect(10, 280, 80, 30, COLOR_WHITE);
    display_draw_text(35, 290, "BACK", COLOR_WHITE, COLOR_DARKBLUE);
    
    // Wait for touch
    while (true) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.pressed) {
                if (point.x >= 10 && point.x <= 90 && point.y >= 280 && point.y <= 310) {
                    break;
                }
                if (point.y >= 35 && point.y <= 58) {
                    // Clear history
                    history_count = 0;
                    display_draw_text(15, 120, "History cleared", COLOR_GREEN, COLOR_BLACK);
                    vTaskDelay(pdMS_TO_TICKS(1000));
                } else if (point.y >= 60 && point.y <= 83) {
                    // Clear bookmarks
                    bookmark_count = 0;
                    display_draw_text(15, 120, "Bookmarks cleared", COLOR_GREEN, COLOR_BLACK);
                    vTaskDelay(pdMS_TO_TICKS(1000));
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}