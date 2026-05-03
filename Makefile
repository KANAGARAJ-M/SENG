
CC     = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -Isrc
LDFLAGS= -lm

SRC = src/common.c   \
      src/lexer.c    \
      src/ast.c      \
      src/parser.c   \
      src/value.c    \
      src/env.c      \
      src/packages.c \
      src/interp.c   \
      src/compiler.c \
      src/vm.c       \
      src/main.c

OBJ = $(SRC:.c=.o)

TARGET = seng

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	@echo "=== hello.se ==="
	./$(TARGET) examples/hello.se
	@echo ""
	@echo "=== loops.se ==="
	./$(TARGET) examples/loops.se
	@echo ""
	@echo "=== functions.se ==="
	./$(TARGET) examples/functions.se
	@echo ""
	@echo "=== lists.se ==="
	./$(TARGET) examples/lists.se
	@echo ""
	@echo "=== test_import.se (file import + built-in packages) ==="
	./$(TARGET) examples/test_import.se

clean:
	del /Q src\*.o $(TARGET).exe 2>nul || rm -f src/*.o $(TARGET)
