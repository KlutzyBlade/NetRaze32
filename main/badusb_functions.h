#ifndef BADUSB_FUNCTIONS_H
#define BADUSB_FUNCTIONS_H

#include "esp_err.h"

esp_err_t badusb_init(void);
void badusb_execute_payload(void);

#endif