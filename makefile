# Host compiler
CC = gcc  

CFLAGS = -Wall -O2 -pthread -Iinclude
LDFLAGS = -lwebsockets -lssl -lcrypto -ljansson -lm -lpthread
SRC = src/core/main.c src/core/queue.c src/core/websocket.c \
      src/data/parser.c src/data/ma.c src/data/corr.c   \
      src/system/scheduler.c src/system/cpu_monitor.c src/system/logger.c src/system/timing.c
OBJ      = $(patsubst src/%.c,build/host/%.o,$(SRC))

# Cross-compiler for Raspberry Pi Zero W (ARMv6)
CC_RPI   = arm-linux-gnueabihf-gcc

CFLAGS_RPI  = $(CFLAGS) -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard
LDFLAGS_RPI = -static -lwebsockets -lssl -lcrypto -ljansson -lm -lpthread -L$(HOME)/rpi-sysroot/usr/lib/arm-linux-gnueabihf

OBJ_RPI  = $(patsubst src/%.c,build/rpi/%.o,$(SRC))

$(shell mkdir -p bin logs/raw logs/ma logs/corr test/assets)

# Native build
all: $(OBJ)
	$(CC) $(CFLAGS) -o bin/host $(OBJ) $(LDFLAGS)

build/host/%.o: src/%.c
	@mkdir -p $(@D)  # Auto-create nested build directories (e.g., build/host/core)
	$(CC) $(CFLAGS) -c $< -o $@

# Raspberry Pi build
rpi: $(OBJ_RPI)
	$(CC_RPI) $(CFLAGS_RPI) -o bin/rpi $(OBJ_RPI) $(LDFLAGS_RPI)

build/rpi/%.o: src/%.c
	@mkdir -p $(@D)  # Auto-create nested build directories (e.g., build/rpi/system)
	$(CC_RPI) $(CFLAGS_RPI) -c $< -o $@

clean:
	rm -rf build bin logs test/assets