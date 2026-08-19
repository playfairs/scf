CC = gcc
AR = ar
ARCH = $(shell uname -m)
OS = $(shell uname -s)
BUILD_DIR = build
LIBRARY = libscf.a
TEST = $(BUILD_DIR)/scf-test
CFLAGS = -std=c11 -Wall -Wextra -Werror -Iinclude
ASMFLAGS = -x assembler-with-cpp

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

C_OBJECT = $(BUILD_DIR)/src/scf.o
ASM_OBJECT = $(BUILD_DIR)/$(ASM_SOURCE:.asm=.o)
TEST_OBJECT = $(BUILD_DIR)/tests/unit/hash.o

.PHONY: all test clean

all: $(LIBRARY) $(TEST)

$(LIBRARY): $(C_OBJECT) $(ASM_OBJECT)
	$(AR) rcs $@ $^

$(TEST): $(TEST_OBJECT) $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJECT) -L. -lscf

$(BUILD_DIR)/src/%.o: src/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/tests/unit/%.o: tests/unit/%.c
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
