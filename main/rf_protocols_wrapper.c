#include "rf_functions.h"
#include "subghz_protocols.h"
#include "display.h"
#include "touchscreen.h"

void rf_princeton_tx(void) {
    subghz_send_princeton(0x123456, 24, 350, 5);
}

void rf_garage_protocols(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Garage Protocols", COLOR_WHITE, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_WHITE);
    
    display_fill_rect(10, 40, 220, 25, COLOR_DARKBLUE);
    display_draw_rect(10, 40, 220, 25, COLOR_GRAY);
    display_draw_text(15, 50, "CAME (12-bit)", COLOR_GREEN, COLOR_DARKBLUE);
    
    display_fill_rect(10, 70, 220, 25, COLOR_DARKBLUE);
    display_draw_rect(10, 70, 220, 25, COLOR_GRAY);
    display_draw_text(15, 80, "Nice FLO (12-bit)", COLOR_BLUE, COLOR_DARKBLUE);
    
    display_fill_rect(10, 100, 220, 25, COLOR_DARKBLUE);
    display_draw_rect(10, 100, 220, 25, COLOR_GRAY);
    display_draw_text(15, 110, "Linear (10-bit)", COLOR_ORANGE, COLOR_DARKBLUE);
    
    display_fill_rect(10, 130, 220, 25, COLOR_DARKBLUE);
    display_draw_rect(10, 130, 220, 25, COLOR_GRAY);
    display_draw_text(15, 140, "Chamberlain Sec+", COLOR_PURPLE, COLOR_DARKBLUE);
    
    display_fill_rect(10, 160, 220, 25, COLOR_DARKBLUE);
    display_draw_rect(10, 160, 220, 25, COLOR_GRAY);
    display_draw_text(15, 170, "KeeLoq Rolling", COLOR_RED, COLOR_DARKBLUE);
    
    display_fill_rect(10, 280, 80, 30, COLOR_DARKBLUE);
    display_draw_rect(10, 280, 80, 30, COLOR_WHITE);
    display_draw_text(35, 290, "BACK", COLOR_WHITE, COLOR_DARKBLUE);
    
    while (true) {
        if (touchscreen_is_touched()) {
            touch_point_t point = touchscreen_get_point();
            if (point.pressed) {
                if (point.y >= 40 && point.y <= 65) {
                    subghz_send_came(0xABC, 12);
                    break;
                } else if (point.y >= 70 && point.y <= 95) {
                    subghz_send_nice(0xDEF, 12);
                    break;
                } else if (point.y >= 100 && point.y <= 125) {
                    subghz_send_linear(0x123, 10);
                    break;
                } else if (point.y >= 130 && point.y <= 155) {
                    subghz_send_chamberlain(0x456789, 20);
                    break;
                } else if (point.y >= 160 && point.y <= 185) {
                    subghz_send_keeloq(0x123456789ABCDEF, 0x12345678, 100);
                    break;
                } else if (point.x >= 10 && point.x <= 90 && point.y >= 280 && point.y <= 310) {
                    break;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void rf_raw_capture(void) {
    subghz_capture_raw(433920000, 5000);
}