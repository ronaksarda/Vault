#ifndef CRYPTO_H
#define CRYPTO_H

// Encrypt file contents in-place using password
int crypto_encrypt_file(const char* filepath,const char* password);

// Decrypt file contents in-place using password
int crypto_decrypt_file(const char* filepath,const char* password);

#endif // CRYPTO_H
