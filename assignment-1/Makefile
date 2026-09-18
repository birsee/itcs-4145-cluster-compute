ifeq ($(OS),Windows_NT)
    FIX_PATH = $(subst /,\,$1)
    MKDIR= if not exist "$(call FIX_PATH,$1)" mkdir "$(call FIX_PATH,$1)"
    RMDIR = rmdir /Q /S
    TARGET = main.exe
else
    FIX_PATH = $1
    MKDIR = mkdir -p $1
    RMDIR = rm -rf $1
    TARGET = main
endif

CXX = g++
CXXFLAGS ?= -std=c++17 -O3

SRC_DIR = src
OBJ_DIR = obj

rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SRCS = $(call rwildcard,$(SRC_DIR),*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@$(call MKDIR,$(dir $@))
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@$(call RMDIR,$(OBJ_DIR))
	@ifeq ($(OS),Windows_NT)
		@if exist $(TARGET) del /F /Q $(TARGET)
	@else
		@rm -f $(TARGET)
	@endif
.PHONY: all clean