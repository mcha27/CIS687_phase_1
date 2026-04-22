# Compiler
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Iheaders

# Directories
SRC_DIR = src
OBJ_DIR = bin/obj
BIN_DIR = bin

# Find all .cpp files automatically
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Convert src/file.cpp -> bin/obj/file.o
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Output executable
TARGET = $(BIN_DIR)/main

# Default target
all: $(TARGET)

# Link step
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile step
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Run
run: all
	./$(TARGET)