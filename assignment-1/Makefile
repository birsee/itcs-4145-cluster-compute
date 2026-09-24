CXX = g++
OPT ?= -O3
CXXFLAGS = -std=c++17 -Isrc -MMD -MP
 
empty :=
space := $(empty) $(empty)
OPT_TAG = $(subst =,_,$(subst $(space),_,$(subst -,,$(OPT))))
 
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
 
# each optimization level gets its own stuff
TARGET = main_$(OPT_TAG)$(EXT)
SRC_DIR = src
OBJ_ROOT = obj
OBJ_DIR = $(OBJ_ROOT)/$(OPT_TAG)
 
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
 
LEVELS = O0 O1 O2 O3
all-levels: $(addprefix build-,$(LEVELS))
build-%:
	@$(MAKE) --no-print-directory OPT=-$*
 
clean:
	-@$(call RMDIR,$(OBJ_ROOT))
	-@$(call RMFILE,main_*$(EXT))
 
.PHONY: all all-levels clean
