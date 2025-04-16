CC     = gcc                     # Host compiler
CC_RPI = arm-linux-gnueabihf-gcc # Cross-compiler for Raspberry Pi Zero W (ARMv6)

CFLAGS     = -Wall -O2 -pthread -Iinclude
CFLAGS_RPI = $(CFLAGS) -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard
LDFLAGS    = -lwebsockets -lssl -lcrypto -ljansson -lm 
LDFLAGS_RPI = -static -lwebsockets -lssl -lcrypto -ljansson -lm -L$(HOME)/rpi-sysroot/usr/lib/arm-linux-gnueabihf

SRC = src/main.c src/websocket.c src/queue.c src/parser.c src/logger.c src/ma.c

OBJ     = $(patsubst src/%.c,build/host/%.o,$(SRC))
OBJ_RPI = $(patsubst src/%.c,build/rpi/%.o,$(SRC))
$(shell mkdir -p bin build/host build/rpi logs/raw logs/ma logs/corr)

# native build
all: $(OBJ)
	$(CC) $(CFLAGS) -o bin/host $(OBJ) $(LDFLAGS)

build/host/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Raspberry Pi build
rpi: $(OBJ_RPI)
	$(CC_RPI) $(CFLAGS_RPI) -o bin/rpi $(OBJ_RPI) $(LDFLAGS_RPI)

build/rpi/%.o: src/%.c
	$(CC_RPI) $(CFLAGS_RPI) -c $< -o $@

# extra rules
clean:
	rm -rf build bin logs