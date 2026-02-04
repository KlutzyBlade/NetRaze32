#ifndef AUTOMATION_H
#define AUTOMATION_H

#include "esp_err.h"
#include <stdbool.h>

typedef struct {
    char name[32];
    char commands[256];
    int delay_ms;
    bool enabled;
} script_t;

typedef struct {
    char name[32];
    int hour;
    int minute;
    int script_id;
    bool enabled;
} scheduled_task_t;

esp_err_t automation_init(void);
void automation_script_runner(void);
void automation_scheduled_tasks(void);
void automation_macro_recorder(void);
void automation_batch_operations(void);
void automation_auto_exploit(void);

#endif // AUTOMATION_H