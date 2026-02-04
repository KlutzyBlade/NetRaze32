#ifndef USB_MSC_H
#define USB_MSC_H

#include "esp_err.h"

esp_err_t usb_msc_init(void);
void usb_msc_deinit(void);
esp_err_t usb_msc_mode_ui(void);

#endif
