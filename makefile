CC = gcc
CFLAGS = -Wall -I./include -pthread -lm

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
BIN_DIR = bin
RESULT_DIR = test/results

TARGET = $(BIN_DIR)/pc

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

all: directories $(TARGET)

directories:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BIN_DIR)
	mkdir -p $(RESULT_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ -pthread -lm

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) $(RESULT_DIR)

.PHONY: all clean directories