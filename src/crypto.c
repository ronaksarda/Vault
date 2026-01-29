#include "crypto.h"
#include <stdio.h>
#include <string.h>

#define BUF_SIZE 4096

static int xor_transform_file(const char* filepath,
                              const char* password)
{
    if (!filepath || !password || password[0] == '\0')
        return -1;

    FILE* fp = fopen(filepath, "rb+");
    if (!fp)
        return -2;

    unsigned char buffer[BUF_SIZE];
    size_t pass_len = strlen(password);
    size_t pass_index = 0;

    size_t bytes_read;
    long pos = 0;

    while ((bytes_read = fread(buffer, 1, BUF_SIZE, fp)) > 0) {

        for (size_t i = 0; i < bytes_read; i++) {
            buffer[i] ^= password[pass_index % pass_len];
            pass_index++;
        }

        fseek(fp, pos, SEEK_SET);
        fwrite(buffer, 1, bytes_read, fp);

        pos += bytes_read;
        fseek(fp, pos, SEEK_SET);
    }

    fclose(fp);
    return 0;
}

int crypto_encrypt_file(const char* filepath,
                        const char* password)
{
    return xor_transform_file(filepath, password);
}

int crypto_decrypt_file(const char* filepath,
                        const char* password)
{
    return xor_transform_file(filepath, password);
}
