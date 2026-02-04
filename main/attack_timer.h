#ifndef ATTACK_TIMER_H
#define ATTACK_TIMER_H

#include <stdint.h>
#include <stdbool.h>

void attack_timer_start(uint32_t duration_sec);
void attack_timer_stop(void);
void attack_timer_draw_overlay(void);
bool attack_timer_expired(void);
uint32_t attack_timer_remaining(void);

#endif
