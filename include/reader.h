#ifndef READER_H
#define READER_H
#include "vault_system.h"

void system_load(VaultSystem* vault_sys);
void system_save(const VaultSystem* vault_sys);

#endif