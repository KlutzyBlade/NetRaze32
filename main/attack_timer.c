#include "attack_timer.h"
#include "display.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

static uint32_t start_time = 0;
static uint32_t duration = 0;
static bool active = false;

void attack_timer_start(uint32_t duration_sec) {
    start_time = xTaskGetTickCount();
    duration = duration_sec;
    active = true;
}

void attack_timer_stop(void) {
    active = false;
}

void attack_timer_draw_overlay(void) {
    if (!active) return;
    
    uint32_t elapsed = (xTaskGetTickCount() - start_time) / 1000;
    uint32_t remaining = (elapsed < duration) ? (duration - elapsed) : 0;
    
    uint32_t min = remaining / 60;
    uint32_t sec = remaining % 60;
    
    display_fill_rect(170, 25, 65, 20, COLOR_DARKGRAY);
    display_draw_rect(170, 25, 65, 20, COLOR_ORANGE);
    
    char buf[16];
    snprintf(buf, sizeof(buf), "%02lu:%02lu", min, sec);
    display_draw_text(180, 30, buf, COLOR_ORANGE, COLOR_DARKGRAY);
}

bool attack_timer_expired(void) {
    if (!active) return false;
    uint32_t elapsed = (xTaskGetTickCount() - start_time) / 1000;
    return elapsed >= duration;
}

uint32_t attack_timer_remaining(void) {
    if (!active) return 0;
    uint32_t elapsed = (xTaskGetTickCount() - start_time) / 1000;
    return (elapsed < duration) ? (duration - elapsed) : 0;
}
