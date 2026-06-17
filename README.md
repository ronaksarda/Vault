# 🛡️ Vault — Filesystem-Based File Locker (C)

Vault is a lightweight, interactive command-line application written in C that manages and encrypts files directly on your operating system's filesystem.

It groups files into encrypted directories called **vaults**. You can easily lock or unlock entire vaults with a master password, keeping your files secure through an integrated XOR-based cipher while treating the filesystem as the authoritative source of truth.

---

## ✨ Features

- **Vault Management:** Create, open, merge, and delete vaults.
- **Batch Lock/Unlock:** Lock or unlock all files inside a vault in a single operation.
- **File Management:** Move existing files into a vault, create new ones, or delete them safely.
- **Basic Encryption:** Built-in XOR stream cipher encryption directly derived from your vault password.
- **Secure File System Syncing:** Metadata mirrors the real-time filesystem state, avoiding "ghost" files.
- **Pure C Implementation:** Zero external dependencies, leveraging standard C libraries.

---

## Build and Installation

This project requires `gcc` (GNU Compiler Collection).

### Windows / Linux / macOS
You can compile the program manually using `gcc`:
```bash
mkdir -p bin
gcc -Wall -Wextra -I include src/main.c src/vault_system.c src/vault.c src/reader.c src/crypto.c src/fs.c -o bin/vault
```

---

## Usage

Launch the interactive CLI by running the executable:

```bash
# Windows
bin\vault.exe

# Linux / macOS
./bin/vault
```

### Main Menu
From the main menu, you can:
1. **List vaults:** View all currently available vaults.
2. **Create vault:** Setup a new vault directory and assign a master password.
3. **Open vault:** Enter a vault using your password to manage its files.
4. **Merge vaults:** Move all unlocked files from one vault into another.
5. **Delete vault:** Permanently delete a vault and all its files (requires password confirmation).

### Inside a Vault
Once a vault is opened, you can manage the internal files:
- **Lock/Unlock vault:** Encrypts or decrypts every file in the vault, appending/removing the `.locked` extension.
- **Add existing file:** Safely move a file from outside the vault into the encrypted directory.
- **Create new file:** Quickly generate an empty file directly inside the vault.

---

## Testing

The project includes an isolated unit testing suite for the core vault data structures.

To run the tests:
```bash
gcc -Wall -Wextra -I include src/test_vault.c src/vault.c -o bin/test_vault
./bin/test_vault
```

---

## Architecture & Semantics

### Storage Model
- Vaults are stored locally as subdirectories inside a `Vaults_folder/` directory.
- Global metadata is saved to `data/vault.dat`. The filesystem always acts as the ultimate source of truth.

### Locking & Encryption
- **Fully Atomic States:** A vault is either entirely locked or entirely unlocked. Partial or mixed states will reject subsequent lock/unlock requests.
- **In-place Transformation:** The XOR cipher transforms the binary content of the files in-place and renames them to explicitly indicate their `.locked` status.
- *Note:* The XOR encryption is designed as a lightweight deterrent and structural exercise, not for military-grade cryptographic security. 

---

## Limitations & Notes

- **Basic Cryptography:** Uses a repeating-key XOR cipher. It does not utilize AES, PBKDF2 hashing, or CSPRNG, making it unsuitable for highly sensitive information.
- **Non-transactional:** File operations are applied sequentially. In the rare event of a sudden power loss during a lock/unlock sequence, manual recovery may be necessary.
- **Single-User Environment:** Concurrency/multithreading locks are not implemented.

---

## License

This project is open-source and free to use. Designed as an entry-level systems architecture project in C.
