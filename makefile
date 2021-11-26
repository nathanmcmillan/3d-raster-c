NAME = scroll-and-sigil

SOURCE = $(wildcard src/*.c)
HEADERS = $(wildcard src/*.h)
OBJECTS = $(patsubst src/%.c,objects/%.o,$(SOURCE))
DEPENDENCY = $(patsubst %.o,%.d,$(OBJECTS))

INCLUDE = -Isrc

COMPILER_FLAGS = -Wall -Wextra -Werror -pedantic -std=c11 $(INCLUDE)
LINKER_FLAGS = -lSDL2
LIBS = -lm
PREFIX =
CC = gcc

ifneq ($(shell uname), Linux)
	CC = clang
	COMPILER_FLAGS += -Wno-nullability-extension -Wno-deprecated-declarations
endif

.PHONY: all analysis address valgrind clean test list-source list-objects

all: $(NAME)

-include $(DEPENDENCY)

analysis: PREFIX = scan-build
analysis: all

address: COMPILER_FLAGS += -fsanitize=address
address: all

valgrind: COMPILER_FLAGS += -g
valgrind: all

$(NAME): $(HEADERS) $(OBJECTS)
	$(PREFIX) $(CC) $(OBJECTS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(NAME) $(LIBS)

objects/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) -c $< $(COMPILER_FLAGS) -MMD -o $@

clean:
	rm -f ./$(NAME)
	rm -rf ./objects
