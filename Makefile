CC = gcc
CFLAGS = -Wall -Wextra -g

SRC = $(wildcard src/*.c)
BIN = build/program.out

$(BIN): $(SRC)
	mkdir -p build
	$(CC) $(CFLAGS) -o $(BIN) $(SRC)

clean: 
	rm -rf build