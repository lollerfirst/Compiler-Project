#pragma once

#include <lexer.h>
#include <stdbool.h>
#include <stdint.h>

/*** Fix for productions array indexing since lexer exists ***/
#define I(_idx) ((_idx)-NOTOK-1)

typedef enum __vartype{
    DELIM_LIST = NOTOK+1,
    ASSIGN_OP_VAR,
    ARITHM_OP_VAR,
    MUL_OP_VAR,
    BOOL_OP_VAR,
    L_ROUNDB_VAR,
    R_ROUNDB_VAR,
    L_SQUAREB_VAR,
    R_SQUAREB_VAR,
    L_CURLYB_VAR,
    R_CURLYB_VAR,
    END_STMT_VAR,
    ARGSEPARATOR,
    LOGIC_NOT_VAR,
    IF_VAR,
    WHILE_VAR,
    BREAK_VAR,
    ELSE_VAR,
    RETURN_VAR,
    TYPE_VAR,
    NUMBER_VAR,
    VARIABLE,
    STRING_VAR,
    CHAR_VAR,
    ARRAY,
    EXPR,
    VARLIST,
    EXPR_LIST,
    CALL,
    MULEXPR,
    ARITHMEXPR,
    ASSIGNEXPR,
    BOOLEXPR,
    ALLEXPR,
    DECLARATION,
    STATEMENT,
    STATEMENT_LIST,
    PROGRAM,
    END_PROD,
    END_ARR
} vartype_t;

union vardual_t{
    vartype_t vartype;
    toktype_t toktype;
};

typedef struct _ast{
    union vardual_t vardual;
    char* tk;
    size_t tl_len;
    size_t tl_capacity;
    struct _ast* tl;
} ast_t;


int parser_ast(ast_t* ast, const toklist_t* token_list);
void parser_free(ast_t* ast);
int parser_graph(ast_t* ast, const char* filename);
const char* parser_vartypestr(vartype_t vartype);