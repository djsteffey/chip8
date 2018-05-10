# compiler options
CXX = g++
CXX_FLAGS = -std=c++11
CXX_LINK_FLAGS = -lsfml-graphics -lsfml-window -lsfml-system -pthread

# Final binary
BIN = chip8

# Put all auto generated stuff to this build dir.
BUILD_DIR = ./build

# List of all .cpp source files.
CPP = $(wildcard *.cpp)

# All .o files go to build dir.
OBJ = $(CPP:%.cpp=$(BUILD_DIR)/%.o)

# Gcc/Clang will create these .d files containing dependencies.
DEP = $(OBJ:%.o=%.d)

# the binary to build rule
$(BIN):	$(OBJ)
    # Just link all the object files.
	$(CXX) $(CXX_FLAGS) $^ -o $@ $(CXX_LINK_FLAGS)

# Include all .d files
-include $(DEP)

# Build target for every single object file.
# The potential dependency on header files is covered
# by calling `-include $(DEP)`.
$(BUILD_DIR)/%.o : %.cpp
	mkdir -p $(@D)
    # The -MMD flags additionaly creates a .d file with
    # the same name as the .o file.
	$(CXX) $(CXX_FLAGS) -MMD -c $< -o $@

.PHONY : clean
clean :
    # This should remove all generated files.
	-rm $(BIN) $(OBJ) $(DEP)