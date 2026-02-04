#include "battery_monitor.h"
#include "utils.h"
#include "display.h"
#include "esp_log.h"
#include <string.h>

typedef struct {
    const char* attack_name;
    float avg_current_ma;
} attack_power_t;

static const attack_power_t power_profiles[] = {
    {"WiFi Deauth", 180.0f},
    {"BLE Spam", 120.0f},
    {"WiFi Scan", 90.0f},
    {"RF Jammer", 200.0f},
    {"Idle", 50.0f}
};

float battery_get_percentage(void) {
    float v = read_battery_voltage();
    if (v >= 4.2f) return 100.0f;
    if (v <= 3.0f) return 0.0f;
    return ((v - 3.0f) / 1.2f) * 100.0f;
}

int battery_estimate_runtime(const char* attack_type) {
    float capacity_mah = 1000.0f;
    float current_ma = 50.0f;
    
    for (int i = 0; i < sizeof(power_profiles) / sizeof(power_profiles[0]); i++) {
        if (strcmp(power_profiles[i].attack_name, attack_type) == 0) {
            current_ma = power_profiles[i].avg_current_ma;
            break;
        }
    }
    
    float pct = battery_get_percentage();
    float remaining_mah = (pct / 100.0f) * capacity_mah;
    return (int)((remaining_mah / current_ma) * 60);
}

void battery_draw_widget(int x, int y) {
    float pct = battery_get_percentage();
    
    display_draw_rect(x, y, 40, 15, COLOR_WHITE);
    display_fill_rect(x + 40, y + 5, 3, 5, COLOR_WHITE);
    
    int fill = (int)((pct / 100.0f) * 36);
    uint16_t color = pct > 50 ? COLOR_GREEN : pct > 20 ? COLOR_ORANGE : COLOR_RED;
    display_fill_rect(x + 2, y + 2, fill, 11, color);
    
    char txt[8];
    snprintf(txt, sizeof(txt), "%.0f%%", pct);
    display_draw_text(x + 50, y + 3, txt, COLOR_WHITE, COLOR_BLACK);
}

void battery_show_details(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Battery Status", COLOR_WHITE, COLOR_BLACK);
    
    float v = read_battery_voltage();
    float pct = battery_get_percentage();
    
    char buf[64];
    snprintf(buf, sizeof(buf), "Voltage: %.2fV", v);
    display_draw_text(10, 40, buf, COLOR_GREEN, COLOR_BLACK);
    
    snprintf(buf, sizeof(buf), "Charge: %.1f%%", pct);
    display_draw_text(10, 60, buf, COLOR_GREEN, COLOR_BLACK);
    
    display_draw_text(10, 90, "Estimated Runtime:", COLOR_ORANGE, COLOR_BLACK);
    
    int y = 110;
    for (int i = 0; i < sizeof(power_profiles) / sizeof(power_profiles[0]); i++) {
        int mins = battery_estimate_runtime(power_profiles[i].attack_name);
        snprintf(buf, sizeof(buf), "%s: %dmin", power_profiles[i].attack_name, mins);
        display_draw_text(15, y, buf, COLOR_WHITE, COLOR_BLACK);
        y += 20;
    }
    
    display_draw_text(10, 280, "Touch to close", COLOR_GRAY, COLOR_BLACK);
}
