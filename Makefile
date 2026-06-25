# Compiler settings
CC       := gcc
CFLAGS   := -Wall -Wextra -O2 -std=c11 -Iinclude
CFLAGS += -D_POSIX_C_SOURCE=200809L

# Discovery for libjpeg-turbo dependencies via pkg-config
# Falls back to default flags if pkg-config is missing
PKG_CONF := $(shell pkg-config --exists turbojpeg && echo yes)

ifeq ($(PKG_CONF),yes)
    CFLAGS  += $(shell pkg-config --cflags turbojpeg)
    # Added -lm to the end of the libraries flag
    LDFLAGS += $(shell pkg-config --libs turbojpeg) -lm
else
    # Fallback paths for standard Linux installs if pkg-config fails
    LDFLAGS += -lturbojpeg -lm
endif

# Directory layout
SRC_DIR  := src
OBJ_DIR  := obj
BIN_DIR  := bin

# Name of the output executable binary
TARGET   := $(BIN_DIR)/img_app

# Automatically locate all source files and map them to object files
SRCS     := $(wildcard $(SRC_DIR)/*.c)
OBJS     := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Default build rule
.PHONY: all
all: $(TARGET)

# Link the executable binary from object modules
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Compile C source code files into temporary object binaries
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create required directories if they do not exist
$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

# Clean rule to wipe away binary distributions for a fresh compile build
.PHONY: clean
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Clean completed successfully."

