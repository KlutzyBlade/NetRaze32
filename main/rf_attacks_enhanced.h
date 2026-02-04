#ifndef RF_ATTACKS_ENHANCED_H
#define RF_ATTACKS_ENHANCED_H

#include "esp_err.h"

// Enhanced RF attack functions imported from Bruce
void rf_jammer_full(void);
void rf_jammer_intermittent(void);
void rf_spectrum_analyzer_enhanced(void);
void rf_signal_recorder(void);
void rf_replay_attack_enhanced(void);

#endif // RF_ATTACKS_ENHANCED_H