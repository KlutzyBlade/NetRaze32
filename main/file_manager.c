#include "file_manager.h"
#include "display.h"
#include "touchscreen.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

static const char* TAG = "FILE_MANAGER";
static file_browser_t browser = {0};
static bool initialized = false;

static file_type_t get_file_type(const char* filename) {
    const char* ext = strrchr(filename, '.');
    if (!ext) return FILE_TYPE_UNKNOWN;
    
    if (strcmp(ext, ".txt") == 0) return FILE_TYPE_TXT;
    if (strcmp(ext, ".csv") == 0) return FILE_TYPE_CSV;
    if (strcmp(ext, ".log") == 0) return FILE_TYPE_LOG;
    if (strcmp(ext, ".bin") == 0) return FILE_TYPE_BIN;
    return FILE_TYPE_UNKNOWN;
}

static const char* get_file_icon(file_type_t type) {
    switch (type) {
        case FILE_TYPE_DIR: return "[DIR]";
        case FILE_TYPE_TXT: return "[TXT]";
        case FILE_TYPE_CSV: return "[CSV]";
        case FILE_TYPE_LOG: return "[LOG]";
        case FILE_TYPE_BIN: return "[BIN]";
        default: return "[???]";
    }
}

static uint16_t get_file_color(file_type_t type) {
    switch (type) {
        case FILE_TYPE_DIR: return COLOR_BLUE;
        case FILE_TYPE_TXT: return COLOR_WHITE;
        case FILE_TYPE_CSV: return COLOR_GREEN;
        case FILE_TYPE_LOG: return COLOR_ORANGE;
        case FILE_TYPE_BIN: return COLOR_RED;
        default: return COLOR_GRAY;
    }
}

static void scan_directory(const char* path) {
    DIR* dir = opendir(path);
    if (!dir) {
        ESP_LOGE(TAG, "Failed to open directory: %s", path);
        return;
    }
    
    browser.file_count = 0;
    struct dirent* entry;
    
    // Add parent directory entry if not root
    if (strcmp(path, "/sdcard") != 0) {
        strcpy(browser.files[0].name, "..");
        strcpy(browser.files[0].path, path);
        browser.files[0].type = FILE_TYPE_DIR;
        browser.files[0].size = 0;
        browser.files[0].is_selected = false;
        browser.file_count = 1;
    }
    
    while ((entry = readdir(dir)) != NULL && browser.file_count < MAX_FILES_PER_DIR) {
        if (entry->d_name[0] == '.' && strcmp(entry->d_name, "..") != 0) {
            continue; // Skip hidden files except ..
        }
        
        file_entry_t* file = &browser.files[browser.file_count];
        strncpy(file->name, entry->d_name, MAX_FILENAME_LEN - 1);
        file->name[MAX_FILENAME_LEN - 1] = '\0';
        
        strncpy(file->path, path, MAX_PATH_LEN - 1);
        strncat(file->path, "/", MAX_PATH_LEN - strlen(file->path) - 1);
        strncat(file->path, entry->d_name, MAX_PATH_LEN - strlen(file->path) - 1);
        file->path[MAX_PATH_LEN - 1] = '\0';
        
        struct stat st;
        if (stat(file->path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                file->type = FILE_TYPE_DIR;
                file->size = 0;
            } else {
                file->type = get_file_type(entry->d_name);
                file->size = st.st_size;
            }
        } else {
            file->type = FILE_TYPE_UNKNOWN;
            file->size = 0;
        }
        
        file->is_selected = false;
        browser.file_count++;
    }
    
    closedir(dir);
    browser.current_index = 0;
    strcpy(browser.current_path, path);
}

void file_manager_init(void) {
    if (initialized) return;
    
    // Initialize SD card
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    
    sdmmc_card_t* card;
    const char mount_point[] = "/sdcard";
    
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = GPIO_NUM_5;
    slot_config.host_id = SPI2_HOST;
    
    esp_err_t ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SD card");
        return;
    }
    
    scan_directory("/sdcard");
    initialized = true;
    ESP_LOGI(TAG, "File manager initialized");
}

void file_manager_show(void) {
    if (!initialized) {
        file_manager_init();
    }
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "File Manager", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    // Show current path
    char path_display[40];
    if (strlen(browser.current_path) > 35) {
        snprintf(path_display, sizeof(path_display), "...%s", 
                browser.current_path + strlen(browser.current_path) - 32);
    } else {
        strcpy(path_display, browser.current_path);
    }
    display_draw_text(10, 30, path_display, COLOR_GRAY, COLOR_BLACK);
    
    // Show files (max 10 visible)
    int start_y = 50;
    int visible_files = (browser.file_count > 10) ? 10 : browser.file_count;
    int scroll_offset = (browser.current_index >= 10) ? browser.current_index - 9 : 0;
    
    for (int i = 0; i < visible_files; i++) {
        int file_idx = i + scroll_offset;
        if (file_idx >= browser.file_count) break;
        
        file_entry_t* file = &browser.files[file_idx];
        int y = start_y + i * 22;
        
        // Highlight selected file
        if (file_idx == browser.current_index) {
            display_fill_rect(5, y - 2, 230, 20, COLOR_DARKBLUE);
        }
        
        // Draw file icon and name
        uint16_t color = get_file_color(file->type);
        display_draw_text(10, y, get_file_icon(file->type), color, 
                         (file_idx == browser.current_index) ? COLOR_DARKBLUE : COLOR_BLACK);
        
        char display_name[30];
        if (strlen(file->name) > 25) {
            strncpy(display_name, file->name, 22);
            strcpy(display_name + 22, "...");
        } else {
            strcpy(display_name, file->name);
        }
        
        display_draw_text(50, y, display_name, color,
                         (file_idx == browser.current_index) ? COLOR_DARKBLUE : COLOR_BLACK);
        
        // Show file size for files
        if (file->type != FILE_TYPE_DIR && file->size > 0) {
            char size_str[16];
            if (file->size < 1024) {
                snprintf(size_str, sizeof(size_str), "%dB", (int)file->size);
            } else if (file->size < 1024 * 1024) {
                snprintf(size_str, sizeof(size_str), "%dK", (int)(file->size / 1024));
            } else {
                snprintf(size_str, sizeof(size_str), "%dM", (int)(file->size / (1024 * 1024)));
            }
            display_draw_text(190, y, size_str, COLOR_GRAY,
                             (file_idx == browser.current_index) ? COLOR_DARKBLUE : COLOR_BLACK);
        }
    }
    
    // Show scroll indicator
    if (browser.file_count > 10) {
        int scroll_bar_height = 200;
        int scroll_pos = (browser.current_index * scroll_bar_height) / browser.file_count;
        display_draw_rect(235, 50, 4, scroll_bar_height, COLOR_GRAY);
        display_fill_rect(235, 50 + scroll_pos, 4, 10, COLOR_WHITE);
    }
    
    // Control buttons
    display_fill_rect(10, 280, 50, 25, COLOR_DARKBLUE);
    display_draw_rect(10, 280, 50, 25, COLOR_WHITE);
    display_draw_text(25, 290, "VIEW", COLOR_WHITE, COLOR_DARKBLUE);
    
    display_fill_rect(70, 280, 50, 25, COLOR_DARKBLUE);
    display_draw_rect(70, 280, 50, 25, COLOR_WHITE);
    display_draw_text(85, 290, "DEL", COLOR_WHITE, COLOR_DARKBLUE);
    
    display_fill_rect(130, 280, 50, 25, COLOR_DARKBLUE);
    display_draw_rect(130, 280, 50, 25, COLOR_WHITE);
    display_draw_text(145, 290, "COPY", COLOR_WHITE, COLOR_DARKBLUE);
    
    display_fill_rect(190, 280, 50, 25, COLOR_DARKBLUE);
    display_draw_rect(190, 280, 50, 25, COLOR_WHITE);
    display_draw_text(205, 290, "BACK", COLOR_WHITE, COLOR_DARKBLUE);
}

void file_manager_navigate_up(void) {
    if (browser.current_index > 0) {
        browser.current_index--;
        file_manager_show();
    }
}

void file_manager_navigate_down(void) {
    if (browser.current_index < browser.file_count - 1) {
        browser.current_index++;
        file_manager_show();
    }
}

void file_manager_select_file(void) {
    if (browser.file_count == 0) return;
    
    file_entry_t* file = &browser.files[browser.current_index];
    
    if (file->type == FILE_TYPE_DIR) {
        if (strcmp(file->name, "..") == 0) {
            // Go to parent directory
            char* last_slash = strrchr(browser.current_path, '/');
            if (last_slash && last_slash != browser.current_path) {
                *last_slash = '\0';
                scan_directory(browser.current_path);
            }
        } else {
            // Enter directory
            scan_directory(file->path);
        }
        file_manager_show();
    } else {
        file_manager_view_file();
    }
}

void file_manager_delete_file(void) {
    if (browser.file_count == 0) return;
    
    file_entry_t* file = &browser.files[browser.current_index];
    
    if (strcmp(file->name, "..") == 0) return;
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Delete File", COLOR_RED, COLOR_BLACK);
    display_draw_text(10, 40, "Are you sure?", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 60, file->name, COLOR_ORANGE, COLOR_BLACK);
    
    display_fill_rect(50, 100, 60, 30, COLOR_RED);
    display_draw_rect(50, 100, 60, 30, COLOR_WHITE);
    display_draw_text(70, 110, "YES", COLOR_WHITE, COLOR_RED);
    
    display_fill_rect(130, 100, 60, 30, COLOR_GREEN);
    display_draw_rect(130, 100, 60, 30, COLOR_WHITE);
    display_draw_text(150, 110, "NO", COLOR_WHITE, COLOR_GREEN);
    
    // Wait for touch decision
    while (true) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.pressed) {
                if (point.x >= 50 && point.x <= 110 && point.y >= 100 && point.y <= 130) {
                    // Delete confirmed
                    if (remove(file->path) == 0) {
                        ESP_LOGI(TAG, "Deleted: %s", file->path);
                        scan_directory(browser.current_path);
                    } else {
                        ESP_LOGE(TAG, "Failed to delete: %s", file->path);
                    }
                    break;
                } else if (point.x >= 130 && point.x <= 190 && point.y >= 100 && point.y <= 130) {
                    // Cancel
                    break;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    
    file_manager_show();
}

void file_manager_copy_file(void) {
    // Simplified copy - just show info for now
    if (browser.file_count == 0) return;
    
    file_entry_t* file = &browser.files[browser.current_index];
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Copy File", COLOR_BLUE, COLOR_BLACK);
    display_draw_text(10, 40, "File copied to clipboard", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 60, file->name, COLOR_ORANGE, COLOR_BLACK);
    display_draw_text(10, 280, "Touch to continue", COLOR_GRAY, COLOR_BLACK);
    
    ESP_LOGI(TAG, "File marked for copy: %s", file->path);
    
    while (!touchscreen_is_touched()) {
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    
    file_manager_show();
}

void file_manager_view_file(void) {
    if (browser.file_count == 0) return;
    
    file_entry_t* file = &browser.files[browser.current_index];
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "File Viewer", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    display_draw_text(10, 30, file->name, COLOR_ORANGE, COLOR_BLACK);
    
    char size_info[32];
    snprintf(size_info, sizeof(size_info), "Size: %d bytes", (int)file->size);
    display_draw_text(10, 50, size_info, COLOR_GRAY, COLOR_BLACK);
    
    // Try to read and display file content
    FILE* f = fopen(file->path, "r");
    if (f) {
        char line[80];
        int y = 80;
        int lines_shown = 0;
        
        while (fgets(line, sizeof(line), f) && lines_shown < 12) {
            // Remove newline
            line[strcspn(line, "\n")] = 0;
            
            // Truncate long lines
            if (strlen(line) > 35) {
                line[32] = '.';
                line[33] = '.';
                line[34] = '.';
                line[35] = 0;
            }
            
            display_draw_text(10, y, line, COLOR_WHITE, COLOR_BLACK);
            y += 15;
            lines_shown++;
        }
        
        fclose(f);
        
        if (lines_shown == 12) {
            display_draw_text(10, y, "...", COLOR_GRAY, COLOR_BLACK);
        }
    } else {
        display_draw_text(10, 80, "Cannot read file", COLOR_RED, COLOR_BLACK);
    }
    
    display_draw_text(10, 280, "Touch to go back", COLOR_GRAY, COLOR_BLACK);
    
    while (!touchscreen_is_touched()) {
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    
    file_manager_show();
}

bool file_manager_handle_touch(int x, int y) {
    // Handle file list touches
    if (x >= 5 && x <= 235 && y >= 50 && y <= 270) {
        int file_idx = (y - 50) / 22;
        int scroll_offset = (browser.current_index >= 10) ? browser.current_index - 9 : 0;
        file_idx += scroll_offset;
        
        if (file_idx < browser.file_count) {
            browser.current_index = file_idx;
            file_manager_select_file();
            return true;
        }
    }
    
    // Handle control buttons
    if (y >= 280 && y <= 305) {
        if (x >= 10 && x <= 60) {
            file_manager_view_file();
            return true;
        } else if (x >= 70 && x <= 120) {
            file_manager_delete_file();
            return true;
        } else if (x >= 130 && x <= 180) {
            file_manager_copy_file();
            return true;
        } else if (x >= 190 && x <= 240) {
            return false; // Back button
        }
    }
    
    return true;
}