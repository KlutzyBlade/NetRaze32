#ifndef RF_FUNCTIONS_H
#define RF_FUNCTIONS_H

#include "esp_err.h"

esp_err_t rf_24ghz_init(void);
esp_err_t rf_subghz_init(void);
void rf_jammer_24ghz(void);
void rf_scanner_24ghz(void);
void rf_replay_attack(void);
void rf_signal_generator(void);
void rf_spectrum_analyzer(void);

#endif