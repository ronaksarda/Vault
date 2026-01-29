#include "vault_system.h"
#include "reader.h"
#include<stdio.h>


void system_load(VaultSystem* vault_sys){
    FILE *fp = fopen("data/vault.dat","rb");
    if(!fp){
        vault_sys->vault_count=0;
        return;
    }
    size_t read_count = fread(vault_sys,sizeof(VaultSystem),1,fp);
    if(read_count != 1){
        vault_sys->vault_count=0;
    }
    fclose(fp);
}

void system_save(const VaultSystem* vault_sys){
    FILE *fp = fopen("data/vault.dat","wb");
    if(!fp){
        //file not found or cannot be opened
        return;
    }
    fwrite(vault_sys,sizeof(VaultSystem),1,fp);
    fclose(fp);
};
