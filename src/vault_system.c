#include "vault_system.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "fs.h"
#include "crypto.h"

// ================= System Init =================

void system_init(VaultSystem* vault_sys)
{
    if (!vault_sys)
        return;

    vault_sys->vault_count = 0;
}

// ================= Vault Creation =================

int create_vault(VaultSystem* vault_sys,
                 const char* name,
                 const char* password)
{
    if (!vault_sys || !name || !password)
        return -1;

    if (name[0] == '\0' || password[0] == '\0')
        return -1;

    if (vault_sys->vault_count >= MAX_VAULTS)
        return -2; // capacity full

    // Enforce unique vault names
    for (size_t i = 0; i < vault_sys->vault_count; i++) {
        if (strcmp(vault_sys->lockers[i].vault_name, name) == 0)
            return -3; // duplicate vault
    }

    // Filesystem is source of truth
    if (fs_create_vault_dir(name) != 0)
        return -4; // filesystem error

    Vault* v = &vault_sys->lockers[vault_sys->vault_count];

    strncpy(v->vault_name, name, MAX_FILENAME - 1);
    v->vault_name[MAX_FILENAME - 1] = '\0';

    strncpy(v->password, password, MAX_PASSWORD - 1);
    v->password[MAX_PASSWORD - 1] = '\0';

    v->file_count = 0;
    vault_sys->vault_count++;

    return 0;
}

// ================= Vault Open (API preserved) =================

int open_vault(VaultSystem* vault_sys,
               const char* name,
               const char* password)
{
    if (!vault_sys || !name || !password)
        return -1;

    for (size_t i = 0; i < vault_sys->vault_count; i++) {
        Vault* v = &vault_sys->lockers[i];

        if (strcmp(v->vault_name, name) == 0) {
            if (strcmp(v->password, password) == 0)
                return 0;   // success
            return -3;      // wrong password
        }
    }

    return -4; // vault not found
}

// ================= Vault Delete =================

int delete_vault(VaultSystem* vault_sys,
                 const char* name,
                 const char* password)
{
    if (!vault_sys || !name || !password)
        return -1;

    for (size_t i = 0; i < vault_sys->vault_count; i++) {
        Vault* v = &vault_sys->lockers[i];

        if (strcmp(v->vault_name, name) == 0) {
            if (strcmp(v->password, password) != 0)
                return -3; // wrong password

            // Shift vaults left
            for (size_t j = i; j + 1 < vault_sys->vault_count; j++)
                vault_sys->lockers[j] = vault_sys->lockers[j + 1];

            vault_sys->vault_count--;
            return 0;
        }
    }

    return -4; // vault not found
}

// ================= Vault List =================

int vault_list(const VaultSystem* vault_sys)
{
    if (!vault_sys)
        return -1;

    if (vault_sys->vault_count == 0) {
        printf("No vaults available.\n");
        return 0;
    }

    printf("List of vaults:\n");
    for (size_t i = 0; i < vault_sys->vault_count; i++) {
        printf("%zu. %s (files: %zu)\n",
               i + 1,
               vault_sys->lockers[i].vault_name,
               vault_sys->lockers[i].file_count);
    }

    return 0;
}

// ================= Vault Selection =================

Vault* select_vault(VaultSystem* vault_sys, const char* input)
{
    if (!vault_sys || !input || input[0] == '\0')
        return NULL;

    int numeric = 1;
    for (size_t i = 0; input[i]; i++) {
        if (!isdigit((unsigned char)input[i])) {
            numeric = 0;
            break;
        }
    }

    if (numeric) {
        char* end;
        unsigned long index = strtoul(input, &end, 10);

        if (*end != '\0' || index < 1 || index > vault_sys->vault_count)
            return NULL;

        return &vault_sys->lockers[index - 1];
    }

    for (size_t i = 0; i < vault_sys->vault_count; i++) {
        if (strcmp(vault_sys->lockers[i].vault_name, input) == 0)
            return &vault_sys->lockers[i];
    }

    return NULL;
}

// ================= Lock Helpers =================

static int is_locked(const char* name)
{
    const char* ext = ".locked";
    size_t len = strlen(name);
    size_t ext_len = strlen(ext);

    return (len > ext_len &&
            strcmp(name + len - ext_len, ext) == 0);
}

// ================= Vault Lock =================

int lock_vault(Vault* vault)
{
    if (!vault)
        return -1;

    // Reject mixed or already locked vault
    for (size_t i = 0; i < vault->file_count; i++) {
        if (is_locked(vault->files[i].encrypted_path))
            return -5; // vault not fully unlocked
    }

    for (size_t i = 0; i < vault->file_count; i++) {
        VaultFile* vf = &vault->files[i];

        char path[512];
        snprintf(path, sizeof(path),
                 "%s/%s/%s",
                 VAULTS_ROOT,
                 vault->vault_name,
                 vf->encrypted_path);

        if (crypto_encrypt_file(path, vault->password) != 0)
            return -2;

        char new_name[MAX_FILENAME];
        int n = snprintf(new_name, sizeof(new_name),
                         "%s.locked", vf->encrypted_path);
        if (n < 0 || n >= (int)sizeof(new_name))
            return -4;

        char new_path[512];
        snprintf(new_path, sizeof(new_path),
                 "%s/%s/%s",
                 VAULTS_ROOT,
                 vault->vault_name,
                 new_name);

        if (rename(path, new_path) != 0)
            return -3;

        strncpy(vf->encrypted_path, new_name, MAX_FILENAME - 1);
        vf->encrypted_path[MAX_FILENAME - 1] = '\0';
    }

    return 0;
}

// ================= Vault Unlock =================

int unlock_vault(Vault* vault, const char* password)
{
    if (!vault || !password)
        return -1;

    // Authenticate first to avoid information leaks
    if (strcmp(vault->password, password) != 0)
        return -2;

    // Ensure vault is fully locked
    for (size_t i = 0; i < vault->file_count; i++) {
        if (!is_locked(vault->files[i].encrypted_path))
            return -7; // mixed or already unlocked
    }

    for (size_t i = 0; i < vault->file_count; i++) {
        VaultFile* vf = &vault->files[i];

        char path[512];
        snprintf(path, sizeof(path),
                 "%s/%s/%s",
                 VAULTS_ROOT,
                 vault->vault_name,
                 vf->encrypted_path);

        if (crypto_decrypt_file(path, password) != 0)
            return -3;

        char original_name[MAX_FILENAME];
        strncpy(original_name, vf->encrypted_path, MAX_FILENAME - 1);
        original_name[MAX_FILENAME - 1] = '\0';

        size_t len = strlen(original_name);
        size_t ext_len = strlen(".locked");

        if (len <= ext_len)
            return -6;

        original_name[len - ext_len] = '\0';

        char new_path[512];
        int n = snprintf(new_path, sizeof(new_path),
                         "%s/%s/%s",
                         VAULTS_ROOT,
                         vault->vault_name,
                         original_name);
        if (n < 0 || n >= (int)sizeof(new_path))
            return -4;

        if (rename(path, new_path) != 0)
            return -5;

        strncpy(vf->encrypted_path, original_name, MAX_FILENAME - 1);
        vf->encrypted_path[MAX_FILENAME - 1] = '\0';
    }

    return 0;
}

// ================= Vault Merge =================

int merge_vaults(VaultSystem* system,
                 const char* src_name,
                 const char* dst_name,
                 const char* password)
{
    // NOTE: merge_vaults is non-atomic.
    // Partial failure may require manual recovery.

    if (!system || !src_name || !dst_name || !password)
        return -1;

    if (strcmp(src_name, dst_name) == 0)
        return -2;

    Vault* src = select_vault(system, src_name);
    Vault* dst = select_vault(system, dst_name);
    if (!src || !dst)
        return -3;

    // Enforce same-password merge policy
    if (strcmp(src->password, password) != 0 ||
        strcmp(dst->password, password) != 0)
        return -4;

    for (size_t i = 0; i < src->file_count; i++)
        if (is_locked(src->files[i].encrypted_path))
            return -5;

    for (size_t i = 0; i < dst->file_count; i++)
        if (is_locked(dst->files[i].encrypted_path))
            return -6;

    for (size_t i = 0; i < src->file_count; i++)
        if (find_file(dst, src->files[i].filename))
            return -7;

    for (size_t i = 0; i < src->file_count; i++) {
        VaultFile* vf = &src->files[i];

        char old_path[512];
        char new_path[512];

        snprintf(old_path, sizeof(old_path),
                 "%s/%s/%s",
                 VAULTS_ROOT, src->vault_name, vf->encrypted_path);

        snprintf(new_path, sizeof(new_path),
                 "%s/%s/%s",
                 VAULTS_ROOT, dst->vault_name, vf->encrypted_path);

        if (rename(old_path, new_path) != 0)
            return -8;

        add_file_to_vault(dst, vf->filename, vf->encrypted_path);
    }

    src->file_count = 0;

    if (fs_delete_vault_dir(src->vault_name) != 0)
        return -9;

    return 0;
}
