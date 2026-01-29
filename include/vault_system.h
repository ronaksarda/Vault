#ifndef VAULT_SYSTEM_H
#define VAULT_SYSTEM_H
#include "vault.h"

#define MAX_VAULTS 32

typedef struct{
    Vault lockers[MAX_VAULTS];
    size_t vault_count;
}VaultSystem;

// vault_system.h

int lock_vault(Vault* vault);
int unlock_vault(Vault* vault, const char* password); // we'll implement next


void system_init(VaultSystem* vault_sys);
int vault_list(const VaultSystem* vault_sys);
Vault* select_vault(VaultSystem* vault_sys,const char* name);


int create_vault(VaultSystem* vault_sys, const char* name, const char* password);
int open_vault(VaultSystem* vault_sys, const char* name, const char* password);
int delete_vault(VaultSystem* vault_sys, const char* name, const char* password);
int merge_vaults(VaultSystem* system,const char* src_name,const char* dst_name,const char* password);

#endif // VAULT_SYSTEMS_H , this file is used to define the structure and functions for managing multiple vaults in a vault system.