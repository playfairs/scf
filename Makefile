CC = gcc
AR = ar
ARCH = $(shell uname -m)
OS = $(shell uname -s)
BUILD_DIR = build
LIBRARY = libscf.a
TEST = $(BUILD_DIR)/scf-test
TOOL = $(BUILD_DIR)/scf
CFLAGS = -std=c11 -Wall -Wextra -Werror -Iinclude
ASMFLAGS = -x assembler-with-cpp

SRC = \
	src/scf.c \
	src/memory.c \
	src/hash.c \
	src/cipher.c \
	src/kdf.c \
	src/keys.c \
	src/format.c \
	src/provider.c \
	src/kat.c \
	src/sha256.c

TEST_SRC = \
	tests/unit/scf.c \
	tests/unit/memory.c \
	tests/unit/hash.c \
	tests/unit/cipher.c \
	tests/unit/kdf.c \
	tests/unit/keys.c \
	tests/unit/format.c \
	tests/unit/provider.c \
	tests/unit/kat.c \
	tests/unit/sha256.c \
	tests/integration/scarlett.c \
	tests/vectors/scarlett.c

ASM_SRC = \
	$(ASM_SOURCE) \
	asm/$(if $(filter x86_64,$(ARCH)),x86_64,aarch64)/auth.asm \
	asm/$(if $(filter x86_64,$(ARCH)),x86_64,aarch64)/cipher.asm \
	asm/$(if $(filter x86_64,$(ARCH)),x86_64,aarch64)/hash.asm \
	asm/$(if $(filter x86_64,$(ARCH)),x86_64,aarch64)/memory.asm

ifeq ($(ARCH),x86_64)
ASM_SOURCE = asm/x86_64/test.asm
else ifeq ($(ARCH),aarch64)
ASM_SOURCE = asm/aarch64/test.asm
else ifeq ($(ARCH),arm64)
ASM_SOURCE = asm/aarch64/test.asm
else
$(error Unsupported architecture: $(ARCH))
endif

ifeq ($(OS),Darwin)
ASMFLAGS += -DSCF_DARWIN
endif

C_OBJECTS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))
ASM_OBJECTS = $(patsubst %.asm,$(BUILD_DIR)/%.o,$(ASM_SRC))
TEST_OBJECTS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(TEST_SRC))
TOOL_OBJECT = $(BUILD_DIR)/tools/scf/main.o

.PHONY: all test clean

all: $(LIBRARY) $(TEST) $(TOOL)

$(LIBRARY): $(C_OBJECTS) $(ASM_OBJECTS)
	$(AR) rcs $@ $^

$(TEST): $(TEST_OBJECTS) $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJECTS) -L. -lscf

$(TOOL): $(TOOL_OBJECT) $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $(TOOL_OBJECT) -L. -lscf

$(BUILD_DIR)/src/%.o: src/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/tests/unit/%.o: tests/unit/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/tests/integration/%.o: tests/integration/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/tests/vectors/%.o: tests/vectors/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/tools/scf/%.o: tools/scf/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/asm/x86_64/%.o: asm/x86_64/%.asm
	mkdir -p $(@D)
	$(CC) $(ASMFLAGS) -c $< -o $@

$(BUILD_DIR)/asm/aarch64/%.o: asm/aarch64/%.asm
	mkdir -p $(@D)
	$(CC) $(ASMFLAGS) -c $< -o $@

test: $(TEST)
	./$(TEST)

clean:
	rm -rf $(BUILD_DIR) $(LIBRARY)
