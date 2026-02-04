#ifndef NFC_FUNCTIONS_H
#define NFC_FUNCTIONS_H

#include "esp_err.h"

esp_err_t nfc_init(void);
void nfc_scan_cards(void);

#endif