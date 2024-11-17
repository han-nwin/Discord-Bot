# Variables
CXX = g++
CXXFLAGS = -Wall -Wextra -DDEBUG -g -std=c++20
LDFLAGS = -lcpr -lcurl
SRC_DIR = src
OBJ_DIR = build
BIN_DIR = bin
TARGET = $(BIN_DIR)/main

# CPU Core Detection (Cross-Platform)
CPUS = $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)
MAKEFLAGS += -j$(CPUS)

# File lists
CPP_SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
CPP_OBJECTS = $(CPP_SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Default target
all: $(TARGET)

# Link object files to create the final binary
$(TARGET): $(CPP_OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CPP_OBJECTS) -o $@ $(LDFLAGS)

# Compile C++ source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -rf $(OBJ_DIR)/* $(BIN_DIR)/* 

# Clean json files
cleanjson:
	rm -f *.json

# Run the program
run: all
	./$(TARGET) $(ARGS)

# Phony targets
.PHONY: all clean cleanjson run
