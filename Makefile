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
OUTPUT_DIR = output
TEMP_DIR = temp

# =========================
# Output targets
# =========================
TARGET = $(BIN_DIR)/main

MAPPER_DLL_TARGET = $(DLL_DIR)/mapper.$(LIB_EXT)
REDUCER_DLL_TARGET = $(DLL_DIR)/reducer.$(LIB_EXT)

# =========================
# Source files
# =========================
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# =========================
# MAIN EXECUTABLE OBJECTS
# =========================
MAIN_OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/main_%.o, $(SRCS))

# =========================
# MAPPER DLL OBJECTS
# =========================
MAPPER_DLL_OBJS = \
    $(OBJ_DIR)/dll_mapper.o \
    $(OBJ_DIR)/dll_mapper_dll.o \
    $(OBJ_DIR)/dll_mapper_file_manager.o

# =========================
# REDUCER DLL OBJECTS
# =========================
REDUCER_DLL_OBJS = \
    $(OBJ_DIR)/dll_reducer.o \
    $(OBJ_DIR)/dll_reducer_dll.o \
    $(OBJ_DIR)/dll_reducer_file_manager.o

# =========================
# Default target
# =========================
all: $(TARGET) $(MAPPER_DLL_TARGET) $(REDUCER_DLL_TARGET)

# =========================
# MAIN EXECUTABLE
# =========================
$(TARGET): $(MAIN_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile main executable objects (NO PIC)
$(OBJ_DIR)/main_%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# =========================================================
# MAPPER DLL
# =========================================================
$(MAPPER_DLL_TARGET): $(MAPPER_DLL_OBJS)
	@mkdir -p $(DLL_DIR)
	$(CXX) -shared -o $@ $^

$(OBJ_DIR)/dll_mapper.o: $(SRC_DIR)/mapper.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -fPIC -DMAPPER_DLL_EXPORTS -c $< -o $@

$(OBJ_DIR)/dll_mapper_dll.o: $(SRC_DIR)/mapper_dll.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -fPIC -DMAPPER_DLL_EXPORTS -c $< -o $@

$(OBJ_DIR)/dll_mapper_file_manager.o: $(SRC_DIR)/file_manager.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -fPIC -DMAPPER_DLL_EXPORTS -c $< -o $@

# =========================================================
# REDUCER DLL
# =========================================================
$(REDUCER_DLL_TARGET): $(REDUCER_DLL_OBJS)
	@mkdir -p $(DLL_DIR)
	$(CXX) -shared -o $@ $^

$(OBJ_DIR)/dll_reducer.o: $(SRC_DIR)/reducer.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -fPIC -DREDUCER_DLL_EXPORTS -c $< -o $@

$(OBJ_DIR)/dll_reducer_dll.o: $(SRC_DIR)/reducer_dll.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -fPIC -DREDUCER_DLL_EXPORTS -c $< -o $@

$(OBJ_DIR)/dll_reducer_file_manager.o: $(SRC_DIR)/file_manager.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -fPIC -DREDUCER_DLL_EXPORTS -c $< -o $@

# =========================
# CLEAN
# =========================
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(OUTPUT_DIR) $(TEMP_DIR)

# =========================
# RUN
# =========================
run: all
	./$(TARGET)