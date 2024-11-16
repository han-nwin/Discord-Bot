# Variables
CXX = g++
CXXFLAGS = -Iinclude -I/opt/homebrew/include -Wall -Wextra -DDEBUG -g -std=c++20
LDFLAGS = -L/opt/homebrew/lib -lcpr -lcurl
SRC_DIR = src
OBJ_DIR = build
BIN_DIR = bin
TARGET = $(BIN_DIR)/main

# File lists
CPP_SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
CPP_OBJECTS = $(CPP_SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Default target
all: $(TARGET)

# Link object files to create the final binary
$(TARGET): $(CPP_OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CPP_OBJECTS) -o $@ $(LDFLAGS)  # Added $(LDFLAGS) for linking libraries

# Compile C++ source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -rf $(OBJ_DIR)/* $(BIN_DIR)/*

# Run the program
run: all
	./$(TARGET) $(ARGS)

# Phony targets
.PHONY: all clean run
