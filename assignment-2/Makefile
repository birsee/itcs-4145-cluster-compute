CXX = g++
OPT ?= -O2
CXXFLAGS = -std=c++17 -fopenmp -pthread -Isrc -MMD -MP
 
empty :=
space := $(empty) $(empty)
 
ifeq ($(OS),Windows_NT)
    FIX_PATH = $(subst /,\,$1)
    MKDIR = if not exist "$(call FIX_PATH,$1)" mkdir "$(call FIX_PATH,$1)"
    RMDIR = rmdir /Q /S $(call FIX_PATH,$1)
    RMFILE = del /Q /F $(call FIX_PATH,$1)
    EXT = .exe
else
    FIX_PATH = $1
    MKDIR = mkdir -p $1
    RMDIR = rm -rf $1
    RMFILE = rm -f $1
    EXT =
endif
 
TARGET = main$(EXT)
SRC_DIR = src
OBJ_ROOT = obj
OBJ_DIR = $(OBJ_ROOT)
 
rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
 
SRCS = $(call rwildcard,$(SRC_DIR),*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
 
all: $(TARGET)
 
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OPT) $(OBJS) -o $@
 
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@$(call MKDIR,$(dir $@))
	$(CXX) $(CXXFLAGS) $(OPT) -c $< -o $@
 
-include $(OBJS:.o=.d)
 
build-%:
	@$(MAKE) --no-print-directory
 
clean:
	-@$(call RMDIR,$(OBJ_ROOT))
	-@$(call RMFILE,main_*$(EXT))
 
.PHONY: all all-levels clean
