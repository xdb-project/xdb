# XDB-Project Build System Configuration
# Standard: GNU Make

# Compiler Settings
CC      := gcc
CFLAGS  := -Wall -Wextra -I./include -I./third_party -pthread -g
# Adding -pthread ensures both compiler and linker use the POSIX threads library

# Directories
BIN_DIR  := bin
DATA_DIR := data
SRC_DIR  := src
TEST_DIR := tests
TP_DIR   := third_party/cJSON

# Source Files
# Third-party dependency: cJSON
THIRD_PARTY_SRC := $(TP_DIR)/cJSON.c

# Core engine source files
CORE_SRC := $(SRC_DIR)/database.c \
            $(SRC_DIR)/query.c \
            $(SRC_DIR)/utils.c \
            $(SRC_DIR)/server.c \
            $(THIRD_PARTY_SRC)

# Source files specifically for unit testing
TEST_SRC := $(SRC_DIR)/database.c \
            $(SRC_DIR)/query.c \
            $(SRC_DIR)/utils.c \
            $(THIRD_PARTY_SRC)

# Build Targets

.PHONY: all setup clean test format

# Default target: prepares directories and builds the main binary
all: setup xdb

# Ensure required directory structure exists
setup:
	@mkdir -p $(BIN_DIR) $(DATA_DIR)
	@touch $(DATA_DIR)/.gitkeep

# Build the primary XDB application binary
xdb: $(CORE_SRC) $(SRC_DIR)/main.c
	$(CC) $(CFLAGS) -o $(BIN_DIR)/xdb $(SRC_DIR)/main.c $(CORE_SRC)

# Build and execute the test suite
# Environment isolation is maintained within the test source files
test: setup
	$(CC) $(CFLAGS) -o $(BIN_DIR)/test_runner \
		$(TEST_DIR)/main_test.c \
		$(TEST_DIR)/test_crud.c \
		$(TEST_DIR)/test_query.c \
		$(TEST_SRC)
	./$(BIN_DIR)/test_runner

# Apply clang-format to internal source and header files
# Excludes third-party libraries to maintain original upstream formatting
format:
	@echo "Applying clang-format to internal source files..."
	@clang-format -i $(SRC_DIR)/*.c include/*.h $(TEST_DIR)/*.c
	@echo "Formatting complete."

# Remove build artifacts and temporary test data
# Note: Production data (e.g., production.json) is preserved for safety
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BIN_DIR)
	rm -f $(DATA_DIR)/test_db.json
	rm -f $(DATA_DIR)/*.tmp
	@echo "Clean operation successful."
