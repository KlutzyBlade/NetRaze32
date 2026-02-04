#ifndef ATTACK_PROFILES_H
#define ATTACK_PROFILES_H

#include <stdbool.h>

bool attack_profiles_init(void);
void attack_profiles_save(void);
void attack_profiles_menu(void);
void attack_profiles_run(int profile_id);
void attack_profiles_edit(int profile_id);

#endif