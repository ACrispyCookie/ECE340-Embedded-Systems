# Compiler binary and flags
CC = gcc
CFLAGS = -Wall -O3 -g -mavx2
ifdef TEST
	CFLAGS += -DENABLE_TEST_OUTPUT
endif

#  Directories
SRC_DIR = ./src/
TARGET_DIR = ./target/
OUT_DIR = ./out/

# Default names and final paths
SRC ?= lsal.c
CORRECT_OUT = correct_out
TARGET_PATH = $(TARGET_DIR)$(basename $(SRC) .c)
SRC_PATH   = $(SRC_DIR)$(SRC)

all: $(TARGET_PATH)

$(TARGET_PATH): $(SRC_PATH)
	$(CC) $(CFLAGS) -o $(TARGET_PATH) $(SRC_PATH)

clean-all: clean clean-out

clean-out:
	find $(OUT_DIR) -type f ! -name '$(CORRECT_OUT)' -exec rm -f {} +

clean:
	rm -f $(TARGET_DIR)*
