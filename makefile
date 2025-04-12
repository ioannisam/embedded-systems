CC     = gcc
CC_RPI = arm-linux-gnueabihf-gcc

CFLAGS     = -Wall -O2 -pthread -lwebsockets -Iinclude
CFLAGS_RPI = $(CFLAGS) -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard

SRC = src/main.c

OBJ_NATIVE = $(patsubst src/%.c,build/arch/%.o,$(SRC))
OBJ_RPI    = $(patsubst src/%.c,build/rpi/%.o,$(SRC))
$(shell mkdir -p bin build/arch build/rpi)

# native build
all: $(OBJ_NATIVE)
	$(CC) $(CFLAGS) -o bin/host $(OBJ_NATIVE)

build/arch/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Raspberry Pi build
rpi: $(OBJ_RPI)
	$(CC_RPI) $(CFLAGS_RPI) -o bin/rpi $(OBJ_RPI)

build/rpi/%.o: src/%.c
	$(CC_RPI) $(CFLAGS_RPI) -c $< -o $@

# extra rules
clean:
	rm -rf build bin