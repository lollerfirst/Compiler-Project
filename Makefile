CC=gcc
SOURCE=./src
BIN=./build
INCLUDE=./include
OBJECT_FILES=$(BIN)/lexer.o $(BIN)/regexparse.o


all:
	make main

clean:
	rm $(BIN)/*

lexer: $(SOURCE)/lexer.c 
	$(CC) $(SOURCE)/lexer.c -I$(INCLUDE) -c -g -o$(BIN)/lexer.o

regexparse: $(SOURCE)/regexparse.c
	$(CC) $(SOURCE)/regexparse.c -I$(INCLUDE) -c -g -o$(BIN)/regexparse.o

main: regexparse lexer $(SOURCE)/main.c 
	$(CC) $(SOURCE)/main.c $(OBJECT_FILES) -I$(INCLUDE) -g -o$(BIN)/main