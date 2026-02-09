#include "sd_card.h"
#include "board_config.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdspi_host.h"
#include "esp_log.h"
#include "display.h"
#include "touchscreen.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

static const char* TAG = "SD_CARD";
static sdmmc_card_t* card = NULL;
static bool mounted = false;

esp_err_t sd_card_init(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "SD Card Init", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 40, "Mounting...", COLOR_ORANGE, COLOR_BLACK);
    
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI2_HOST;
    host.command_timeout_ms = 15000;
    
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = SD_CS_PIN;
    slot_config.host_id = host.slot;
    
    esp_err_t ret = esp_vfs_fat_sdspi_mount("/sdcard", &host, &slot_config, &mount_config, &card);
    
    if (ret == ESP_OK) {
        mounted = true;
        display_draw_text(10, 60, "SD Card mounted!", COLOR_GREEN, COLOR_BLACK);
        
        // Display card information
        char info[64];
        snprintf(info, sizeof(info), "Name: %s", card->cid.name);
        display_draw_text(10, 80, info, COLOR_BLUE, COLOR_BLACK);
        
        snprintf(info, sizeof(info), "Size: %lluMB", ((uint64_t)card->csd.capacity) * card->csd.sector_size / (1024 * 1024));
        display_draw_text(10, 100, info, COLOR_BLUE, COLOR_BLACK);
        
        snprintf(info, sizeof(info), "Type: SD Card");
        display_draw_text(10, 120, info, COLOR_BLUE, COLOR_BLACK);
        
        snprintf(info, sizeof(info), "Speed: %uMHz", (unsigned int)(card->max_freq_khz / 1000));
        display_draw_text(10, 140, info, COLOR_BLUE, COLOR_BLACK);
        
        // Check for existing files
        FILE* f = fopen("/sdcard/scan_logs.csv", "r");
        if (f) {
            fclose(f);
            display_draw_text(10, 160, "Log files found", COLOR_GREEN, COLOR_BLACK);
        } else {
            display_draw_text(10, 160, "No log files", COLOR_GRAY, COLOR_BLACK);
        }
        
        ESP_LOGI(TAG, "SD card mounted successfully");
    } else {
        mounted = false;
        display_draw_text(10, 60, "SD Card failed", COLOR_RED, COLOR_BLACK);
        ESP_LOGW(TAG, "Failed to mount SD card: %s", esp_err_to_name(ret));
    }
    
    display_draw_text(10, 280, "Touch to continue", COLOR_GRAY, COLOR_BLACK);
    
    // Wait for touch
    while (true) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.pressed) {
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    return ret;
}

void sd_card_deinit(void) {
    if (mounted) {
        esp_vfs_fat_sdcard_unmount("/sdcard", card);
        mounted = false;
        ESP_LOGI(TAG, "SD card unmounted");
    }
}

bool sd_card_is_mounted(void) {
    return mounted;
}