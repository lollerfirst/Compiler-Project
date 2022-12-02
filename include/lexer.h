#ifndef _LEXER_H_
#define _LEXER_H_

#include <regexparse.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef _DEBUG
#include <assert.h>
#endif
#include <nfa_builder.h>

typedef enum {
	DELIM,
	DEFINE_OP,
	L_ROUNDB,
	R_ROUNDB,
	L_SQUAREB,
	R_SQUAREB,
	END_STMT,
	ARGSTOP,
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
	size_t list_size;
	size_t list_capacity;
	token_t* list;

	nfa_t* nfa_collection;
	size_t nfa_collection_size;
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
