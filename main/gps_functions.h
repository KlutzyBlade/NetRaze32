#ifndef GPS_FUNCTIONS_H
#define GPS_FUNCTIONS_H

#include "esp_err.h"

esp_err_t gps_init(void);
void gps_get_location(void);

#endif