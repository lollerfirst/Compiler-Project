#include <interpreter.h>
#include <parser.h>
#include <lexer.h>
#include <unistd.h>
#include <compiler_errors.h>

#define DEFAULT_CALLS_THRESHOLD 5

#ifdef _DEBUG
#include <assert.h>
#endif

static symbol_t *global_symbol_table;
static size_t symbol_table_length;
static size_t symbol_table_capacity;

/*** INTERNAL ***/

static void release_symbol(symbol_t *symbol);
static int interpret_prototype(const ast_t *ast, symbol_t *symbol, char **parameter_names);
static int interpret_definition(const ast_t *ast);
static int interpret_statement(const ast_t *ast);
static int interpret_truecall(const ast_t *ast, symbol_t *symbol, char* def_name, char *local_names);
static int interpret_name(const ast_t *ast, char **name);
static int interpret_proto_signature(const ast_t *ast, char **signature, char **local_names);
static int interpret_stepcall_list(const ast_t* ast, symbol_t *symbol, char* def_name, char *local_names);
static int interpret_parameter(const ast_t *ast, symbol_t *symbol, char *local_names);
static int interpret_paramlist(const ast_t *ast, symbol_t *symbol, char *local_names);
static int interpret_basecall(const ast_t *ast, symbol_t *symbol, char* def_name, char *local_names);
static int interpret_stepcall(const ast_t *ast, symbol_t *symbol, char* def_name, char *local_names);
static int interpret_statement_list(const ast_t *ast);
static int parameter_list_extend(symbol_t *symbol);
static int parameter_list_alloc(symbol_t *symbol);
static int symbol_list_extend(symbol_t *symbol);
static int symbol_list_alloc(symbol_t *symbol);
static int get_signature(symbol_t *symbol);
static int execute_call(const ast_t *ast);
static int fetch_paramlist(const ast_t *ast, symbol_t *symbol);
static int fetch_parameter(const ast_t *ast, symbol_t *symbol);
static int select_from_set(size_t* selected, const size_t* suitable_set, size_t suitable_set_len, const char* signature);
static int execute_global_call(symbol_t *symbol);
static int execute_descent_recursive(symbol_t* selected_function, symbol_t* symbol);

// BASE FUNCTIONS
static parameter_t next(parameter_t arg)
{
    parameter_t rv = {0};

    switch (arg.parameter_type)
    {
    case INT:
        rv.parameter_type = INT;
        rv.param.number_literal = (arg.param.number_literal) + 1;
        break;

    case CHARACTER:
        rv.parameter_type = CHARACTER;
        rv.param.character_literal = (arg.param.character_literal) + 1;

        break;

    case LOCAL_REFERENCE:
        break;
    }

    return rv;
}

static parameter_t prev(parameter_t arg)
{
    parameter_t rv = {0};

    switch (arg.parameter_type)
    {
    case INT:
        rv.parameter_type = INT;
        rv.param.number_literal = (arg.param.number_literal) - 1;
        break;

    case CHARACTER:
        rv.parameter_type = CHARACTER;
        rv.param.character_literal = (arg.param.character_literal) - 1;
        break;

    case LOCAL_REFERENCE:
        break;
    }

    return rv;
}

static parameter_t proj(parameter_t *args)
{
    int index = args[0].param.number_literal;
    return args[index+1];
}

static parameter_t zero(void)
{
    parameter_t rv = {0};
    return rv;
}

static parameter_t wr(parameter_t *argument_list)
{
    #ifdef _DEBUG
    assert (argument_list[0].parameter_type == INT);
    #endif

    parameter_t rv = {.parameter_type = INT};

    int fd = argument_list[0].param.number_literal;
    int len = argument_list[1].param.number_literal;

    int i;
    for (i = 2; i < (len + 2); ++i)
    {
        switch (argument_list[i].parameter_type)
        {
        case INT:
            rv.param.number_literal = (int)write(fd, (void *)&argument_list[i].param.number_literal, sizeof(int));
            break;

        case CHARACTER:
            rv.param.number_literal = (int)write(fd, (void *)&argument_list[i].param.character_literal, sizeof(char));
            break;

        case LOCAL_REFERENCE:
            break;
        }
    }

    return rv;
}

// ***

int interpreter_init(void)
{
    symbol_t *temp;
    if ((temp = calloc(sizeof(symbol_t), 20)) == NULL)
    {
        #ifdef _DEBUG
        fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        #endif

        return BAD_ALLOCATION;
    }

    global_symbol_table = temp;
    symbol_table_capacity = 20;

    global_symbol_table[0].name = "next";
    global_symbol_table[0].signature = "S";
    
    global_symbol_table[1].name = "prev";
    global_symbol_table[1].signature = "S";
    
    global_symbol_table[2].name = "proj";
    global_symbol_table[2].signature = "S";

    global_symbol_table[3].name = "zero";
    global_symbol_table[3].signature = "S";

    global_symbol_table[4].name = "write";
    global_symbol_table[4].signature = "SSS";

    symbol_table_length = 5;

    return OK;
}

void interpreter_release(void)
{
    size_t i;
    for (i = 0; i < symbol_table_length; ++i)
    {
        if (i >= DEFAULT_CALLS_THRESHOLD)
        {
            release_symbol(&global_symbol_table[i]);
        }
    }

    free(global_symbol_table);
    global_symbol_table = NULL;
    symbol_table_length = 0;
    symbol_table_capacity = 0;
}

int interpret(const ast_t *ast)
{
#ifdef _DEBUG
    assert(ast != NULL);
    assert(ast->tl_len > 0);
    assert(ast->vardual.vartype == PROGRAM);
#endif

    ERROR_RETHROW(interpret_statement_list(&ast->tl[0]));

    return OK;
}

static int interpret_statement_list(const ast_t *ast)
{
#ifdef _DEBUG
    assert(ast != NULL);
    assert(ast->tl_len > 0);
    assert(ast->vardual.vartype == STATEMENTLIST);
#endif

    ERROR_RETHROW(interpret_statement(&ast->tl[0]));

    if (ast->tl_len > 1)
    {
        ERROR_RETHROW(interpret_statement_list(&ast->tl[1]));
    }

    return OK;
}

static int interpret_statement(const ast_t *ast)
{
#ifdef _DEBUG
    assert(ast != NULL);
    assert(ast->tl_len > 0);
    assert(ast->vardual.vartype == STATEMENT);
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

static int interpret_definition(const ast_t *ast)
{
#ifdef _DEBUG
    assert(ast != NULL);
    assert(ast->tl_len > 0);
    assert(ast->vardual.vartype == DEFINITION);
#endif

    symbol_t new_symbol = {0};
    char *def_name;
    char *parameter_names;

    ERROR_RETHROW(interpret_prototype(&ast->tl[0], &new_symbol, &parameter_names));

    def_name = new_symbol.name;

    ERROR_RETHROW(interpret_truecall(&ast->tl[2], &new_symbol, def_name, parameter_names),
                  release_symbol(&new_symbol),
                  free(parameter_names)
    );
//
    // allocate more memory if necessary
    if (symbol_table_length >= symbol_table_capacity)
    {
        symbol_t *temp;
        if ((temp = reallocarray(global_symbol_table, symbol_table_capacity * 2, sizeof(symbol_t))) == NULL)
        {
            release_symbol(&new_symbol);
            free(parameter_names);

            #ifdef _DEBUG
            fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
            #endif
            return BAD_ALLOCATION;
        }

        global_symbol_table = temp;
        symbol_table_capacity *= 2;
    }

    global_symbol_table[symbol_table_length++] = new_symbol;
    free(parameter_names);
    return OK;
}

static int interpret_prototype(const ast_t *ast, symbol_t *symbol, char **local_names)
{
#ifdef _DEBUG
    assert(ast != NULL);
    assert(ast->tl_len > 0);
    assert(symbol != NULL);
    assert(local_names != NULL);
    assert(ast->vardual.vartype == BASECALL);
#endif

    ERROR_RETHROW(interpret_name(&ast->tl[0], &symbol->name));

    ERROR_RETHROW(interpret_proto_signature(&ast->tl[2], &symbol->signature, local_names));

    return OK;
}

static int interpret_name(const ast_t *ast, char **name) // pass a NAME_VAR
{
#ifdef _DEBUG
    assert(ast != NULL);
    assert(name != NULL);
    assert(ast->tl_len > 0);
#endif

    *name = (ast->tl_len > 1) ? ast->tl[1].tk : ast->tl[0].tk;
    return OK;
}

static int interpret_proto_signature(const ast_t *ast, char **signature, char **local_names)
{
#ifdef _DEBUG
    assert(ast != NULL);
    assert(local_names != NULL);
    assert(ast->tl_len > 0);
    assert(ast->vardual.vartype == PARAMLIST);
#endif

    size_t signature_len, signature_capacity, parameter_len, parameter_capacity;

    char *temp;
    if ((temp = calloc(64, sizeof(char))) == NULL)
    {
        #ifdef _DEBUG
        fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        #endif

        return BAD_ALLOCATION;
    }
    signature_len = 0;
    signature_capacity = 64;

    char *temp0;
    if ((temp0 = calloc(64, sizeof(char))) == NULL)
    {
        #ifdef _DEBUG
        fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        #endif

        return BAD_ALLOCATION;
    }
    parameter_len = 0;
    parameter_capacity = 64;

    do
    {
        // get the actual token
        vartype_t tt = ast->tl[0].tl[0].vardual.vartype;
        
        char *tk;
        ERROR_RETHROW(interpret_name(&(ast->tl[0].tl[0]), &tk));
        size_t p_len = strlen(tk);
        size_t i;

        switch (tt)
        {
        case NAME_VAR:

            // parameter name handling
            while (parameter_len + p_len + 1 >= parameter_capacity)
            {
                if ((temp0 = reallocarray(temp0, parameter_capacity * 2, sizeof(char))) == NULL)
                {
                    #ifdef _DEBUG
                    fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
                    #endif

                    free(temp);
                    free(temp0);
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
                if ((temp = reallocarray(temp, signature_capacity * 2, sizeof(char))) == NULL)
                {
                    #ifdef _DEBUG
                    fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
                    #endif

                    free(temp);
                    free(temp0);
                    return BAD_ALLOCATION;
                }
                signature_capacity *= 2;
            }

            // e.g. "S" -> (signifies an arbitrary parameter)
            temp[signature_len] = 'S';
            temp[signature_len + 1] = '\0';
            ++signature_len;

            break;

        case STRING_VAR:
            tk = strstr(tk, "\"") + 1;

            for (i = 0; (i < p_len) && (tk[i] != '\"'); ++i)
            {
                while (signature_len + 2 >= signature_capacity)
                {
                    if ((temp = reallocarray(temp, signature_capacity * 2, sizeof(char))) == NULL)
                    {
                        #ifdef _DEBUG
                        fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
                        #endif

                        free(temp);
                        free(temp0);
                        return BAD_ALLOCATION;
                    }
                    signature_capacity *= 2;
                }

                // e.g. "CsCtCrCiCnCg\0"
                temp[signature_len] = 'C';
                temp[signature_len + 1] = tk[i];
                signature_len += 2;
            }

            temp[signature_len] = '\0';
            break;

        case CHAR_VAR:
            tk = strstr(tk, "'") + 1;

            while (signature_len + 2 >= signature_capacity)
            {
                if ((temp = reallocarray(temp, signature_capacity * 2, sizeof(char))) == NULL)
                {
                    #ifdef _DEBUG
                    fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
                    #endif

                    free(temp);
                    free(temp0);
                    return BAD_ALLOCATION;
                }
                signature_capacity *= 2;
            }

            // e.g. "Ca"
            temp[signature_len] = 'C';
            temp[signature_len + 1] = tk[i];
            signature_len += 2;

            break;

        case NUMBER_VAR:
            while (signature_len + p_len + 2 >= signature_capacity)
            {
                if ((temp = reallocarray(temp, signature_capacity * 2, sizeof(char))) == NULL)
                {
                    #ifdef _DEBUG
                    fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
                    #endif

                    free(temp);
                    free(temp0);
                    return BAD_ALLOCATION;
                }
                signature_capacity *= 2;
            }

            // e.g. "D1001D"
            temp[signature_len++] = 'D';
            strncpy(&temp[signature_len], tk, p_len);
            signature_len += p_len;

            temp[signature_len++] = 'D';
            temp[signature_len] = '\0';
            break;

        default:
            free(temp);
            free(temp0);

            #ifdef _DEBUG
            fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
            #endif
            return INVALID_AST;
        }

        // ***

        if (ast->tl_len == 1)
        {
            break;
        }

        ast = &ast->tl[2];

    } while (1);

    *signature = temp;
    *local_names = temp0;

    return OK;
}

static void release_symbol(symbol_t *symbol)
{
    #ifdef _DEBUG
        assert(symbol != NULL);
    #endif

    if (symbol->signature != NULL)
    {
        free(symbol->signature);
        symbol->signature = NULL;
    }

    if (symbol->forward_calls != NULL)
    {
        size_t i;
        for (i = 0; i < symbol->forward_calls_len; ++i)
        {
            release_symbol(&(symbol->forward_calls[i]));
        }

        free(symbol->forward_calls);
        symbol->forward_calls = NULL;
    }

    if (symbol->parameters_map != NULL)
    {
        free(symbol->parameters_map);
        symbol->parameters_map = NULL;
    }

    symbol->name = NULL;
    symbol->forward_calls_capacity = 0;
    symbol->forward_calls_len = 0;
    symbol->parameters_map_capacity = 0;
    symbol->parameters_map_len = 0;

    return;
}

static int interpret_truecall(const ast_t *ast, symbol_t *symbol, char* def_name, char *local_names)
{
#ifdef _DEBUG
    assert(ast != NULL);
    assert(ast->tl_len > 0);
    assert(symbol != NULL);
    assert(local_names != NULL);
    assert(ast->vardual.vartype == TRUECALL);
#endif


    if (ast->tl_len == 1)
    {
        ERROR_RETHROW(symbol_list_alloc(symbol));
        symbol->forward_calls_len = 1;

        ERROR_RETHROW(interpret_basecall(&ast->tl[0], symbol->forward_calls, def_name, local_names),
            release_symbol(symbol)
        );

    }
    else
    {
    
        // fetch the name of the call
        char *name;
        ERROR_RETHROW(interpret_name(&ast->tl[0], &name));

        // verify it either appears on the table or it's a recursive call
        if (strcmp(name, def_name) != 0)
        {
            // verify it appears on the table
            size_t i;
            bool match = false;
            for (i = 0; i < symbol_table_length; ++i)
            {
                if (strcmp(global_symbol_table[i].name, name) == 0)
                {
                    match = true;
                }
            }

            if (!match)
            {
                #ifdef _DEBUG
                fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
                #endif

                return UNDEFINED_SYMBOL;
            }
        }

        // allocate forward calls list
        ERROR_RETHROW(symbol_list_alloc(symbol));

        symbol->forward_calls->name = name;
        symbol->forward_calls_len = 1;

        ERROR_RETHROW(interpret_stepcall_list(&ast->tl[2], symbol->forward_calls, def_name, local_names),
            release_symbol(symbol)
        );

        
    }

    return OK;
}

static int interpret_stepcall_list(const ast_t *ast, symbol_t *symbol, char* def_name, char *local_names)
{
#ifdef _DEBUG
    assert(ast != NULL);
    assert(ast->tl_len > 0);
    assert(symbol != NULL);
    assert(local_names != NULL);
    assert(ast->vardual.vartype == STEPCALL_LIST);
#endif

    ERROR_RETHROW(symbol_list_alloc(symbol));
    symbol->forward_calls_len = 1;

    ERROR_RETHROW(interpret_stepcall(&ast->tl[0], symbol->forward_calls, def_name, local_names),
                  release_symbol(symbol)
    );
    

    while (ast->tl_len > 1)
    {
        while (symbol->forward_calls_len >= symbol->forward_calls_capacity)
        {
            ERROR_RETHROW(symbol_list_extend(symbol),
                          release_symbol(symbol));
        }

        ast = &ast->tl[2];

        ERROR_RETHROW(interpret_stepcall(&ast->tl[0], &symbol->forward_calls[symbol->forward_calls_len], def_name, local_names),
                      release_symbol(symbol));

        ++symbol->forward_calls_len;
    }

    return OK;
}

static int interpret_stepcall(const ast_t *ast, symbol_t *symbol, char* def_name, char *local_names)
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
        ERROR_RETHROW(interpret_basecall(&ast->tl[0], symbol, def_name, local_names));
    }
    else
    {
        ERROR_RETHROW(symbol_list_alloc(symbol));

        // fetch the name of the call
        char *name;
        ERROR_RETHROW(interpret_name(&ast->tl[0], &name),
                      release_symbol(symbol));

        // verify it either appears on the table or it's a recursive call
        if (strcmp(name, def_name) != 0)
        {
            // verify it appears on the table
            size_t i;
            bool match = false;
            for (i = 0; i < symbol_table_length; ++i)
            {
                if (strcmp(global_symbol_table[i].name, name) == 0)
                {
                    match = true;
                }
            }

            if (!match)
            {
                #ifdef _DEBUG
                fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
                #endif

                return UNDEFINED_SYMBOL;
            }
        }

        ERROR_RETHROW(interpret_stepcall(&ast->tl[2], symbol->forward_calls, def_name, local_names),
            release_symbol(symbol));

        symbol->name = name;
        symbol->forward_calls_len = 1;
    }

    return OK;
}

static int interpret_basecall(const ast_t *ast, symbol_t *symbol, char* def_name, char *local_names)
{
#ifdef _DEBUG
    assert(ast != NULL);
    assert(ast->tl_len > 0);
    assert(symbol != NULL);
    assert(local_names != NULL);
    assert(ast->vardual.vartype == BASECALL);
#endif

    char *name;
    ERROR_RETHROW(interpret_name(&ast->tl[0], &name));

    // verify it either appears on the table or it's a recursive call
    if (strcmp(name, def_name) != 0)
    {
        // verify it appears on the table
        size_t i;
        bool match = false;
        for (i = 0; i < symbol_table_length; ++i)
        {
            if (strcmp(global_symbol_table[i].name, name) == 0)
            {
                match = true;
            }
        }

        if (!match)
        {
            #ifdef _DEBUG
            fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
            #endif

            return UNDEFINED_SYMBOL;
        }
    }

    ERROR_RETHROW(interpret_paramlist(&ast->tl[2], symbol, local_names));

    symbol->name = name;

    return OK;
}

static int interpret_paramlist(const ast_t *ast, symbol_t *symbol, char *local_names)
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
                  release_symbol(symbol););

    while (ast->tl_len > 1)
    {
        ast = &ast->tl[2];

        ERROR_RETHROW(interpret_parameter(&ast->tl[0], symbol, local_names),
                      release_symbol(symbol));
    }

    return OK;
}

static int interpret_parameter(const ast_t *ast, symbol_t *symbol, char *local_names)
{
#ifdef _DEBUG
    assert(ast != NULL);
    assert(ast->tl_len > 0);
    assert(symbol != NULL);
    assert(local_names != NULL);
    assert(ast->vardual.vartype == PARAMETER);
#endif

    size_t i;
    char *tk;
    ERROR_RETHROW(interpret_name(&ast->tl[0], &tk));

    while (symbol->parameters_map_len >= symbol->parameters_map_capacity)
    {
        ERROR_RETHROW(parameter_list_extend(symbol));
    }

    vartype_t tt = ast->tl[0].vardual.vartype;
    char* f;
    char c;

    switch (tt)
    {
    case NAME_VAR:
        // check the name is in the local_name
        
        if ((f = strstr(local_names, "\t")) == NULL)
        {
            #ifdef _DEBUG
            fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
            #endif

            return UNDEFINED_SYMBOL;
        }

        i = 0;
        bool match = false;
        do
        {
            *f = '\0';
            if (strcmp(tk, local_names) == 0)
            {
                match = true;
                *f = '\t';
                break;
            }
            ++i;
            *f = '\t';
            local_names = f+1;
        } while ((f = strstr(local_names, "\t")) != NULL);

        if (!match)
        {
            #ifdef _DEBUG
            fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
            #endif

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
            #ifdef _DEBUG
            fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
            #endif

            return INVALID_AST;
        }

        symbol->parameters_map[symbol->parameters_map_len].parameter_type = CHARACTER;
        symbol->parameters_map[symbol->parameters_map_len].param.character_literal = tk[1];
        ++symbol->parameters_map_len;

        break;

    case STRING_VAR:
        if ((tk = strstr(tk, "\"")) == NULL)
        {
            #ifdef _DEBUG
            fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
            #endif

            return INVALID_AST;
        }

        i = 1;
        while (tk[i] != '"' && tk[i] != '\0')
        {
            while (symbol->parameters_map_len >= symbol->parameters_map_capacity)
            {
                ERROR_RETHROW(parameter_list_extend(symbol));
            }

            c = tk[i];
            if (c == '\\')
            {
                switch (tk[i+1])
                {
                case 'n':
                    c = '\n';
                    ++i;
                    break;
                
                // insert other cases when necessary
                default:
                    break;
                }
            }

            symbol->parameters_map[symbol->parameters_map_len].parameter_type = CHARACTER;
            symbol->parameters_map[symbol->parameters_map_len].param.character_literal = c;
            ++symbol->parameters_map_len;

            ++i;
        }

        break;

    default:
        #ifdef _DEBUG
        fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        #endif
        return INVALID_AST;
    }

    return OK;
}

static int symbol_list_alloc(symbol_t *symbol)
{
    symbol_t *temp1;
    if ((temp1 = calloc(1, sizeof(symbol_t))) == NULL)
    {
        #ifdef _DEBUG
        fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        #endif
        return BAD_ALLOCATION;
    }

    symbol->forward_calls = temp1;
    symbol->forward_calls_capacity = 1;

    return OK;
}

static int symbol_list_extend(symbol_t *symbol)
{
    symbol_t *temp1;
    if ((temp1 = reallocarray(symbol->forward_calls, symbol->forward_calls_capacity * 2, sizeof(symbol_t))) == NULL)
    {
        #ifdef _DEBUG
        fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        #endif
        return BAD_ALLOCATION;
    }

    symbol->forward_calls = temp1;
    symbol->forward_calls_capacity *= 2;

    bzero(symbol->forward_calls + symbol->forward_calls_len,
        (symbol->forward_calls_capacity - symbol->forward_calls_len) * sizeof(symbol_t));

    return OK;
}

static int parameter_list_alloc(symbol_t *symbol)
{
    parameter_t *temp1;
    if ((temp1 = calloc(2, sizeof(parameter_t))) == NULL)
    {
        #ifdef _DEBUG
        fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        #endif

        return BAD_ALLOCATION;
    }

    symbol->parameters_map = temp1;
    symbol->parameters_map_capacity = 2;

    return OK;
}

static int parameter_list_extend(symbol_t *symbol)
{
    parameter_t *temp;
    if ((temp = reallocarray(symbol->parameters_map, symbol->parameters_map_capacity * 2, sizeof(parameter_t))) == NULL)
    {
        #ifdef _DEBUG
        fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        #endif
        return BAD_ALLOCATION;
    }

    symbol->parameters_map = temp;
    symbol->parameters_map_capacity *= 2;

    bzero(symbol->parameters_map + symbol->parameters_map_len,
        (symbol->parameters_map_capacity - symbol->parameters_map_len) * sizeof(parameter_t));

    return OK;
}

static int execute_call(const ast_t *ast)
{
    #ifdef _DEBUG
        assert(ast != NULL);
        assert(ast->tl_len > 0);
        assert(ast->vardual.vartype == BASECALL);
    #endif

    symbol_t target = {0};

    ERROR_RETHROW(interpret_name(&ast->tl[0], &target.name));
    ERROR_RETHROW(fetch_paramlist(&ast->tl[2], &target));


    ERROR_RETHROW(execute_global_call(&target),
        release_symbol(&target);
    );

    release_symbol(&target);
    return OK;
}


static int execute_global_call(symbol_t *symbol)
{
#ifdef _DEBUG
    assert(symbol->name != NULL);
    assert(symbol->parameters_map != NULL);
    assert(symbol->parameters_map_len > 0);
#endif


    size_t i;
    size_t suitable_set[64];
    size_t suitable_set_len = 0;

    bool match = false;
    for (i = 0; i < symbol_table_length; ++i)
    {
        if (strcmp(symbol->name, global_symbol_table[i].name) == 0)
        {
            match = true;
            
            if (suitable_set_len < 64)
            {
                suitable_set[suitable_set_len++] = i;
            }
        }
    }

    if (!match)
    {
        #ifdef _DEBUG
        fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        #endif

        return UNDEFINED_SYMBOL;
    }
    
    // get the signature
    ERROR_RETHROW(get_signature(symbol));

    // select a function from the suitable set
    size_t selected;
    ERROR_RETHROW(select_from_set(&selected, suitable_set, suitable_set_len, symbol->signature),
        release_symbol(symbol)
    );
    symbol_t* selected_function = &global_symbol_table[suitable_set[selected]];

    // Distinguish between default call and defined call
    if (suitable_set[selected] >= DEFAULT_CALLS_THRESHOLD)
    {
        // descend onto the call tree
        ERROR_RETHROW(execute_descent_recursive(selected_function->forward_calls, symbol),
            release_symbol(symbol)
        );
    }
    else
    {
        switch (suitable_set[selected])
        {
        case 0:
            symbol->parameters_map[0] = next(symbol->parameters_map[0]);
            return OK;

        case 1:
            symbol->parameters_map[0] = prev(symbol->parameters_map[0]);
            return OK;

        case 2:
            symbol->parameters_map[0] = proj(symbol->parameters_map);
            return OK;

        case 3:
            symbol->parameters_map[0] = zero();
            return OK;

        case 4:
            symbol->parameters_map[0] = wr(symbol->parameters_map);
            return OK;

        default:
            fprintf(stderr, "FILE: %s, LINE: %d\n", __FILE__, __LINE__);
            return UNDEFINED_SYMBOL;        
        }
    }

    return 0;
}

static int execute_descent_recursive(symbol_t* selected_function, symbol_t* arguments)
{
    #ifdef _DEBUG
    assert(selected_function != NULL);
    assert(arguments != NULL);
    assert(arguments->parameters_map != NULL);
    assert(selected_function->parameters_map != NULL || selected_function->forward_calls != NULL);
    #endif

    size_t i;
    symbol_t return_values_collector = {0};
    

    // DISTINGUISH BETWEEN BASECALL AND STRUCTURED ONES
    if (selected_function->forward_calls_len > 0)
    {
        // Allocate memory for the return values collector
        ERROR_RETHROW(parameter_list_alloc(&return_values_collector));
        
        for (i=0; i<selected_function->forward_calls_len; ++i)
        {
            // allocate memory to forward arguments and get back the return value
            symbol_t forward_arguments = {0};
            ERROR_RETHROW(parameter_list_alloc(&forward_arguments),
                release_symbol(&return_values_collector)
            );

            // copy in arguments AS THEY ARE
            size_t j;
            for (j=0; j<arguments->parameters_map_len; ++j)
            {
                // extendend memory as needed
                while (forward_arguments.parameters_map_len >= forward_arguments.parameters_map_capacity)
                {
                    ERROR_RETHROW(parameter_list_extend(&forward_arguments),
                        release_symbol(&return_values_collector)
                    );
                }

                forward_arguments.parameters_map[j] = arguments->parameters_map[j];
                ++forward_arguments.parameters_map_len;
            }

            // Descend onto the tree passing the forwarded_arguments
            ERROR_RETHROW(execute_descent_recursive(&selected_function->forward_calls[i], &forward_arguments),
                release_symbol(&forward_arguments),
                release_symbol(&return_values_collector)
            );

            // The return value is in forward_arguments[0], save it onto the collector before moving on
            while (return_values_collector.parameters_map_len >= return_values_collector.parameters_map_capacity)
            {
                ERROR_RETHROW(parameter_list_extend(&return_values_collector),
                    release_symbol(&forward_arguments)
                );
            }
            return_values_collector.parameters_map[i] = forward_arguments.parameters_map[0];
            ++return_values_collector.parameters_map_len;

            // release the memory allocated for forwarding the arguments
            release_symbol(&forward_arguments);
        }

        // CALL THE ACTUAL FUNCTION WITH THE RETURNED PARAMETERS
        return_values_collector.name = selected_function->name;
        ERROR_RETHROW(execute_global_call(&return_values_collector),
            release_symbol(&return_values_collector)
        );

        
        arguments->parameters_map[0] = return_values_collector.parameters_map[0];
        release_symbol(&return_values_collector);

    }
    else if(selected_function->parameters_map_len > 0) // ELSE IF THE SELECTED CALL IS A BASE CALL, BIND THE ARGUMENTS TO THE PARAMETERS
    {
        // allocate memory to forward arguments and get back the return value
        symbol_t forward_arguments = {0};
        ERROR_RETHROW(parameter_list_alloc(&forward_arguments),
            release_symbol(&return_values_collector)
        );

        // Scan the selected functions parameters to find references or literals and copy them into forward arguments
        size_t j;
        size_t last_max = 0;
        size_t max = 0;
        bool is_referencing = false;

        for (j=0; j<selected_function->parameters_map_len; ++j)
        {
            // extendend memory as needed
            while (forward_arguments.parameters_map_len >= forward_arguments.parameters_map_capacity)
            {
                ERROR_RETHROW(parameter_list_extend(&forward_arguments),
                    release_symbol(&return_values_collector)
                );
            }
            
            size_t reference;

            // distinguish between references and actual literals
            switch (selected_function->parameters_map[j].parameter_type)
            {
            case INT:
            case CHARACTER:
                
                forward_arguments.parameters_map[j] = selected_function->parameters_map[j];
                ++forward_arguments.parameters_map_len;
                break;

            case LOCAL_REFERENCE:
                
                is_referencing = true;
                reference = selected_function->parameters_map[j].param.symbol_reference;
                forward_arguments.parameters_map[j] = arguments->parameters_map[reference];
                ++forward_arguments.parameters_map_len;
                
                // if the reference number is greater than the previous one, update this variable with later use
                if (reference >= max)
                {
                    last_max = j;
                    max = reference;
                }
            }            
        }
        
        // Check if the max reference is ALSO the last parameter, if so, copy in all the other (variadic style)
        if (is_referencing && last_max == j - 1)
        {
            ++max;

            for (; max < arguments->parameters_map_len; ++max)
            {
                // extendend memory as needed
                while (forward_arguments.parameters_map_len >= forward_arguments.parameters_map_capacity)
                {
                    ERROR_RETHROW(parameter_list_extend(&forward_arguments),
                        release_symbol(&return_values_collector)
                    );
                }

                forward_arguments.parameters_map[j++] = arguments->parameters_map[max];
                ++forward_arguments.parameters_map_len;
            }
        }

        // CALL THE ACTUAL FUNCTION
        forward_arguments.name = selected_function->name;
        ERROR_RETHROW(execute_global_call(&forward_arguments),
            release_symbol(&forward_arguments)
        );

        // return the returned value
        arguments->parameters_map[0] = forward_arguments.parameters_map[0];

        // release the memory allocated here
        release_symbol(&forward_arguments);
    }

    return OK;
}

static int fetch_paramlist(const ast_t *ast, symbol_t *symbol)
{
#ifdef _DEBUG
    assert(ast != NULL);
    assert(ast->tl_len > 0);
    assert(symbol != NULL);
    assert(ast->vardual.vartype == PARAMLIST);
#endif

    ERROR_RETHROW(parameter_list_alloc(symbol));

    ERROR_RETHROW(fetch_parameter(&ast->tl[0], symbol),
                  release_symbol(symbol));

    while (ast->tl_len > 1)
    {
        ast = &ast->tl[2];

        ERROR_RETHROW(fetch_parameter(&ast->tl[0], symbol),
                      release_symbol(symbol));
    }

    return OK;
}

static int fetch_parameter(const ast_t *ast, symbol_t *symbol)
{
#ifdef _DEBUG
    assert(ast != NULL);
    assert(ast->tl_len > 0);
    assert(symbol != NULL);
    assert(ast->vardual.vartype == PARAMETER);
#endif

    size_t i;
    char *tk;
    ERROR_RETHROW(interpret_name(&ast->tl[0], &tk));

    while (symbol->parameters_map_len >= symbol->parameters_map_capacity)
    {
        ERROR_RETHROW(parameter_list_extend(symbol));
    }

    vartype_t tt = (ast->tl_len > 1) ? ast->tl[1].vardual.vartype : ast->tl[0].vardual.vartype;

    switch (tt)
    {
    case NAME_VAR:
        #ifdef _DEBUG
        fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        #endif

        return EXPECTED_LITERAL;

    case NUMBER_VAR:

        symbol->parameters_map[symbol->parameters_map_len].parameter_type = INT;
        symbol->parameters_map[symbol->parameters_map_len].param.number_literal = atoi(tk);
        ++symbol->parameters_map_len;

        break;

    case CHAR_VAR:
        if ((tk = strstr(tk, "'")) == NULL)
        {
            #ifdef _DEBUG
            fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
            #endif
            
            return INVALID_AST;
        }

        symbol->parameters_map[symbol->parameters_map_len].parameter_type = CHARACTER;
        symbol->parameters_map[symbol->parameters_map_len].param.character_literal = tk[1];
        ++symbol->parameters_map_len;

        break;

    case STRING_VAR:
        if ((tk = strstr(tk, "\"")) == NULL)
        {
            #ifdef _DEBUG
            fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
            #endif

            return INVALID_AST;
        }

        i = 1;
        while (tk[i] != '\"' && tk[i] != '\0')
        {
            while (symbol->parameters_map_len >= symbol->parameters_map_capacity)
            {
                ERROR_RETHROW(parameter_list_extend(symbol));
            }

            symbol->parameters_map[symbol->parameters_map_len].parameter_type = CHARACTER;
            symbol->parameters_map[symbol->parameters_map_len].param.character_literal = tk[i];
            ++symbol->parameters_map_len;
        }

        break;

    default:
        #ifdef _DEBUG
        fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        #endif
        return INVALID_AST;
    }

    return OK;
}

static int get_signature(symbol_t *symbol)
{
    // allocate memory
    size_t signature_len, signature_capacity;

    char *temp;
    if ((temp = calloc(64, sizeof(char))) == NULL)
    {
        #ifdef _DEBUG
        fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        #endif
        return BAD_ALLOCATION;
    }

    signature_len = 0;
    signature_capacity = 64;

    size_t i;
    for (i = 0; i < symbol->parameters_map_len; ++i)
    {  
        size_t p_len;

        switch (symbol->parameters_map[i].parameter_type)
        {
        case INT:
            // get the length of the string that will be written
            p_len = snprintf(NULL, 0, "%d", symbol->parameters_map[i].param.number_literal);

            while (signature_len + p_len + 2 >= signature_capacity)
            {
                if ((temp = reallocarray(temp, signature_capacity * 2, sizeof(char))) == NULL)
                {
                    #ifdef _DEBUG
                    fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
                    #endif
                    free(temp);
                    return BAD_ALLOCATION;
                }
                signature_capacity *= 2;
            }

            // e.g. "D1001D"
            temp[signature_len++] = 'D';

            sprintf(&temp[signature_len], "%d", symbol->parameters_map[i].param.number_literal);

            signature_len += p_len;

            temp[signature_len++] = 'D';
            temp[signature_len] = '\0';
            break;

        case CHARACTER:
            while (signature_len + 1 >= signature_capacity)
            {
                if ((temp = reallocarray(temp, signature_capacity * 2, sizeof(char))) == NULL)
                {
                    #ifdef _DEBUG
                    fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
                    #endif
                    free(temp);
                    return BAD_ALLOCATION;
                }
                signature_capacity *= 2;
            }

            temp[signature_len++] = 'C';
            temp[signature_len++] = symbol->parameters_map[i].param.character_literal;
            temp[signature_len] = '\0';
            break;

        default:
            #ifdef _DEBUG
            fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
            #endif

            free(temp);
            return EXPECTED_LITERAL;
        }
    }

    symbol->signature = temp;
    return OK;
}

static int select_from_set(size_t* selected, const size_t* suitable_set, size_t suitable_set_len, const char* signature)
{
    #ifdef _DEBUG
    assert(selected != NULL);
    assert(suitable_set != NULL);
    assert(suitable_set_len > 0);
    assert(signature != NULL);
    #endif

    size_t i;
    bool found = false;

    for (i=0; i<suitable_set_len; ++i)
    {
        symbol_t* symbol = &global_symbol_table[suitable_set[i]];

        const char* candidate = symbol->signature;
        if (candidate == NULL)
        {
            #ifdef _DEBUG
            fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
            #endif
            return INVALID_SIGNATURE;
        }

        bool accepts = true;
        size_t j = 0;
        size_t k = 0;

        while (accepts && candidate[j] != '\0')
        {
            if (signature[k] == '\0')
            {
                accepts = false;
                break;
            }

            switch (candidate[j])
            {

            case 'D':
                accepts = false;
                if (signature[k++] == candidate[j++])
                {
                    while ((signature[k] == candidate[j]) && (signature[k] != '\0'))
                    {
                        if (candidate[j] == 'D')
                        {
                            ++j;
                            ++k;
                            accepts = true;
                            break;
                        }

                        ++j;
                        ++k;
                    }
                }
                break;

            case 'C':
                accepts = false;

                if (signature[k++] == candidate[j++])
                {
                    if ((signature[k] == candidate[j]) && signature[k] != '\0')
                    {
                        accepts = true;
                    }

                    ++j;
                    ++k;
                }

                break;
                
            case 'S':
                if (signature[k] == 'D')
                {
                    do
                    {
                        ++k;
                    }
                    while (signature[k] != 'D' && signature[k] != '\0');

                    k = (signature[k] != '\0') ? k+1 : k;
                }
                else if (signature[k] == 'C')
                {
                    ++k;
                    k = (signature[k] != '\0') ? k+1 : k;
                }
                else
                {
                    ++k;
                }
                ++j;

                break;

            default:
                #ifdef _DEBUG
                fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
                #endif
                return INVALID_SIGNATURE;
            }
        }

        if (accepts)
        {
            found = true;
            *selected = i;

            break;
        }
    }

    if (!found)
    {
        #ifdef _DEBUG
        fprintf(stderr, "[!!] FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        #endif
        return INVALID_SIGNATURE;
    }

    return OK;
}