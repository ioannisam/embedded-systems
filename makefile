# Host Compiler
CC = gcc

CFLAGS = -Wall -Wextra -Wpedantic -O2 -flto -D_GNU_SOURCE -pthread -Iinclude
LDFLAGS = -lwebsockets -lssl -lcrypto -ljansson -lm -lpthread -latomic

# Raspberry Pi Compiler
CC_RPI = arm-linux-gnueabihf-gcc

CFLAGS_RPI = $(CFLAGS) -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard
LDFLAGS_RPI = $(LDFLAGS) -static -L$(HOME)/rpi-sysroot/usr/lib/arm-linux-gnueabihf

# File structure
SRC = src/core/main.c src/core/queue.c src/core/websocket.c \
	src/data/parser.c src/data/ma.c src/data/corr.c \
	src/system/scheduler.c src/system/cpu_monitor.c src/system/logger.c src/system/timing.c
OBJ = $(patsubst src/%.c,build/host/%.o,$(SRC))
OBJ_RPI = $(patsubst src/%.c,build/rpi/%.o,$(SRC))

all: dirs host

dirs:
	@mkdir -p bin logs/raw logs/ma logs/corr test/assets \
		$(patsubst src/%,build/host/%,$(dir $(SRC))) \
		$(patsubst src/%,build/rpi/%,$(dir $(SRC)))

host: $(OBJ)
	$(CC) $(CFLAGS) -o bin/host $^ $(LDFLAGS)

rpi: $(OBJ_RPI)
	$(CC_RPI) $(CFLAGS_RPI) -o bin/rpi $^ $(LDFLAGS_RPI)

build/host/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

build/rpi/%.o: src/%.c
	$(CC_RPI) $(CFLAGS_RPI) -c $< -o $@

clean:
	rm -rf build bin logs test/assets

.PHONY: all dirs host rpi clean