#include "sd_browser.h"
#include "sd_card.h"
#include "display.h"
#include "touchscreen.h"
#include "esp_log.h"
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
static char current_path[256] = "/sdcard";
static char files[32][64];
static int file_count = 0;
static int scroll_pos = 0;
static int selected = 0;

static void scan_dir(void) {
    DIR* dir = opendir(current_path);
    if (!dir) return;
    
    file_count = 0;
    struct dirent* ent;
    while ((ent = readdir(dir)) && file_count < 32) {
        if (ent->d_name[0] != '.') {
            strncpy(files[file_count++], ent->d_name, 63);
        }
    }
    closedir(dir);
    selected = 0;
    scroll_pos = 0;
}

static void draw_browser(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 5, "SD Browser", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 20, current_path, COLOR_GRAY, COLOR_BLACK);
    
    for (int i = 0; i < 10 && (i + scroll_pos) < file_count; i++) {
        int idx = i + scroll_pos;
        int y = 40 + i * 20;
        uint16_t bg = (idx == selected) ? COLOR_DARKBLUE : COLOR_BLACK;
        display_fill_rect(5, y, 230, 18, bg);
        display_draw_text(10, y + 2, files[idx], COLOR_WHITE, bg);
    }
    
    display_fill_rect(10, 280, 50, 30, COLOR_RED);
    display_draw_text(20, 290, "DEL", COLOR_WHITE, COLOR_RED);
    display_fill_rect(70, 280, 50, 30, COLOR_BLUE);
    display_draw_text(80, 290, "VIEW", COLOR_WHITE, COLOR_BLUE);
    display_fill_rect(180, 280, 50, 30, COLOR_GRAY);
    display_draw_text(190, 290, "BACK", COLOR_WHITE, COLOR_GRAY);
}

esp_err_t sd_browser_ui(void) {
    if (!sd_card_is_mounted()) {
        display_fill_screen(COLOR_BLACK);
        display_draw_text(10, 100, "SD card not mounted", COLOR_RED, COLOR_BLACK);
        vTaskDelay(pdMS_TO_TICKS(2000));
        return ESP_FAIL;
    }
    
    scan_dir();
    draw_browser();
    
    while (1) {
        touch_point_t p = touchscreen_get_point();
        if (p.pressed) {
            if (p.y >= 40 && p.y < 240) {
                int idx = (p.y - 40) / 20 + scroll_pos;
                if (idx < file_count) selected = idx;
                draw_browser();
            } else if (p.y >= 280) {
                if (p.x >= 10 && p.x <= 60) {
                    char path[300];
                    snprintf(path, sizeof(path), "%s/%s", current_path, files[selected]);
                    remove(path);
                    scan_dir();
                    draw_browser();
                } else if (p.x >= 70 && p.x <= 120) {
                    char path[300];
                    snprintf(path, sizeof(path), "%s/%s", current_path, files[selected]);
                    FILE* f = fopen(path, "r");
                    if (f) {
                        display_fill_screen(COLOR_BLACK);
                        display_draw_text(10, 5, files[selected], COLOR_ORANGE, COLOR_BLACK);
                        char line[40];
                        int y = 30;
                        for (int i = 0; i < 15 && fgets(line, sizeof(line), f); i++) {
                            line[strcspn(line, "\n")] = 0;
                            display_draw_text(5, y, line, COLOR_WHITE, COLOR_BLACK);
                            y += 18;
                        }
                        fclose(f);
                        while (!touchscreen_get_point().pressed) vTaskDelay(pdMS_TO_TICKS(50));
                        draw_browser();
                    }
                } else if (p.x >= 180) {
                    return ESP_OK;
                }
            }
            vTaskDelay(pdMS_TO_TICKS(300));
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
