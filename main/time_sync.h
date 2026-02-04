#ifndef TIME_SYNC_H
#define TIME_SYNC_H

#include "esp_err.h"
#include <stdbool.h>

esp_err_t time_sync_init(void);
void time_sync_start(void);
bool is_time_synced(void);
void get_current_time_string(char* time_str, size_t max_len);

#endif // TIME_SYNC_H