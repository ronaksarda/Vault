#include "fs.h"

#include <sys/stat.h>   // mkdir, rmdir
#include <errno.h>      // errno
#include <string.h>     // strerror
#include <stdio.h>      // snprintf
#include<dirent.h>      //Dir ,

int fs_create_vault_dir(const char* vault_name)
{
    if (!vault_name || vault_name[0] == '\0')
        return -1;

    printf("[FS] Ensuring root directory: %s\n", VAULTS_ROOT);

    if (mkdir(VAULTS_ROOT) == -1) {
        if (errno != EEXIST) {
            perror("[FS] Failed to create Vaults_folder");
            return -2;
        }
    }

    char path[256];
    snprintf(path, sizeof(path), "%s/%s", VAULTS_ROOT, vault_name);

    printf("[FS] Creating vault directory: %s\n", path);

    if (mkdir(path) == -1) {
        if (errno == EEXIST) {
            printf("[FS] Vault directory already exists.\n");
            return 0;
        }
        perror("[FS] Failed to create vault directory");
        return -3;
    }

    printf("[FS] Vault directory created successfully.\n");
    return 0;
}



int fs_delete_vault_dir(const char* vault_name)
{
    if (vault_name == NULL || vault_name[0] == '\0')
        return -1;

    char path[256];
    snprintf(path, sizeof(path), "%s/%s", VAULTS_ROOT, vault_name);

    if (rmdir(path) == -1) {
        if (errno == ENOENT)
            return -2;  // directory does not exist
        else if (errno == ENOTEMPTY)
            return -3;  // directory not empty
        else
            return -4;  // other filesystem error
    }

    return 0; // success
}

int fs_create_file_in_vault(const char* vault_name, const char* filename)
{
    if (!vault_name || !filename)
        return -1;

    if (vault_name[0] == '\0' || filename[0] == '\0')
        return -1;

    char path[256];

    // Build full path
    snprintf(path, sizeof(path), "%s/%s/%s",
             VAULTS_ROOT, vault_name, filename);

    // Create empty file
    FILE* fp = fopen(path, "w");
    if (!fp) {
        return -2; // filesystem error
    }

    fclose(fp);
    return 0;
}

int fs_delete_file_in_vault(const char* vault_name,const char* filename){
    if (!vault_name || !filename)
        return -1;

    if (vault_name[0] == '\0' || filename[0] == '\0')
        return -1;

    char path[256];

    snprintf(path,sizeof(path),"%s/%s/%s",VAULTS_ROOT,vault_name,filename);

    if(remove(path)!= 0){
        return -2;
    }
    return 0;
}


int fs_scan_vault_dir(const char* vault_name, Vault* vault)
{
    if (!vault_name || !vault)
        return -1;

    if (vault_name[0] == '\0')
        return -1;

    char path[256];
    snprintf(path, sizeof(path),
             "%s/%s", VAULTS_ROOT, vault_name);

    DIR* dir = opendir(path);
    if (!dir)
        return -2; // vault directory missing

    // 🔴 filesystem is source of truth
    vault->file_count = 0;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {

        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
            continue;

        char fullpath[512];
          int n = snprintf(fullpath, sizeof(fullpath),"%s/%s", path, entry->d_name);

    if (n < 0 || n >= (int)sizeof(fullpath)) {
            // Path too long or formatting error — skip safely
            continue;
        }


        struct stat st;
        if (stat(fullpath, &st) != 0)
            continue;

        // Ignore directories
        if (!S_ISREG(st.st_mode))
            continue;

        // Handle .locked files
        char logical_name[MAX_FILENAME];
        char stored_name[MAX_FILENAME];

// stored name = exact filename on disk
strncpy(stored_name, entry->d_name, MAX_FILENAME - 1);
stored_name[MAX_FILENAME - 1] = '\0';

// logical name = strip ".locked" IF present
strncpy(logical_name, entry->d_name, MAX_FILENAME - 1);
logical_name[MAX_FILENAME - 1] = '\0';

const char* ext = ".locked";
size_t len = strlen(logical_name);
size_t ext_len = strlen(ext);

if (len > ext_len &&
    strcmp(logical_name + len - ext_len, ext) == 0) {
    logical_name[len - ext_len] = '\0';
}

// IMPORTANT: logical name vs stored name
    add_file_to_vault(vault, logical_name, stored_name);
}
return 0;
}


int fs_delete_vault_dir_recursive(const char* vault_name)
{
    char path[512];
    int n = snprintf(path, sizeof(path),
                     "%s/%s", VAULTS_ROOT, vault_name);

    if (n < 0 || n >= (int)sizeof(path))
        return -1;

    DIR* dir = opendir(path);
    if (!dir)
        return -2;

    struct dirent* entry;
    char fullpath[512];

    while ((entry = readdir(dir)) != NULL) {

        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
            continue;

        n = snprintf(fullpath, sizeof(fullpath),
                     "%s/%s", path, entry->d_name);

        if (n < 0 || n >= (int)sizeof(fullpath)) {
            /* Path too long → skip safely */
            continue;
        }

        struct stat st;
        if (stat(fullpath, &st) == 0) {
            if (S_ISREG(st.st_mode)) {
                remove(fullpath);
            }
        }
    }

    closedir(dir);
    return rmdir(path);
}
