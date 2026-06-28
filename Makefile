# Compiler settings
CC       := gcc
# Added explicitly defined include search paths for core and utils directories
CFLAGS   := -Wall -Wextra -g -O3 -std=c11 -Iinclude -Iinclude/core -Iinclude/utils -fPIC
CFLAGS   += -D_POSIX_C_SOURCE=200809L
LDFLAGS  :=

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

# This automatically grabs all .c files inside src/, src/core/, and src/utils/
SRCS     := $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/core/*.c) $(wildcard $(SRC_DIR)/utils/*.c)

# This converts any source file into a flat object file path inside obj/
# Example: src/core/phash.c becomes obj/phash.o
OBJS     := $(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(SRCS)))

# Prevent main.o from accidentally entering the shared object library (.so)
SO_OBJS  := $(filter-out $(OBJ_DIR)/main.o, $(OBJS))

# --- LOGGING ENGINE MODES ---
.PHONY: all
all: $(TARGET_EXE) $(TARGET_SO) $(TARGET_LUT)

# Release build rule (Strips out all logging statements completely)
.PHONY: release
release: CFLAGS += -DRELEASE_MODE
release: all

# Link standalone executable
$(TARGET_EXE): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Link shared object library
$(TARGET_SO): $(SO_OBJS) | $(BIN_DIR)
	$(CC) -shared $(SO_OBJS) -o $@ $(LDFLAGS)

# Rule to calculate the master binary if it does not exist using Python
$(ASSET_DIR)/dct_lut.bin: $(SCRIPT_DIR)/generate_dct_lut.py | $(ASSET_DIR)
	python3 $(SCRIPT_DIR)/generate_dct_lut.py

# Copy the generated lookup table to the bin directory
$(TARGET_LUT): $(ASSET_DIR)/dct_lut.bin | $(BIN_DIR)
	cp $< $@

# VPATH tells Make where to look for .c prerequisites if they aren't in the root directory
vpath %.c $(SRC_DIR) $(SRC_DIR)/core $(SRC_DIR)/utils

# Compile step now safely catches nested source files and generates flat .o outputs
$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create required directories
$(BIN_DIR) $(OBJ_DIR) $(ASSET_DIR):
	mkdir -p $@

# Clean rule
.PHONY: clean
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(ASSET_DIR)
	@echo "Clean completed successfully."
