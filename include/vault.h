#ifndef VAULT_H
#define VAULT_H

#include<stddef.h>

#define MAX_FILES 128
#define MAX_FILENAME 64
#define MAX_PASSWORD 32

typedef struct{
        char filename[MAX_FILENAME];
        char encrypted_path[MAX_FILENAME];
    } VaultFile;

typedef struct{
        char vault_name[MAX_FILENAME];
        char password[MAX_PASSWORD];
        VaultFile files[MAX_FILES];
        size_t file_count; //file_count <= MAX_FILES
    } Vault;

int list_files(const Vault* vault);
int add_file_to_vault(Vault* vault,const char* filename,const char* encrypted_path);
int remove_file_from_vault(Vault* vault,const char* filename);



VaultFile* find_file(Vault* vault , const char* filename);

#endif //This is Vault.h , used to define the structure of a vault and its files.
