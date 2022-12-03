#ifndef _PARSER_H_
#define _PARSER_H_

#include <lexer.h>
#include <stdbool.h>
#include <stdint.h>

/*** Fix for productions array indexing to offset toktype_t values ***/
#define indexer(_idx) ((_idx)-NOTOK-1)

typedef enum __vartype{
    DELIM_LIST = NOTOK+1,
    DEFINE_OP_VAR,
    L_ROUNDB_VAR,
    R_ROUNDB_VAR,
    L_SQUAREB_VAR,
    R_SQUAREB_VAR,
    END_STMT_VAR,
    ARGSEPARATOR,
    NUMBER_VAR,
    NAME_VAR,
    STRING_VAR,
    CHAR_VAR,
    PARAMETER,
    PARAMLIST,
    BASECALL,
    BASECALL_LIST,
    STEPCALL,
    STEPCALL_LIST,
    TRUECALL,
    DEFINITION,
    STATEMENT,
    STATEMENTLIST,
    PROGRAM,

    END_ARR,
    END_PROD
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


int parser_ast(ast_t* ast, toklist_t* token_list);
void parser_ast_delete(ast_t* ast);
int parser_ast_graph(ast_t* ast, const char* filename);
const char* parser_vartypestr(vartype_t vartype);

#endif