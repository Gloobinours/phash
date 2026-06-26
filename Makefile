# Compiler settings
CC       := gcc
CFLAGS   := -Wall -Wextra -O3 -std=c11 -Iinclude -fPIC
CFLAGS   += -D_POSIX_C_SOURCE=200809L

# Discovery for libjpeg-turbo dependencies via pkg-config
PKG_CONF := $(shell pkg-config --exists turbojpeg && echo yes)

ifeq ($(PKG_CONF),yes)
    CFLAGS  += $(shell pkg-config --cflags turbojpeg)
    LDFLAGS += $(shell pkg-config --libs turbojpeg) -lm
else
    LDFLAGS += -lturbojpeg -lm
endif

# Directory layout
SRC_DIR  := src
OBJ_DIR  := obj
BIN_DIR  := bin
ASSET_DIR:= assets
SCRIPT_DIR:= scripts

# Targets
TARGET_EXE := $(BIN_DIR)/img_app
TARGET_SO  := $(BIN_DIR)/libphash.so
TARGET_LUT := $(BIN_DIR)/dct_lut.bin

# Automatically locate all source files and map them to object files
SRCS     := $(wildcard $(SRC_DIR)/*.c)
OBJS     := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
SO_OBJS  := $(filter-out $(OBJ_DIR)/main.o, $(OBJS))

# Default build rule
.PHONY: all
all: $(TARGET_EXE) $(TARGET_SO) $(TARGET_LUT)

# Link standalone executable
$(TARGET_EXE): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Link shared object library
$(TARGET_SO): $(SO_OBJS) | $(BIN_DIR)
	$(CC) -shared $(SO_OBJS) -o $@ $(LDFLAGS)

# Rule to calculate the master binary if it does not exist using Python
# Added | $(ASSET_DIR) to ensure the directory is present before running the script
$(ASSET_DIR)/dct_lut.bin: $(SCRIPT_DIR)/generate_dct_lut.py | $(ASSET_DIR)
	python3 $(SCRIPT_DIR)/generate_dct_lut.py

# Copy the generated lookup table to the bin directory
$(TARGET_LUT): $(ASSET_DIR)/dct_lut.bin | $(BIN_DIR)
	cp $< $@

# Compile C files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create required directories
$(BIN_DIR) $(OBJ_DIR) $(ASSET_DIR):
	mkdir -p $@

# Clean rule
.PHONY: clean
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(ASSET_DIR)
	@echo "Clean completed successfully."
