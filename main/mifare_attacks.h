#ifndef MIFARE_ATTACKS_H
#define MIFARE_ATTACKS_H

#include "esp_err.h"
#include <stdint.h>

typedef struct {
    uint8_t uid[7];
    uint8_t uid_len;
    uint8_t sak;
    uint8_t atqa[2];
} nfc_card_t;

esp_err_t mifare_attacks_init(void);
void mifare_nested_attack(void);
void mifare_hardnested_attack(void);
void mifare_ultralight_dump(void);
void mifare_classic_dump(void);
void mifare_key_recovery(void);

#endif // MIFARE_ATTACKS_H