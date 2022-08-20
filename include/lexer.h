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
	MUL_OP,
	BOOLEAN_OP,
	L_ROUNDB,
	R_ROUNDB,
	L_SQUAREB,
	R_SQUAREB,
	L_CURLYB,
	R_CURLYB,
	END_STMT,
	ARGSTOP,
	LOGIC_NOT,
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

typedef struct{
	size_t len;
	size_t capacity;
	token_t* list;
} toklist_t;

/* scans a string for tokens */
int tokenize(toklist_t*, char*);
/* prints the scanned tokens */
void print_tokens(const toklist_t*);
/* Initializes the tokenizer (builds NFAs with hard-coded regular expressions) */
int tokenizer_init();
void tokenizer_deinit();
const char* tokenizer_typetokstr(toktype_t tktype);

#endif
