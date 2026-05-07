UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
    LIB_EXT = so
else
    LIB_EXT = dll
endif

# =========================
# Compiler
# =========================
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Iheaders

# =========================
# Directories
# =========================
SRC_DIR = src
OBJ_DIR = bin/obj
BIN_DIR = bin
DLL_DIR = bin/dll

# =========================
# Output targets
# =========================
TARGET = $(BIN_DIR)/main
DLL_TARGET = $(DLL_DIR)/mapper.$(LIB_EXT)

# =========================
# Source files
# =========================
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Main executable objects (NO PIC)
MAIN_OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/main_%.o, $(SRCS))

# DLL-specific sources
DLL_SRCS = mapper.cpp mapper_dll.cpp file_manager.cpp

DLL_OBJS = \
    $(OBJ_DIR)/dll_mapper.o \
    $(OBJ_DIR)/dll_mapper_dll.o \
    $(OBJ_DIR)/dll_file_manager.o

# =========================
# Default target
# =========================
all: $(TARGET) $(DLL_TARGET)

# =========================
# MAIN EXECUTABLE
# =========================
$(TARGET): $(MAIN_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile main objects (NO -fPIC)
$(OBJ_DIR)/main_%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# =========================
# DLL BUILD
# =========================
$(DLL_TARGET): $(DLL_OBJS)
	@mkdir -p $(DLL_DIR)
	$(CXX) -shared -o $@ $^

# Compile DLL objects (PIC REQUIRED)
$(OBJ_DIR)/dll_mapper.o: src/mapper.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@

$(OBJ_DIR)/dll_mapper_dll.o: src/mapper_dll.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@

$(OBJ_DIR)/dll_file_manager.o: src/file_manager.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@

# =========================
# CLEAN
# =========================
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# =========================
# RUN
# =========================
run: all
	./$(TARGET)