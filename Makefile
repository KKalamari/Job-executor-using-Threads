# Compiler and flags
CC = g++
CFLAGS = -Wall -std=c++11 -Iinclude  # Added -Iinclude to include header files
LDFLAGS = -lpthread

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin
INCLUDE_DIR = include

# Sources and Objects
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))

# Executables
TARGETS = $(BIN_DIR)/comander $(BIN_DIR)/server

# Search Paths
VPATH = $(SRC_DIR)

# Default target
all: $(TARGETS)

# Compile main binaries
$(BIN_DIR)/comander: $(BUILD_DIR)/comander.o
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(BIN_DIR)/server: $(BUILD_DIR)/server.o $(BUILD_DIR)/issueJob.o $(BUILD_DIR)/poll.o \
                   $(BUILD_DIR)/setConcurrency.o $(BUILD_DIR)/stop.o $(BUILD_DIR)/triplet_to_string.o
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Generic rule for compiling object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean generated files
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
