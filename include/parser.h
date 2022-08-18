#pragma once

#include <lexer.h>

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
    END_PROD
} vartype_t;

#undef I