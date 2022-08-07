CC=gcc
SOURCE=./src
BIN=./build
INCLUDE=./include
OBJECT_FILES=$(BIN)/lexer.o $(BIN)/regexparse.o $(BIN)/nfabuilder.o $(BIN)/vector.o
TARGETS=regexparse lexer nfa_builder vector


all:
	make main

clean:
	rm $(BIN)/*

lexer: $(SOURCE)/lexer.c 
	$(CC) $(SOURCE)/lexer.c -I$(INCLUDE) -c -g -o$(BIN)/lexer.o

regexparse: $(SOURCE)/regexparse.c
	$(CC) $(SOURCE)/regexparse.c -I$(INCLUDE) -c -g -o$(BIN)/regexparse.o

nfa_builder: $(SOURCE)/NFA_builder.c
	$(CC) $(SOURCE)/NFA_builder.c -I$(INCLUDE) -c -g -o$(BIN)/nfabuilder.o

vector: $(SOURCE)/vector.c
	$(CC) $(SOURCE)/vector.c -I$(INCLUDE) -c -g -o$(BIN)/vector.o

main: $(TARGETS) $(SOURCE)/main.c 
	$(CC) $(SOURCE)/main.c $(OBJECT_FILES) -I$(INCLUDE) -g -o$(BIN)/main