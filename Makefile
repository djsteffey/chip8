EXE_NAME	:= chip8
LINK_LIBS	:= -lsfml-graphics -lsfml-window -lsfml-system
MODULES		:= .

CC			:= g++
LD			:= g++


SRC_DIR		:= $(addprefix src/,$(MODULES))
BUILD_DIR	:= $(addprefix build/,$(MODULES))
SRC			:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.cpp))
OBJ			:= $(patsubst src/%.cpp,build/%.o,$(SRC))

vpath %.cpp $(SRC_DIR)

define make-goal
$1/%.o: %.cpp
	$(CC) -c -ggdb $$< -o $$@
endef

.PHONY: all checkdirs clean

all: checkdirs $(EXE_NAME)

$(EXE_NAME): $(OBJ)
	$(LD) $^ -o $@ $(LINK_LIBS)


checkdirs: $(BUILD_DIR)

$(BUILD_DIR):
	@mkdir -p $@

clean:
	@rm -rf build
	@rm -rf $(EXE_NAME)

$(foreach bdir,$(BUILD_DIR),$(eval $(call make-goal,$(bdir))))