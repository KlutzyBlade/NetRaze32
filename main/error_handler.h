#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void error_handler_init(void);
void handle_critical_error(const char* error_msg);
void handle_warning(const char* warning_msg);
void log_error(const char* component, esp_err_t error_code);

#endif