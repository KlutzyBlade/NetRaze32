#ifndef OUI_SPY_H
#define OUI_SPY_H

#include "esp_err.h"
#include <stdint.h>

esp_err_t oui_spy_init(void);
void oui_spy_scan(void);
void oui_spy_flock_detector(void);
void oui_spy_add_filter(const char* mac_or_oui, const char* description);
void oui_spy_clear_filters(void);
int oui_spy_get_device_count(void);

#endif
