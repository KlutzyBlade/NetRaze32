#ifndef SD_CARD_H
#define SD_CARD_H

#include "esp_err.h"
#include <stdbool.h>

esp_err_t sd_card_init(void);
void sd_card_deinit(void);
bool sd_card_is_mounted(void);

#endif // SD_CARD_H