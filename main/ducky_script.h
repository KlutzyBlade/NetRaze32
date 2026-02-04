#ifndef DUCKY_SCRIPT_H
#define DUCKY_SCRIPT_H

#include "esp_err.h"
#include <stdbool.h>

typedef struct {
    char command[32];
    char parameter[128];
    uint16_t delay_ms;
} ducky_command_t;

esp_err_t ducky_script_init(void);
void ducky_script_interpreter(void);
void ducky_execute_command(const char* command, const char* parameter);
void ducky_run_script_from_sd(const char* filename);
void ducky_script_editor(void);

#endif // DUCKY_SCRIPT_H