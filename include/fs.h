#ifndef FS_H
#define FS_H
#include "vault.h"
#include "vault_system.h"


#define VAULTS_ROOT "Vaults_folder"

/* vault directory management */
int fs_create_vault_dir(const char* vault_name);
int fs_delete_vault_dir(const char* vault_name);

/* file operations inside a vault */
int fs_create_file_in_vault(const char* vault_name, const char* filename);
int fs_scan_vault_dir(const char* vault_name, Vault* vault);
int fs_delete_file_in_vault(const char* vault_name,const char* filename);
int fs_delete_vault_dir_recursive(const char* vault_name);


#endif
