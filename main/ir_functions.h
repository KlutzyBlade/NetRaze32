#ifndef IR_FUNCTIONS_H
#define IR_FUNCTIONS_H

#include "esp_err.h"

esp_err_t ir_init(void);
void ir_send_nec(uint32_t address, uint32_t command);
void ir_tv_power_attack(void);
void ir_ac_attack(void);
void ir_universal_remote(void);
void ir_signal_recorder(void);
void ir_jammer(void);

#endif