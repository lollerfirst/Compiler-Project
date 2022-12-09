#include <parser.h>
#include <lexer.h>
#include <unistd.h>
#include <compiler_errors.h>

#ifdef _DEBUG
#include <assert.h>
#endif

typedef enum
{
    LOCAL_REFERENCE,
    CHAR,
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
    char* name;
    char* signature;
    
    // tree of calls to make
    symbol_t* forward_calls;
    size_t forward_calls_len;
    size_t forward_calls_capacity;
    
    // how to rearrange the passed-in parameters
    parameter_t* parameters_map;
    size_t parameters_map_len;
    size_t parameters_map_capacity;

} symbol_t;

static symbol_t* global_symbol_table;
static size_t symbol_table_length;
static size_t symbol_table_capacity;






/* *INTERNAL* */
static void release_symbol(symbol_t* symbol);
static int interpret_prototype(const ast_t* ast, symbol_t* symbol, char** parameter_names);
static int interpret_definition(const ast_t* ast);
static int interpret_statement(const ast_t* ast);
static int interpret_truecall(const ast_t* ast, symbol_t* symbol, const char* local_names);
static int interpret_name(const ast_t* ast, char** name);
static int interpret_proto_signature(const ast_t* ast, char** signature, char** parameter_names);
static int interpret_stepcall_list(const ast_t ast, symbol_t* symbol, char* local_names);











// BASE FUNCTIONS
static parameter_t next(parameter_t arg)
{
    parameter_t rv;
    
    switch(arg.parameter_type)
    {
        case INT:
            rv.parameter_type = INT;
            rv.param.number_literal = (arg.param.number_literal)+1;
            break;

        case CHAR:
            rv.parameter_type = CHAR;
            rv.param.character_literal = (arg.param.character_literal)+1;
    }

    return rv;
}


static parameter_t prev(parameter_t arg)
{
    parameter_t rv;
    
    switch(arg.parameter_type)
    {
        case INT:
            rv.parameter_type = INT;
            rv.param.number_literal = (arg.param.number_literal)-1;
            break;

        case CHAR:
            rv.parameter_type = CHAR;
            rv.param.character_literal = (arg.param.character_literal)-1;
    }

    return rv;
}

static parameter_t proj(parameter_t* args)
{
    int index = args[0].param.number_literal;
    return args[index];
}

static parameter_t zero(void)
{
    parameter_t rv = {0};
    return rv;
}

static parameter_t wr(parameter_t* argument_list)
{
    if (argument_list[0].parameter_type != INT)
    {
        return TYPE_ERROR;
    }
    
    parameter_t rv = {.parameter_type = INT};

    int fd = argument_list[0].param.number_literal;
    int len = argument_list[1].param.number_literal;

    size_t i;
    for (i=0; i<len; ++i)
    {
        switch (argument_list[i+2].parameter_type)
        {
            case INT:
                rv.param.number_literal = (int) write(fd, (void*) &argument_list[i+2].param.number_literal, sizeof(int));
                break;

            case CHAR:
                rv.param.number_literal = (int) write(fd, (void*) &argument_list[i+2].param.character_literal, sizeof(char));
        }
    }

    return rv;
}

// ***


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

    global_symbol_table[1].name = "prev";

    global_symbol_table[2].name = "proj";

    global_symbol_table[3].name = "zero";

    global_symbol_table[4].name = "write";

    symbol_table_length = 5;

    return OK;
}

void interpreter_release(void)
{
    size_t i;
    for (i=0; i<symbol_table_length; ++i)
    {
        release_symbol(&global_symbol_table[i]);
    }

    free(global_symbol_table);
    global_symbol_table = NULL;
    symbol_table_length = 0;
    symbol_table_capacity = 0;
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
    assert(ast->tl_len > 0);
    assert(ast->vardual.vartype == STATEMENT)
    #endif

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
    assert(ast->tl_len > 0);
    assert(ast->vardual.vartype == DEFINITION);
    #endif

    symbol_t new_symbol = {0};
    char* parameter_names;

    ERROR_RETHROW(interpret_prototype(&ast->tl[0], &new_symbol, &parameter_names));

    ERROR_RETHROW(interpret_truecall(&ast->tl[2], &new_symbol, parameter_names),
        release_symbol(new_symbol),
        free(parameter_names)
    );

    // allocate more memory if necessary
    if (symbol_table_length >= symbol_table_capacity)
    {
        symbol_t* temp;
        if ((temp = reallocarray(global_symbol_table, symbol_table_capacity*2, sizeof(symbol_t))) == NULL)
        {
            release_symbol(new_symbol);
            return BAD_ALLOCATION;
        }

        global_symbol_table = temp;
        symbol_table_capacity *= 2;
    }

    global_symbol_table[symbol_table_length++] = new_symbol;
    return OK;
}

static int interpret_prototype(const ast_t* ast, symbol_t* symbol, char** parameter_names)
{
    #ifdef _DEBUG
    assert(ast != NULL);
    assert(ast->tl_len > 0);
    assert(symbol != NULL);
    assert(parameter_names != NULL);
    #endif

    ERROR_RETHROW(interpret_name(&ast->tl[0], &symbol->name));

    ERROR_RETHROW(interpret_proto_signature(&ast->tl[2], &symbol->signature, parameter_names));

    return OK;
}


static int interpret_name(const ast_t* ast, char** name) // pass a NAME_VAR
{
    #ifdef _DEBUG
    assert(ast != NULL);
    assert(name != NULL);
    assert(ast->tl_len > 0);
    #endif

    *name = (ast->tl_len > 1) ? ast->tl[1].tk : ast->tl[0].tk;
    return OK;
}

static int interpret_proto_signature(const ast_t* ast, char** signature, char** parameter_names)
{
    #ifdef _DEBUG
    assert(ast != NULL);
    assert(name != NULL);
    assert(ast->tl_len > 0);
    assert(ast->vardual.vartype == PARAMLIST);
    #endif

    size_t signature_len, signature_capacity, parameter_len, parameter_capacity;

    char* temp;
    if ((temp = calloc(64, sizeof(char))) == NULL)
    {
        return BAD_ALLOCATION;
    }
    signature_len = 0;
    signature_capacity = 64;

    char* temp0;
    if ((temp0 = calloc(64, sizeof(char))) == NULL)
    {
        return BAD_ALLOCATION;
    }
    parameter_len = 0;
    parameter_capacity = 64;

    do
    {
        // get the actual token
        toktype_t tt = (ast->tl[0].tl_len > 1) ? ast->tl[0].tl[1].vardual.toktype : ast->tl[0].tl[0].vardual.toktype;
        char* tk = (ast->tl[0].tl_len > 1) ? ast->tl[0].tl[1].tk : ast->tl[0].tl[0].tk;
        size_t p_len = strlen(tk);
        size_t i;

        switch(tt)
        {
            case NAME:

                // parameter name handling
                while (parameter_len + p_len + 1 >= parameter_capacity)
                {
                    if ((temp0 = reallocarray(temp0, parameter_capacity*2, sizeof(char))) == NULL)
                    {
                        return BAD_ALLOCATION;
                    }
                    parameter_capacity *= 2;
                }
                
                // copy names into parameter_names
                strncpy(&temp0[parameter_len], tk, p_len);
                temp0[parameter_len + p_len] = '\t';
                temp0[parameter_len + p_len + 1] = '\0';
                parameter_len += p_len + 1;

                // signature handling
                if (signature_len + 1 >= signature_capacity)
                {
                    if ((temp = reallocarray(temp, signature_capacity*2, sizeof(char))) == NULL)
                    {
                        return BAD_ALLOCATION;
                    }
                    signature_capacity *= 2;
                }

                temp[signature_len] = 'S';
                temp[signature_len+1] = '\0';
                ++signature_len;
                
                break;
            
            case STRING:
                // handle only signature
                tk = strstr(tk, "\"") + 1;
                
                for (i=0; i<p_len && tk[i] != "\""; ++i)
                {
                    if (signature_len + 2 >= signature_capacity)
                    {
                        if ((temp = reallocarray(temp, signature_capacity*2, sizeof(char))) == NULL)
                        {
                            return BAD_ALLOCATION;
                        }
                        signature_capacity *= 2;
                    }    
                

                    temp[signature_len] = 'C';
                    temp[signature_len+1] = tk[i];
                    signature_len += 2;
                }

                temp[signature_len] = '\0';
                break;

            case CHAR:
                tk = strstr(tk, "'") + 1;

                if (signature_len + 2 >= signature_capacity)
                {
                    if ((temp = reallocarray(temp, signature_capacity*2, sizeof(char))) == NULL)
                    {
                        return BAD_ALLOCATION;
                    }
                    signature_capacity *= 2;
                }    
            

                temp[signature_len] = 'C';
                temp[signature_len+1] = tk[i];
                signature_len += 2;

                break;
            
            case NUMBER:
                while (signature_len + p_len + 2 >= signature_capacity)
                {
                    if ((temp = reallocarray(temp, signature_capacity*2, sizeof(char))) == NULL)
                    {
                        return BAD_ALLOCATION;
                    }
                    signature_capacity *= 2;
                }    
            

                temp[signature_len++] = 'D';
                strncpy(&temp[signature_len], tk, p_len);
                signature_len += p_len;

                temp[signature_len++] = 'D';
                temp[signature_len] = '\0';
                break;

            default:
                free(temp);
                free(temp0);
                return INVALID_AST;
        }

        // ***

        if (ast->tl_len <= 1)
        {
            break;
        }
        
        ast = &ast->tl[1];
        

    } while (1);
    

    *signature = temp;
    *parameter_names = temp0;

    return OK;
}

static void release_symbol(symbol_t* symbol)
{
    #ifdef _DEBUG
    assert(symbol != NULL);
    #endif

    if (symbol->signature != NULL)
    {
        free(symbol->signature);
        symbol->signature = NULL;
    }

    if (symbol->forward_calls_list != NULL)
    {
        free(symbol->forwad_calls_list);
        symbol->forward_calls_list = NULL;
    }

    if (symbol->parameters_map != NULL)
    {
        free(symbol->parameters_map);
        symbol->parameters_map = NULL;
    }
}


static int interpret_truecall(const ast_t* ast, symbol_t* symbol, const char* local_names)
{
    #ifdef _DEBUG
    assert(ast != NULL);
    assert(ast->tl_len > 0);
    assert(symbol != NULL);
    assert(local_names != NULL);
    assert(ast->vardual.vartype == TRUECALL);
    #endif

    
    ERROR_RETHROW(symbol_list_alloc(symbol));   

    if (ast->tl_len == 1)
    {
        ERROR_RETHROW(interpret_basecall(&ast->tl[0], symbol->forward_calls, local_names),
            //free(symbol->parameters_map),
            free(symbol->forward_calls)
        );

        symbol->forward_calls_len = 1;
    }
    else
    {
        // fetch the name of the call
        char* name;
        ERROR_RETHROW(interpret_name(&ast->tl[0], &name));
        
        // verify it either appears on the table or it's a recursive call
        if (strcmp(name, symbol->name) != 0)
        {
            // verify it appears on the table
            size_t i;
            bool match = false;
            for (i=0; i<symbol_table_length; ++i)
            {
                if (strcmp(global_symbol_table[i].name, name) == 0)
                {
                    match = true;
                }
            
            }

            if (!match)
            {
                //free(symbol->parameters_map);
                free(symbol->forward_calls);
                return UNDEFINED_SYMBOL;
            }
        } 

        symbol->forward_calls[symbol->forward_calls_len].name = name;

        ERROR_RETHROW(interpret_stepcall_list(&ast->tl[2], symbol->forward_calls, local_names),
            //free(symbol->parameters_map),
            free(symbol->forward_calls)
        );

        ++symbol->forward_calls_len = 1
    }
    
    return OK;
}


static int interpret_stepcall_list(const ast_t ast, symbol_t* symbol, char* local_names)
{
    #ifdef _DEBUG
    assert(ast != NULL);
    assert(ast->tl_len > 0);
    assert(symbol != NULL);
    assert(local_names != NULL);
    assert(ast->vardual.vartype == STEPCALL_LIST);
    #endif

    ERROR_RETHROW(symbol_list_alloc(symbol));

    ERROR_RETHROW(interpret_stepcall(&ast->tl[0], symbol->forward_calls, local_names),
        free(symbol->forward_calls)
    );
    symbol->forward_calls_len = 1;

    while (ast->tl_len > 1)
    {
        while (symbol->forward_calls_len >= symbol->forward_calls_capacity)
        {
            ERROR_RETHROW(symbol_list_extend(symbol),
                free(symbol->foward_calls)
            );
        }
        
        ast = &ast->tl[2];
        ERROR_RETHROW(interpret_stepcall(&ast->tl[0], &symbol->forward_calls[symbol->forward_calls_len], local_names),
            free(symbol->forward_calls)
        );

        ++symbol->forward_calls_len;
    }

    return OK;
}

static int interpret_stepcall(const ast_t* ast, symbol_t* symbol, char* local_names)
{
    #ifdef _DEBUG
    assert(ast != NULL);
    assert(ast->tl_len > 0);
    assert(symbol != NULL);
    assert(local_names != NULL);
    assert(ast->vardual.vartype == STEPCALL);
    #endif


    if (ast->tl_len == 1)
    {
        ERROR_RETHROW(interpret_basecall(&ast->tl[0], symbol, local_names));

    }
    else
    {
        ERROR_RETHROW(symbol_list_alloc(symbol));

        // fetch the name of the call
        char* name;
        ERROR_RETHROW(interpret_name(&ast->tl[0], &name),
            free(symbol->forward_calls)
        );
        
        // verify it either appears on the table or it's a recursive call
        if (strcmp(name, symbol->name) != 0)
        {
            // verify it appears on the table
            size_t i;
            bool match = false;
            for (i=0; i<symbol_table_length; ++i)
            {
                if (strcmp(global_symbol_table[i].name, name) == 0)
                {
                    match = true;
                }
            
            }

            if (!match)
            {
                return UNDEFINED_SYMBOL;
            }
        }

        symbol->forward_calls[symbol->forward_calls_len].name = name;
        symbol->forward_calls_len++;

        ERROR_RETHROW(interpret_stepcall(&ast->tl[2], symbol, local_names));
    }

    return OK;
}


static int interpret_basecall(const ast_t* ast, symbol_t* symbol, char* local_names)
{
    #ifdef _DEBUG
    assert(ast != NULL);
    assert(ast->tl_len > 0);
    assert(symbol != NULL);
    assert(local_names != NULL);
    assert(ast->vardual.vartype == BASECALL);
    #endif

    ERROR_RETHROW(symbol_list_alloc(symbol));

    ERROR_RETHROW(interpret_paramlist(ast->tl[2], symbol, local_names),
        free(symbol->forward_calls)
    );
    
    char* name;
    ERROR_RETHROW(interpret_name(&ast->tl[0], &name),
        free(symbol->foward_calls),
        free(symbol->parameters_map)
    );
    
    // verify it either appears on the table or it's a recursive call
    if (strcmp(name, symbol->name) != 0)
    {
        // verify it appears on the table
        size_t i;
        bool match = false;
        for (i=0; i<symbol_table_length; ++i)
        {
            if (strcmp(global_symbol_table[i].name, name) == 0)
            {
                match = true;
            }
        
        }

        if (!match)
        {
            return UNDEFINED_SYMBOL;
        }
    } 

    symbol->forward_calls[(symbol->forward_calls_len)++] = name;

    return OK;
}

static int interpret_paramlist(const ast_t* ast, symbol_t* symbol, char* local_names)
{
    #ifdef _DEBUG
    assert(ast != NULL);
    assert(ast->tl_len > 0);
    assert(symbol != NULL);
    assert(local_names != NULL);
    assert(ast->vardual.vartype == PARAMLIST);
    #endif

    ERROR_RETHROW(parameter_list_alloc(symbol));

    ERROR_RETHROW(interpret_parameter(&ast->tl[0], symbol, local_names),
        free(symbol->parameter_map);
    );

    while (ast->tl_len > 1)
    {
        ast = &ast->tl[2];

        ERROR_RETHROW(interpret_parameter(&ast->tl[0], symbol, local_names),
            free(symbol->parameter_map);
        );
    }

    return OK;
}

static int interpret_parameter(const ast_t* ast, symbol_t* symbol, char* local_names)
{
    #ifdef _DEBUG
    assert(ast != NULL);
    assert(ast->tl_len > 0);
    assert(symbol != NULL);
    assert(local_names != NULL);
    assert(ast->vardual.vartype == PARAMETER);
    #endif

    size_t i;
    char* tk;
    ERROR_RETHROW(interpret_name(&ast->tl[0], &tk));

    while (symbol->parameters_map_len >= symbol->parameters_map_capacity)
    {
        ERROR_RETHROW(parameter_list_extend(symbol));
    }

    switch (ast->[0].vardual.vartype)
    {
        case NAME_VAR:
            // check the name is in the local_names
            char* f;
            if ((f = strtok(local_names, "\t")) == NULL)
            {
                return UNDEFINED_SYMBOL;
            }
            
            i = 0;
            bool match = false;
            do
            {
                if (strcmp(tk, f) == 0)
                {
                    match = true;
                    break;
                }
                ++i;
            } while ((f = strtok(NULL, "\t")) != NULL);
            
            if (!match)
            {
                return UNDEFINED_SYMBOL;
            }

            symbol->parameters_map[symbol->parameters_map_len].parameter_type = LOCAL_REFERENCE;
            symbol->parameters_map[symbol->parameters_map_len].param.symbol_reference = i;
            ++symbol->parameters_map_len;

            break;

        case NUMBER_VAR:

            symbol->parameters_map[symbol->parameters_map_len].parameter_type = INT;
            symbol->parameters_map[symbol->parameters_map_len].param.number_literal = atoi(tk);
            ++symbol->parameters_map_len;

            break;
        
        case CHAR_VAR:
            if ((tk = strstr(tk, "'")) == NULL)
            {
                return INVALID_AST;
            }

            symbol->parameters_map[symbol->parameters_map_len].parameter_type = CHAR;
            symbol->parameters_map[symbol->parameters_map_len].param.character_literal = tk[1];
            ++symbol->parameters_map_len;

            break;

        case STRING_VAR:
            if ((tk = strstr(tk, "\"")) == NULL)
            {
                return INVALID_AST;
            }
            
            i = 1;
            while (tk[i] != '\"' && tk[i] != '\0')
            {
                while (symbol->parameter_map_len >= symbol->parameter_map_capacity)
                {
                    ERROR_RETHROW(parameter_list_extend(symbol));
                }

                symbol->parameters_map[symbol->parameters_map_len].parameter_type = CHAR;
                symbol->parameters_map[symbol->parameters_map_len].param.character_literal = tk[i];
                ++symbol->parameters_map_len;
            }

            break;

        default: return INVALID_AST;
    }

    return OK;
}


static int symbol_list_alloc(symbol_t* symbol)
{
    symbol_t* temp1;
    if ((temp1 = calloc(1, sizeof(symbol_t))) == NULL)
    {
        return BAD_ALLOCATION;
    }
    
    symbol->forward_calls = temp1;
    symbol->forward_calls_capacity = 1;

    return OK;
}

static int symbol_list_extend(symbol_t* symbol)
{
    symbol_t* temp1;
    if ((temp1 = reallocarray(symbol->foward_calls, symbol->forward_calls_capacity*2, sizeof(symbol_t))) == NULL)
    {
        return BAD_ALLOCATION;
    }

    symbol->forward_calls = temp1;
    symbol->forward_calls_capacity *= 2;

    return OK;
}

static int parameter_list_alloc(symbol_t* symbol)
{
    parameter_t* temp1;
    if ((temp1 = calloc(2, sizeof(parameter_t))) == NULL)
    {
        return BAD_ALLOCATION;
    }
    
    symbol->parameters_map = temp1;
    symbol->parameters_map_capacity = 2;

    return OK;
}

static int parameter_list_extend(symbol_t* symbol)
{
    parameter_t* temp;
    if ((temp = reallocarray(symbol->parameters_map, symbol->parameters_map_capacity*2, sizeof(parameter_t))) == NULL)
    {
        return BAD_ALLOCATION;
    }

    symbol->parameters_map = temp;
    symbol->parameters_map_capacity *= 2;

    return OK;
}