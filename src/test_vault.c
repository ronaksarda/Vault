#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "vault.h"

// Simple test runner macros
#define RUN_TEST(test_func) \
    do { \
        printf("Running %s... ", #test_func); \
        test_func(); \
        printf("PASSED\n"); \
    } while(0)

void test_add_file_to_vault() {
    Vault v;
    memset(&v, 0, sizeof(v));
    v.file_count = 0;

    // Test successful addition
    int res = add_file_to_vault(&v, "secret.txt", "secret.txt");
    assert(res == 0);
    assert(v.file_count == 1);
    assert(strcmp(v.files[0].filename, "secret.txt") == 0);

    // Test duplicate addition
    res = add_file_to_vault(&v, "secret.txt", "secret.txt");
    assert(res == -3); // Duplicate check should fail
    assert(v.file_count == 1);

    // Test capacity limit
    v.file_count = MAX_FILES;
    res = add_file_to_vault(&v, "new.txt", "new.txt");
    assert(res == -2); // Capacity full
}

void test_remove_file_from_vault() {
    Vault v;
    memset(&v, 0, sizeof(v));
    v.file_count = 0;

    add_file_to_vault(&v, "file1.txt", "file1.txt");
    add_file_to_vault(&v, "file2.txt", "file2.txt");
    add_file_to_vault(&v, "file3.txt", "file3.txt");

    assert(v.file_count == 3);

    // Test removing middle file
    int res = remove_file_from_vault(&v, "file2.txt");
    assert(res == 0);
    assert(v.file_count == 2);
    assert(strcmp(v.files[0].filename, "file1.txt") == 0);
    assert(strcmp(v.files[1].filename, "file3.txt") == 0);

    // Test removing non-existent file
    res = remove_file_from_vault(&v, "notfound.txt");
    assert(res == -2);
    assert(v.file_count == 2);
}

void test_find_file() {
    Vault v;
    memset(&v, 0, sizeof(v));
    v.file_count = 0;

    add_file_to_vault(&v, "target.txt", "target.txt.locked");

    VaultFile* found = find_file(&v, "target.txt");
    assert(found != NULL);
    assert(strcmp(found->encrypted_path, "target.txt.locked") == 0);

    VaultFile* not_found = find_file(&v, "ghost.txt");
    assert(not_found == NULL);
}

int main() {
    printf("=== Running Vault Unit Tests ===\n");
    RUN_TEST(test_add_file_to_vault);
    RUN_TEST(test_remove_file_from_vault);
    RUN_TEST(test_find_file);
    printf("=== All Tests Passed! ===\n");
    return 0;
}
