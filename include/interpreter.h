#ifndef _INTERPRETER_H
#define _INTERPRETER_H

#include <stddef.h>
#include <parser.h>

typedef enum
{
    LOCAL_REFERENCE,
    CHARACTER,
    INT
} type_t;

union __param
{
    size_t symbol_reference;
    int number_literal;
    char character_literal;
};

typedef struct __parameter
{
    type_t parameter_type;
    union __param param;
} parameter_t;

typedef struct __symbol
{
    // call identification
    char *name;
    char *signature;

    // tree of calls to make
    struct __symbol *forward_calls;
    size_t forward_calls_len;
    size_t forward_calls_capacity;

    // how to rearrange the passed-in parameters
    parameter_t *parameters_map;
    size_t parameters_map_len;
    size_t parameters_map_capacity;

} symbol_t;

int interpreter_init(void);
void interpreter_release(void);
int interpret(const ast_t *ast);


#endif