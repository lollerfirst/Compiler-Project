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
	L_ROUNDB,
	R_ROUNDB,
	L_SQUAREB,
	R_SQUAREB,
	L_CURLYB,
	R_CURLYB,
	END_STMT,
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

inline const char* typetokstr(toktype_t tktype){
	switch (tktype){
		case DELIM:
			return "delimiter";
		case ASSIGN_OP:
			return "assign-op";
		case ALGEBRAIC_OP:
			return "algebraic-op";
		case BOOLEAN_OP:
			return "boolean-op";
		case L_ROUNDB:
			return "left-roundbracket";
		case R_ROUNDB:
			return "right-roundbracket";
		case L_SQUAREB:
			return "left-squarebracket";
		case R_SQUAREB:
			return "right-squarebracket";
		case L_CURLYB:
			return "left-curlybracket";
		case R_CURLYB:
			return "right-curlybracket";
		case END_STMT:
			return "end-statement";
		case IF:
			return "if";
		case WHILE:
			return "while";
		case BREAK:
			return "break";
		case ELSE:
			return "else";
		case RETURN:
			return "return";
		case TYPE:
			return "type";
		case NUMBER:
			return "number";
		case NAME:
			return "name";
		case STRING:
			return "string";
		case CHAR:
			return "char";
		default:
			return "no-tok";
	}
}

#endif
