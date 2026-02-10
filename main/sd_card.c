#include "sd_card.h"
#include "board_config.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "esp_log.h"
#include "display.h"
#include "touchscreen.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>

static const char* TAG = "SD_CARD";
static sdmmc_card_t* card = NULL;
static bool mounted = false;

esp_err_t sd_card_init(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "SD Card", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 40, "Not Available", COLOR_RED, COLOR_BLACK);
    display_draw_text(10, 70, "Pin conflict with", COLOR_ORANGE, COLOR_BLACK);
    display_draw_text(10, 90, "display on this board", COLOR_ORANGE, COLOR_BLACK);
    display_draw_text(10, 120, "Use internal flash", COLOR_BLUE, COLOR_BLACK);
    display_draw_text(10, 140, "for logging instead", COLOR_BLUE, COLOR_BLACK);
    display_draw_text(10, 280, "Touch to continue", COLOR_GRAY, COLOR_BLACK);
    
    ESP_LOGW(TAG, "SD card disabled - pin conflict with display");
    
    while (true) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.pressed) break;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    return ESP_ERR_NOT_SUPPORTED;
}

void sd_card_deinit(void) {
    ESP_LOGI(TAG, "SD card not available");
}

bool sd_card_is_mounted(void) {
    return false;
}
