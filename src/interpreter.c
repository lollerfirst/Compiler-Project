#include <parser.h>
#include <unistd.h>
#include <compiler_errors.h>

#ifdef _DEBUG
#include <assert.h>
#endif

// base functions

typedef enum
{
    LOCAL_REFERENCE,
    CHAR,
    INT
} type_t;


union __param
{
    size_t local_symbol_reference; // 0 -> 0-th parameter of the current function, 2 -> 2nd parameter of the current function
    int number_literal; // calling a function with a number literal
    char character_literal; // calling a charachter with a character literal
};

typedef struct __parameter
{
    type_t parameter_type;
    union __param param;
} parameter_t;

typedef struct __symbol
{
    char* name;
    size_t parameters_len;
    
    size_t global_symbol_reference;
    parameter_t* parameter_map;
} symbol_t;

static symbol_t* global_symbol_table;
static size_t symbol_table_length;
static size_t symbol_table_capacity;

static size_t choose_from_set(const size_t* chosen_indexes, size_t chosen_indexes_iterator, const parameter_t* argument_list, size_t argument_list_len);
static int execute_call(const ast_t* ast);
static int interpret_statement(const ast_t* ast);
static int interpret_definition(const ast_t* ast);


// BASE FUNCTIONS
static void next(void* arg0, type_t type_size)
{
    switch(type_size)
    {
        case CHAR:
            char* c = (char*)arg0;
            ++(*c);
            return;

        case INT:
            int* i = (int*)arg0;
            ++(*i);
            return;
    }
}


static void prev(void* arg0, type_t type_size)
{
    switch(type_size)
    {
        case CHAR:
            char* c = (char*)arg0;
            --(*c);
            return;
        
        case INT:
            int* i = (int*)arg0;
            --(*i);
            return;
    }
}

static void proj(int index, void* args, type_t type_size)
{
    switch(type_size)
    {
        case CHAR:
            char* c = (char*)args;
            c[0] = c[index];
            return;
        
        case INT:
            int* i = (int*)args;
            i[0] = i[index];
            return;
    }
}

static void* zero(void* arg0, type_t type_size)
{
    switch(type_size)
    {
        case CHAR:
            char* c = (char*)arg0;
            *c = '\0';
            return;
        
        case INT:
            int* i = (int*)arg0;
            *i = 0;
            return;
    }
}

// ***

static int interpret_statement(const ast_t* ast);
static int interpret_definition(const ast_t* ast);
static int execute_call(const ast_t* ast);
static int execute_call_recursive(const size_t global_table_index, parameter_t* argument_list, size_t argument_list_len, parameter_t* ret_value);
static int fetch_call_name(const ast_t* ast, char* name);


int interpreter_init(void)
{
    symbol_t* temp;
    if ((temp = calloc(sizeof(symbol_t), 20)) == NULL)
    {
        return BAD_ALLOCATION;
    }

    global_symbol_table = temp;
    symbol_table_capacity = 20;
    
    global_symbol_table[0].name = "next";
    global_symbol_table[0].parameters_len = 1;

    global_symbol_table[1].name = "prev";
    global_symbol_table[1].parameters_len = 1;

    global_symbol_table[2].name = "proj";
    global_symbol_table[2].parameters_len = 2;

    global_symbol_table[3].name = "zero";
    global_symbol_table[3].parameters_len = 0;

    global_symbol_table[4].name = "write";
    global_symbol_table[4].parameters_len = 2;

    symbol_table_length = 5;

    return OK;
}

int interpret(const ast_t* ast)
{
    #ifdef _DEBUG
    assert(ast != NULL);
    #endif

    if (ast->tl_len == 0)
    {
        return INVALID_AST;
    }

    if (ast->vardual.vartype == STATEMENT)
    {
        ERROR_RETHROW(interpret_statement(ast));
    }

    size_t i;
    for (i=0; i<ast->tl_len; ++i)
    {
        ERROR_RETHROW(interpret(&ast->tl[i]));
    }

    return OK;
}

static int interpret_statement(const ast_t* ast)
{
    #ifdef _DEBUG
    assert(ast != NULL);
    #endif

    if (ast->tl_len == 0)
    {
        return INVALID_AST;
    }

    if (ast->tl[0].vardual.vartype == BASECALL)
    {
        return execute_call(&ast->tl[0]);
    }
    
    if (ast->tl[0].vardual.vartype == DEFINITION)
    {
        return interpret_definition(&ast->tl[0]);
    }

    return INVALID_AST;
}

static int interpret_definition(const ast_t* ast)
{
    #ifdef _DEBUG
    assert(ast != NULL);
    #endif

    if (ast->tl_len == 0)
    {
        return INVALID_AST;
    }

    // need a symbol 
}

static int execute_call(const ast_t* ast)
{
    #ifdef _DEBUG
    assert(ast != NULL);
    #endif

    if (ast->tl_len == 0)
    {
        return INVALID_AST;
    }

    //fetch name of the call in question
    char* name;
    ERROR_RETHROW(fetch_call_name(&ast->tl[0], name));

    // allocate some memory for the arguments
    parameter_t* argument_list;
    size_t argument_list_len;
    size_t argument_list_capacity;

    if ((argument_list = calloc(sizeof(parameter_t), 10)) == NULL)
    {
        return BAD_ALLOCATION;
    }

    argument_list_len = 0;
    argument_list_capacity = 10;

    // fetch arguments
    ERROR_RETHROW(fetch_call_arguments(&ast->tl[2], argument_list, &argument_list_len, &argument_list_capacity),
        free(argument_list);
    );

    // verify there is a function with such name and number of arguments in the global symbols table
    size_t i;
    size_t chosen_indexes[64] = {0};
    size_t chosen_indexes_iterator = 0;
    bool match = false;

    for (i=0; i<symbol_table_length; ++i)
    {
        if ((strcmp(global_symbol_table[i].name, name) == 0) && (global_symbol_table[i].parameters_len <= argument_list_len))
        {
            match = true;

            chosen_indexes[chosen_indexes_iterator++] = i;
        }
    }

    // handle undefined symbols
    if (!match)
    {
        free(argument_list);
        return UNDEFINED_SYMBOL;
    }

    // choose a function evaluating arguments
    i = choose_from_set(chosen_indexes, chosen_indexes_iterator, argument_list, argument_list_len);
    

    // Call the function, return value will be stored into argument_list[0]
    ERROR_RETHROW(execute_call_recursive(i, argument_list, argument_list_len, NULL),
        free(argument_list);
    );

    free(argument_list);
    return OK;
}

static size_t choose_from_set(const size_t* chosen_indexes, size_t chosen_indexes_iterator, const parameter_t* argument_list, size_t argument_list_len)
{

    size_t i;

    bool match = false;
    for (i=0; i<chosen_indexes_iterator; ++i)
    {
        size_t j;
        for (j=0; j<argument_list_len; ++j)
        {
            if (global_symbol_table[i].parameter_map[j].parameter_type == argument_list[j].parameter_type &&
                argument_list[j].parameter_type == CHAR)
            {
                if (global_symbol_table[i].parameter_map[j].param.character_literal == argument_list[j].param.character_literal)
                {
                    match = true;
                    break;
                }
            }
            else if (global_symbol_table[i].parameter_map[j].parameter_type == argument_list[j].parameter_type &&
                argument_list[j].parameter_type == INT)
            {
                if (global_symbol_table[i].parameter_map[j].param.number_literal == argument_list[j].param.number_literal)
                {
                    match = true;
                    break;
                }
            }
        }

        if (match)
        {
            break;
        }
    }

    // Pick the function with the most argument if no actual match
    if (!match)
    {
        size_t max_index = 0;
        size_t max = 0;

        for(i=0; i<chosen_indexes_iterator; ++i)
        {
            if (global_symbol_table[i].parameters_len >= max)
            {
                max_index = i;
                max = global_symbol_table[i].parameters_len;
            }
        }

        i = max_index;
    }

    return i;
}

static int execute_call_recursive(const size_t global_table_index, parameter_t* argument_list, size_t argument_list_len, parameter_t* ret_value)
{
    #ifdef _DEBUG
    assert(argumen_list_len > 0);
    assert(argument_list != NULL);
    assert(symbol != NULL);
    #endif
    
    // handle base cases
    switch(global_table_index)
    {
        case 0: // next
            next((void*) &argument_list[0].param, argument_list[0].parameter_type);
            break;
        
        case 1:
            prev((void*) &argument_list[0].param, argument_list[0].parameter_type);
            break;
        
        case 2:
            proj(argument_list[0].param.number_literal, (void*) &argument_list[1].param, argument_list[1].parameter_type);
            break;

        case 3:
            zero((void*) &argument_list[0].param, argument_list[0].parameter_type);
            break;
        
        case 4:
            if (argument_list[0].parameter_type != INT)
            {
                return TYPE_ERROR;
            }
            
            int fd = argument_list[0].param.number_literal;
            size_t i;
            for (i=1; i<argument_list_len; ++i)
            {
                switch (argument_list[i].parameter_type)
                {
                    case INT:
                        write(fd, (void*) &argument_list[i].param.number_literal, sizeof(int));
                        break;

                    case CHAR:
                        write(fd, (void*) &argument_list[i].param.character_literal, sizeof(char));
                }
            }

        default: break;
    }

    // Gather the arguments
    size_t i;
    size_t k = 0;
    symbol_t symbol = global_symbol_table[global_table_index];
    parameter_t new_argument_list[argument_list_len];

    for (i=0; i<symbol.parameters_len; ++i)
    {
        switch (symbol.parameter_map[i].parameter_type)
        {
            case INT:
            case CHAR:
                new_argument_list[k++] = symbol.parameter_map[i];
                break;

            case LOCAL_REFERENCE:
                size_t ref = symbol.parameter_map[i].param.local_symbol_reference;

                new_argument_list[k++] = argument_list[ref];

                // if it's the last parameter, attach the rest of the arguments at the end
                if (ref == (symbol.parameters_len - 1))
                {
                    size_t j;
                    for (j = ref+1; j < argument_list_len; ++j)
                    {
                        new_argument_list[k++] = argument_list[j];
                    }

                }
                break;
            
            default:
                return TYPE_ERROR;
        }
    }

    parameter_t rv;
    ERROR_RETHROW(execute_call_recursive(symbol.global_symbol_reference, new_argument_list, k, &rv));

    // return the result
    if (ret_value != NULL)
    {
        *ret_value = rv;
    }

    return OK;

}

static int fetch_call_name(const ast_t* ast, char* name)
{
    #ifdef _DEBUG
    assert(ast != NULL);
    assert(ast->tl_len > 0);
    assert(name != NULL);
    #endif

    if (ast->vardual.vartype != NAME_VAR)
    {
        return INVALID_AST;
    }

    if (ast->tl[0].vardual.toktype == NAME)
    {
        name = ast->tl[0].tk;
        return OK;
    }
    else if (ast->tl[1].vardual.toktype == NAME)
    {
        name = ast->tl[1].tk;
        return OK;
    }
    else
    {
        return INVALID_AST;
    }
}

static int fetch_call_arguments(const ast_t* ast, parameter_t** argument_list, size_t* argument_len, size_t* argument_capacity)
{
    #ifdef _DEBUG
    assert(ast != NULL);
    assert(ast->tl_len > 0);
    assert(argument_list != NULL);
    assert((*argument_list) != NULL);

    assert(argument_len != NULL);
    assert(argument_capacity != NULL);
    assert((*argument_capacity) > 0);
    #endif

    // extend argument_list if necessary
    if ((*argument_len) >= (*argument_capacity))
    {
        parameter_t* temp;
        if ((temp = reallocarray(*argument_list, (*argument_capacity)*2, sizeof(parameter_t))) == NULL)
        {
            return BAD_ALLOCATION;
        }

        *argument_list = temp;
        *argument_capacity *= 2;
    }

    parameter_t* list = *argument_list;

    //fetch 1 parameter
    list[(*argument_len)] = ast->tl[0].tk;
}