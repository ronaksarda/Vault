#include"vault.h"
#include<string.h>
#include<stdio.h>
int add_file_to_vault(Vault* vault,
                      const char* filename,
                      const char* encrypted_path)
{
    if (!vault || !filename || !encrypted_path)
        return -1;

    if (filename[0] == '\0' || encrypted_path[0] == '\0')
        return -1;

    if (vault->file_count >= MAX_FILES)
        return -2;

    // 🔒 DUPLICATE CHECK (MUST BE BEFORE ADD)
    for (size_t i = 0; i < vault->file_count; i++) {
        if (strcmp(vault->files[i].filename, filename) == 0)
            return -3;
    }

    VaultFile* new_file = &vault->files[vault->file_count];

    strncpy(new_file->filename, filename, MAX_FILENAME - 1);
    new_file->filename[MAX_FILENAME - 1] = '\0';

    strncpy(new_file->encrypted_path, encrypted_path, MAX_FILENAME - 1);
    new_file->encrypted_path[MAX_FILENAME - 1] = '\0';

    vault->file_count++;
    return 0;
}




int list_files(const Vault* vault){
    if(vault == NULL)
        return -1;
    if(vault->file_count == 0){
        printf("No files found in the vault.");
        return 0;
    }

    printf("Files in vault: %s \n",vault->vault_name);
    for(size_t i = 0 ; i < vault->file_count; i++ ){
        printf("%zu. %s ",i+1,vault->files[i].filename);
    }
    return 0;
}

VaultFile* find_file(Vault* vault,const char* filename){
    if(vault == NULL){
        return NULL;
    }
    if(filename == NULL || filename[0] == '\0'){
        return NULL;
    }
    for(size_t i = 0 ; i <vault->file_count;i++){
        if(strcmp(vault->files[i].filename,filename) == 0){
            return &vault->files[i];
        }
    }
    return NULL;
}

int remove_file_from_vault(Vault* vault,const char* filename){
    if (!vault || !filename)
        return -1;

    if (filename[0] == '\0')
        return -1;

    size_t index = vault->file_count; // invalid index initially

    // Step 1: find the file
    for (size_t i = 0; i < vault->file_count; i++) {
        if (strcmp(vault->files[i].filename, filename) == 0) {
            index = i;
            break;
        }
    }

    // Step 2: file not found
    if (index == vault->file_count)
        return -2;

    // Step 3: shift remaining files left
    for (size_t i = index; i < vault->file_count - 1; i++) {
        vault->files[i] = vault->files[i + 1];
    }

    // Step 4: update count
    vault->file_count--;

    return 0;
}