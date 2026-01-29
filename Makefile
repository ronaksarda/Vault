CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I include

SRC = src/main.c \
      src/vault_system.c \
      src/vault.c \
      src/reader.c \
      src/crypto.c \
      src/fs.c 

OUT = vault.exe

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) -Wl,-subsystem,console

clean:
	rm -f $(OUT)
