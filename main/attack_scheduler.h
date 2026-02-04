#ifndef ATTACK_SCHEDULER_H
#define ATTACK_SCHEDULER_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    ATTACK_WIFI_DEAUTH,
    ATTACK_WIFI_BEACON_SPAM,
    ATTACK_WIFI_KARMA,
    ATTACK_BLE_APPLE_SPAM,
    ATTACK_BLE_SAMSUNG_SPAM,
    ATTACK_BLE_JAMMER,
    ATTACK_PACKET_CAPTURE,
    ATTACK_TYPE_MAX
} attack_type_t;

typedef enum {
    SCHED_ONCE,
    SCHED_REPEAT,
    SCHED_INTERVAL
} schedule_mode_t;

typedef struct {
    attack_type_t type;
    schedule_mode_t mode;
    uint32_t duration_sec;
    uint32_t interval_sec;
    uint32_t repeat_count;
    bool active;
    uint32_t executions;
} scheduled_attack_t;

// Initialize scheduler
void attack_scheduler_init(void);

// Add attack to queue
bool attack_scheduler_add(attack_type_t type, schedule_mode_t mode, 
                          uint32_t duration, uint32_t interval, uint32_t repeats);

// Start scheduler task
void attack_scheduler_start(void);

// Stop scheduler
void attack_scheduler_stop(void);

// Get scheduler status
bool attack_scheduler_is_running(void);
uint8_t attack_scheduler_get_queue_count(void);

// Clear all scheduled attacks
void attack_scheduler_clear(void);

// UI for configuring attacks
void attack_scheduler_ui(void);

#endif // ATTACK_SCHEDULER_H
