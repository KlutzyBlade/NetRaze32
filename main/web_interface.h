#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include "esp_err.h"

// Web interface functions
esp_err_t web_interface_init(void);
void web_interface_start(void);
void web_interface_stop(void);

#endif // WEB_INTERFACE_H