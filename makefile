include .env
export

# Host Compiler
CC = gcc

CFLAGS = -Wall -Wextra -Wpedantic -O2 -flto -D_GNU_SOURCE -pthread -Iinclude
LDFLAGS = -lwebsockets -lssl -lcrypto -ljansson -lm -lpthread -latomic

# Raspberry Pi Compiler
CC_RPI  = arm-none-linux-gnueabihf-gcc
SYSROOT = rpi-sysroot

CFLAGS_RPI := -Wall -Wextra -Wpedantic -O2 \
              -marm -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard \
              --sysroot=$(SYSROOT) \
			  -B$(SYSROOT)/usr/lib/arm-linux-gnueabihf/ \
              -isystem $(SYSROOT)/usr/include \
			  -isystem $(SYSROOT)/usr/include/arm-linux-gnueabihf \
              -Iinclude \
              -D_GNU_SOURCE -pthread

LDFLAGS_RPI := --sysroot=$(SYSROOT) -static \
               -L$(SYSROOT)/lib \
			   -L$(SYSROOT)/usr/lib/arm-linux-gnueabihf \
			   -lwebsockets \
			   -lssl \
			   -lcrypto \
			   -ljansson \
			   -lz \
			   -latomic \
			   -lpthread \
			   -lm \
			   -lcap

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

host: dirs $(OBJ)
	$(CC) $(CFLAGS) -o bin/host $(OBJ) $(LDFLAGS)

rpi: dirs $(OBJ_RPI)
	$(CC_RPI) $(CFLAGS_RPI) -o bin/rpi $(OBJ_RPI) $(LDFLAGS_RPI)

build/host/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

build/rpi/%.o: src/%.c
	$(CC_RPI) $(CFLAGS_RPI) -c $< -o $@

deploy: rpi
	scp bin/rpi ioannis@$(IP):$(DEST)

clean:
	rm -rf build bin logs test/assets

.PHONY: all dirs host rpi deploy clean