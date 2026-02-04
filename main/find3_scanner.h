#ifndef FIND3_SCANNER_H
#define FIND3_SCANNER_H

#include "esp_err.h"
#include <stdbool.h>

typedef struct {
    char server_url[128];
    char device_name[32];
    char family_name[32];
    char location[32];
    int scan_interval_ms;
    bool learning_mode;
} find3_config_t;

esp_err_t find3_init(const find3_config_t *config);
esp_err_t find3_start_scanning(void);
esp_err_t find3_stop_scanning(void);
esp_err_t find3_set_learning_mode(bool enable, const char *location);
bool find3_is_scanning(void);

#endif // FIND3_SCANNER_H
