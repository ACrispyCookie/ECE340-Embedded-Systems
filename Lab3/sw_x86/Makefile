# Compiler binary and flags
CC = gcc
CFLAGS = -Wall -O3 -g -mavx2 -fopenmp
ifdef TEST
	CFLAGS += -DENABLE_TEST_OUTPUT
endif

#  Directories
SRC_DIR = ./src/
TARGET_DIR = ./target/
ASSEMBLY_DIR = ./asm/
OUT_DIR = ./out/

# Default names and final paths
TARGET ?= lsal
CORRECT_OUT = correct_out
TARGET_PATH = $(TARGET_DIR)$(TARGET)
ASSEMBLY_PATH = $(ASSEMBLY_DIR)$(TARGET).s
SRC_PATH   = $(SRC_DIR)$(TARGET).c

all: $(TARGET_PATH)

$(TARGET_PATH): $(SRC_PATH)
	$(CC) $(CFLAGS) -o $(TARGET_PATH) $(SRC_PATH)

assembly: $(SRC_PATH)
	$(CC) $(CFLAGS) -S $(SRC_PATH) -o $(ASSEMBLY_PATH)

clean-all: clean clean-out

clean-out:
	find $(OUT_DIR) -type f ! -name '$(CORRECT_OUT)' -exec rm -f {} +

clean:
	rm -f $(TARGET_DIR)*
