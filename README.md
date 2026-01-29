# Vault — Filesystem-Based File Locker (C)

Vault is a command-line program written in C that manages files directly on the operating system’s filesystem.

Files are grouped into directories called vaults. All files inside a vault can be locked or unlocked together using a password. Files always remain normal OS files on disk.

---

## High-Level Behavior

- Vaults are directories on disk
- Files inside a vault are real files, not stored internally
- Locking transforms file contents in place and renames files
- Unlocking reverses the transformation and restores filenames
- The filesystem is treated as the source of truth

Metadata mirrors filesystem state and does not override it.

---

## Vault Model

A vault consists of:
- a directory on disk
- a name
- a password
- metadata describing files in the vault

Vaults are stored under a fixed root directory.

---

## File States

Files inside a vault can be in exactly one of two states:

Unlocked:
- file contents are readable
- filename does not end with `.locked`

Locked:
- file contents are transformed
- filename ends with `.locked`

Intermediate or mixed states are not allowed.

---

## Lock Semantics

Locking applies to the entire vault.

When locking a vault:
1. Each file’s contents are transformed using XOR encryption
2. Each file is renamed by appending `.locked`
3. Metadata is updated only after filesystem success

Locking is rejected if:
- any file is already locked
- the vault is in a mixed state
- a filesystem operation fails

Partial locking is not allowed.

---

## Unlock Semantics

Unlocking applies to the entire vault.

When unlocking a vault:
1. The password is verified
2. All files must be in locked state
3. File contents are restored
4. Filenames are restored
5. Metadata is updated after filesystem success

Unlocking is rejected if:
- the password is incorrect
- any file is not locked
- a filesystem operation fails

Partial unlocking is not allowed.

---

## Invariants

The following invariants are enforced:

- a vault is either fully locked or fully unlocked
- locked vaults contain only `.locked` files
- unlocked vaults contain no `.locked` files
- filesystem operations complete before metadata updates
- metadata never references missing files

If an invariant would be violated, the operation fails.

---

## Supported Operations

### Vault Operations
- create a vault
- list vaults
- open a vault by name or index
- delete a vault (password and confirmation required)
- merge two vaults

### File Operations
- move an existing file into a vault
- create a new file inside a vault
- list files in a vault
- delete a file from a vault
- lock a vault
- unlock a vault

All operations are performed through an interactive CLI.

---

## Access Control

Each vault is protected by a password.

The password is required to:
- unlock a vault
- delete a vault
- merge vaults

An override key exists at the CLI level for recovery.

Passwords are stored in plain text.

---

## Merge Semantics

Merging moves all files from one vault into another.

Rules:
- both vaults must be unlocked
- both vaults must use the same password
- filename collisions are rejected
- files are moved on disk before metadata updates
- source vault directory is deleted after success

Merge operations are non-atomic. Partial failure may require manual cleanup.

---

## Encryption

- XOR encryption is used
- the key is derived directly from the password

This encryption is reversible and not intended to provide real security.

---

## Metadata

- metadata is stored on disk
- metadata contains:
  - vault name
  - password
  - file list
- metadata mirrors filesystem state
- metadata can be reconstructed by scanning vault directories

Filesystem state is authoritative.

---

## Persistence

- files exist independently of metadata
- restarting the program does not delete files
- no journaling or transactional rollback is implemented

---

## Build

gcc -Wall -Wextra -pedantic -o vault main.c vault_system.c fs.c crypto.c

---

## Run

./vault.exe

The program runs as an interactive CLI.

---

## .gitignore

Runtime data and binaries should be ignored.

Typical entries:

vault  
vault.exe  
*.o  
vaults/  

---

## Limitations

- no strong cryptography
- no password hashing
- no transactional rollback
- no nested directories
- single-user only
- no concurrency handling
