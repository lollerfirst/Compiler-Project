CC=gcc
SOURCE=./src
BIN=./build
INCLUDE=./include
OBJECT_FILES=$(BIN)/lexer.o $(BIN)/regexparse.o $(BIN)/nfabuilder.o $(BIN)/parser.o
TARGETS=regexparse lexer nfa_builder parser
ifeq ($(OPT),debug)
DEBUG=-g -D'_DEBUG'
else
DEBUG=
endif

FLAGS=-Wall -Wextra $(DEBUG)


main: $(TARGETS) $(SOURCE)/main.c 
	$(CC) $(SOURCE)/main.c $(OBJECT_FILES) -I$(INCLUDE) $(FLAGS) -o$(BIN)/main

lexer: $(SOURCE)/lexer.c 
	$(CC) $(SOURCE)/lexer.c -I$(INCLUDE) -c $(FLAGS) -o$(BIN)/lexer.o

regexparse: $(SOURCE)/regexparse.c
	$(CC) $(SOURCE)/regexparse.c -I$(INCLUDE) -c $(FLAGS) -o$(BIN)/regexparse.o

nfa_builder: $(SOURCE)/NFA_builder.c
	$(CC) $(SOURCE)/NFA_builder.c -I$(INCLUDE) -c $(FLAGS) -o$(BIN)/nfabuilder.o

parser: $(SOURCE)/parser.c
	$(CC) $(SOURCE)/parser.c -I$(INCLUDE) -c $(FLAGS) -o$(BIN)/parser.o

all:
	make main

clean:
	rm $(BIN)/*