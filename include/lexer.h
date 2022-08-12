#ifndef _LEXLER_H_
#define _LEXLER_H_

#include <regexparse.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef _DEBUG
#include <assert.h>
#endif
#include <NFA_builder.h>

typedef enum {
	DELIM,
	ASSIGN_OP,
	ALGEBRAIC_OP,
	BOOLEAN_OP,
	IF,
	WHILE,
	BREAK,
	ELSE,
	RETURN,
	TYPE,
	NUMBER,
	NAME,
	STRING,
	CHAR,
	NOTOK
} toktype_t;


typedef struct{
	toktype_t tt;
	char* tk;
} token_t;

token_t* tokenize(char* buffer);
void print_tokens(token_t*);
const char* typetokstr(toktype_t);
void tokenizer_init();

#endif
