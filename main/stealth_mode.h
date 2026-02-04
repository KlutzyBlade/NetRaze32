#ifndef STEALTH_MODE_H
#define STEALTH_MODE_H

#include <stdbool.h>

bool stealth_mode_init(void);
void stealth_mode_enable(void);
void stealth_mode_disable(void);
void stealth_mode_menu(void);
bool stealth_mode_is_active(void);

#endif