#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vault_system.h"
#include "vault.h"
#include "fs.h"
#include "reader.h"

#define MASTER_OVERRIDE_KEY "VAULT_MASTER_2026"

/* ================= Utility ================= */

static void read_line(char* buf, size_t size)
{
    if (fgets(buf, size, stdin)) {
        buf[strcspn(buf, "\n")] = '\0';
    }
}

/* ================= Authentication ================= */

static int authenticate_vault(VaultSystem* system,
                              const char* input,
                              char* out_password,
                              size_t pass_size,
                              Vault** out_vault)
{
    int attempts = 0;
    char password[64];

    Vault* v = select_vault(system, input);
    if (!v) {
        printf("Vault not found.\n");
        return -1;
    }

    while (attempts < 3) {
        printf("Password: ");
        read_line(password, sizeof(password));

        if (strcmp(v->password, password) == 0) {
            strncpy(out_password, password, pass_size - 1);
            out_password[pass_size - 1] = '\0';
            *out_vault = v;
            return 0;
        }

        printf("Wrong password (%d/3)\n", attempts + 1);
        attempts++;
    }

    printf("Too many failed attempts.\n");
    printf("Enter override key (or press Enter to cancel): ");
    read_line(password, sizeof(password));

    if (strcmp(password, MASTER_OVERRIDE_KEY) == 0) {
        printf("Override access granted.\n");
        out_password[0] = '\0';   // override mode
        *out_vault = v;
        return 0;
    }

    printf("Access denied.\n");
    return -1;
}

/* ================= Vault Menu ================= */

static void vault_menu(VaultSystem* system, Vault* vault, const char* password)
{
    char input[32];
    int running = 1;

    while (running) {
        printf("\n=== VAULT: %s ===\n", vault->vault_name);
        printf("1. List files\n");
        printf("2. Lock vault\n");
        printf("3. Unlock vault\n");
        printf("4. Add existing file (move)\n");
        printf("5. Create new file in vault\n");
        printf("6. Delete file from vault\n");
        printf("7. Back\n");
        printf("Choice: ");

        read_line(input, sizeof(input));
        int choice = atoi(input);

        switch (choice) {

        case 1:
            fs_scan_vault_dir(vault->vault_name, vault);
            list_files(vault);
            break;

        case 2:
            printf("Lock result: %d\n", lock_vault(vault));
            system_save(system);
            break;

        case 3:
            if (password[0] == '\0') {
                printf("Unlock requires real password.\n");
            } else {
                printf("Unlock result: %d\n",
                       unlock_vault(vault, password));
                system_save(system);
            }
            break;

        /* -------- MOVE FILE INTO VAULT -------- */
        case 4: {
            char src_path[256];
            char filename[64];
            char dst_path[512];

            printf("Full path of file to move: ");
            read_line(src_path, sizeof(src_path));

            const char* base = strrchr(src_path, '/');
#ifdef _WIN32
            if (!base) base = strrchr(src_path, '\\');
#endif
            base = base ? base + 1 : src_path;

            strncpy(filename, base, sizeof(filename) - 1);
            filename[sizeof(filename) - 1] = '\0';

            snprintf(dst_path, sizeof(dst_path),
                     "%s/%s/%s",
                     VAULTS_ROOT,
                     vault->vault_name,
                     filename);

            if (rename(src_path, dst_path) != 0) {
                printf("Failed to move file.\n");
                break;
            }

            add_file_to_vault(vault, filename, filename);
            system_save(system);
            printf("File added to vault.\n");
            break;
        }

        /* -------- CREATE NEW FILE -------- */
        case 5: {
            char filename[64];
            char path[512];

            printf("New filename (example: notes.txt): ");
            read_line(filename, sizeof(filename));

            snprintf(path, sizeof(path),
                     "%s/%s/%s",
                     VAULTS_ROOT,
                     vault->vault_name,
                     filename);

            FILE* fp = fopen(path, "wb");
            if (!fp) {
                printf("Failed to create file.\n");
                break;
            }
            fclose(fp);

            add_file_to_vault(vault, filename, filename);
            system_save(system);
            printf("File created in vault.\n");
            break;
        }

        /* -------- DELETE FILE -------- */
        case 6: {
            char filename[64];

            printf("Filename to delete: ");
            read_line(filename, sizeof(filename));

            VaultFile* vf = find_file(vault, filename);
            if (!vf) {
                printf("File not found.\n");
                break;
            }

            if (strstr(vf->encrypted_path, ".locked")) {
                printf("File is locked. Unlock vault first.\n");
                break;
            }

            if (fs_delete_file_in_vault(vault->vault_name,
                                        vf->encrypted_path) != 0) {
                printf("Filesystem delete failed.\n");
                break;
            }

            size_t index = vf - vault->files;
            for (size_t i = index; i + 1 < vault->file_count; i++) {
                vault->files[i] = vault->files[i + 1];
            }
            vault->file_count--;

            system_save(system);
            printf("File deleted from vault.\n");
            break;
        }

        case 7:
            running = 0;
            break;

        default:
            printf("Invalid option.\n");
        }
    }
}

/* ================= Main Menu ================= */

int main(void)
{
    VaultSystem system;
    system_init(&system);
    system_load(&system);

    char input[64];
    int running = 1;

    while (running) {
        printf("\n==== VAULT SYSTEM ====\n");
        printf("1. List vaults\n");
        printf("2. Create vault\n");
        printf("3. Open vault\n");
        printf("4. Merge vaults\n");
        printf("5. Delete vault\n");
        printf("0. Exit\n");
        printf("Choice: ");

        read_line(input, sizeof(input));
        int choice = atoi(input);

        switch (choice) {

        case 1:
            vault_list(&system);
            break;

        case 2: {
            char name[64], pass[64];
            printf("Vault name: ");
            read_line(name, sizeof(name));
            printf("Password: ");
            read_line(pass, sizeof(pass));

            printf("Result: %d\n",
                   create_vault(&system, name, pass));
            system_save(&system);
            break;
        }

        case 3: {
            char name[64];
            char password[64];
            Vault* v = NULL;

            printf("Vault name or index: ");
            read_line(name, sizeof(name));

            if (authenticate_vault(&system,
                                    name,
                                    password,
                                    sizeof(password),
                                    &v) != 0)
                break;

            vault_menu(&system, v, password);
            break;
        }

        case 4: {
            char src[64], dst[64], pass[64];

            printf("Source vault: ");
            read_line(src, sizeof(src));
            printf("Destination vault: ");
            read_line(dst, sizeof(dst));
            printf("Password: ");
            read_line(pass, sizeof(pass));

            printf("Merge result: %d\n",
                   merge_vaults(&system, src, dst, pass));
            system_save(&system);
            break;
        }

        /* -------- DELETE VAULT -------- */
        case 5: {
            char name[64], password[64], confirm[64];
            Vault* v = NULL;

            printf("Vault name or index to DELETE: ");
            read_line(name, sizeof(name));

            if (authenticate_vault(&system,
                                    name,
                                    password,
                                    sizeof(password),
                                    &v) != 0)
                break;

            for (size_t i = 0; i < v->file_count; i++) {
                if (strstr(v->files[i].encrypted_path, ".locked")) {
                    printf("Vault contains locked files. Unlock first.\n");
                    goto abort_delete;
                }
            }

            printf("Type vault name '%s' to confirm deletion: ",
                   v->vault_name);
            read_line(confirm, sizeof(confirm));

            if (strcmp(confirm, v->vault_name) != 0) {
                printf("Confirmation failed.\n");
                break;
            }

            if (fs_delete_vault_dir_recursive(v->vault_name) != 0) {
                printf("Filesystem deletion failed.\n");
                break;
            }

            printf("Delete result: %d\n",
                   delete_vault(&system,
                                v->vault_name,
                                v->password));

            system_save(&system);
            printf("Vault deleted permanently.\n");
            break;

        abort_delete:
            break;
        }

        case 0:
            running = 0;
            break;

        default:
            printf("Invalid option.\n");
        }
    }

    system_save(&system);
    return 0;
}
