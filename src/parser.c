#include <parser.h>
#include <lexer.h>
#include <compiler_errors.h>

/*** WRAPPING TOKENS INTO DELIMITERS ***/

static vartype_t DelimList[] = {(vartype_t)DELIM, END_PROD,
                            (vartype_t)DELIM, DELIM_LIST, END_PROD,
                            END_ARR};

static vartype_t Define_OP[] = {(vartype_t)DEFINE_OP, END_PROD,
                            DELIM_LIST, (vartype_t)DEFINE_OP, END_PROD,
                            (vartype_t)DEFINE_OP, DELIM_LIST, END_PROD,
                            DELIM_LIST, (vartype_t)DEFINE_OP, DELIM_LIST, END_PROD,
                            END_ARR};

static vartype_t Left_roundb[] = {(vartype_t)L_ROUNDB, END_PROD,
                            DELIM_LIST, (vartype_t)L_ROUNDB, END_PROD,
                            (vartype_t)L_ROUNDB, DELIM_LIST, END_PROD,
                            DELIM_LIST, (vartype_t)L_ROUNDB, DELIM_LIST, END_PROD,
                            END_ARR};

static vartype_t Right_roundb[] = {(vartype_t)R_ROUNDB, END_PROD,
                            DELIM_LIST, (vartype_t)R_ROUNDB, END_PROD,
                            (vartype_t)R_ROUNDB, DELIM_LIST, END_PROD,
                            DELIM_LIST, (vartype_t)R_ROUNDB, DELIM_LIST, END_PROD,
                            END_ARR};

static vartype_t Left_squareb[] = {(vartype_t)L_SQUAREB, END_PROD,
                            DELIM_LIST, (vartype_t)L_SQUAREB, END_PROD,
                            (vartype_t)L_SQUAREB, DELIM_LIST, END_PROD,
                            DELIM_LIST, (vartype_t)L_SQUAREB, DELIM_LIST, END_PROD,
                            END_ARR};

static vartype_t Right_squareb[] = {(vartype_t)R_SQUAREB, END_PROD,
                            DELIM_LIST, (vartype_t)R_SQUAREB, END_PROD,
                            (vartype_t)R_SQUAREB, DELIM_LIST, END_PROD,
                            DELIM_LIST, (vartype_t)R_SQUAREB, DELIM_LIST, END_PROD,
                            END_ARR};

static vartype_t End_statement[] = {(vartype_t)END_STMT, END_PROD,
                            DELIM_LIST, (vartype_t)END_STMT, END_PROD,
                            (vartype_t)END_STMT, DELIM_LIST, END_PROD,
                            DELIM_LIST, (vartype_t)END_STMT, DELIM_LIST, END_PROD,
                            END_ARR};

static vartype_t Arg_separator[] = {(vartype_t)ARGSTOP, END_PROD,
                            DELIM_LIST, (vartype_t)ARGSTOP, END_PROD,
                            (vartype_t)ARGSTOP, DELIM_LIST, END_PROD,
                            DELIM_LIST, (vartype_t)ARGSTOP, DELIM_LIST, END_PROD,
                            END_ARR};

static vartype_t Number[] = {(vartype_t)NUMBER, END_PROD,
                            DELIM_LIST, (vartype_t)NUMBER, END_PROD,
                            (vartype_t)NUMBER, DELIM_LIST, END_PROD,
                            DELIM_LIST, (vartype_t)NUMBER, DELIM_LIST, END_PROD,
                            END_ARR};

static vartype_t Name[] = {(vartype_t)NAME, END_PROD,
                            DELIM_LIST, (vartype_t)NAME, END_PROD,
                            (vartype_t)NAME, DELIM_LIST, END_PROD,
                            DELIM_LIST, (vartype_t)NAME, DELIM_LIST, END_PROD,
                            END_ARR};

static vartype_t String[] = {(vartype_t)STRING, END_PROD,
                            DELIM_LIST, (vartype_t)STRING, END_PROD,
                            (vartype_t)STRING, DELIM_LIST, END_PROD,
                            DELIM_LIST, (vartype_t)STRING, DELIM_LIST, END_PROD,
                            END_ARR};

static vartype_t Char[] = {(vartype_t)CHAR, END_PROD,
                            DELIM_LIST, (vartype_t)CHAR, END_PROD,
                            (vartype_t)CHAR, DELIM_LIST, END_PROD,
                            DELIM_LIST, (vartype_t)CHAR, DELIM_LIST, END_PROD,
                            END_ARR};

/*** REAL GRAMMAR PRODUCTIONS ***/
static vartype_t Parameter[] = {NAME_VAR, END_PROD,
                            NUMBER_VAR, END_PROD,
                            STRING_VAR, END_PROD,
                            CHAR_VAR, END_PROD,
                            END_ARR};

static vartype_t ParamList[] = {PARAMETER, ARGSEPARATOR, PARAMLIST, END_PROD,
                            PARAMETER, END_PROD,
                            END_ARR};

static vartype_t BaseCall[] = {NAME_VAR, L_ROUNDB_VAR, PARAMLIST, R_ROUNDB_VAR, END_PROD,
                            END_ARR};

static vartype_t BaseCallList[] = {BASECALL, ARGSEPARATOR, BASECALL_LIST, END_PROD,
                            BASECALL, END_PROD,
                            END_ARR};

static vartype_t StepCall[] = {NAME_VAR, L_ROUNDB_VAR, STEPCALL, R_ROUNDB_VAR, END_PROD,
                            BASECALL_LIST, END_PROD,
                            END_ARR};

static vartype_t StepCallList[] = {STEPCALL, ARGSEPARATOR, STEPCALL_LIST, END_PROD,
                            STEPCALL, END_PROD,
                            END_ARR};

static vartype_t TrueCall[] = {NAME_VAR, L_ROUNDB_VAR, STEPCALL_LIST, R_ROUNDB_VAR, END_PROD,
                            BASECALL, END_PROD,
                            END_ARR};

static vartype_t Definition[] = {BASECALL, DEFINE_OP_VAR, TRUECALL, END_PROD,
                            END_ARR};

static vartype_t Statement[] = {DEFINITION, END_STMT_VAR, END_PROD,
                            BASECALL, END_STMT_VAR, END_PROD,
                            END_ARR};

static vartype_t StatementList[] = {STATEMENT, STATEMENTLIST, END_PROD,
                            STATEMENT, END_PROD,
                            END_ARR};

static vartype_t Program[] = {STATEMENTLIST, END_PROD, END_ARR};

/*** Mapping Enum to production arrays ***/
static vartype_t* production_map[] = {DelimList, Define_OP, Left_roundb, Right_roundb,
                            Left_squareb, Right_squareb, End_statement, Arg_separator,
                            Number, Name, String, Char, Parameter, ParamList,
                            BaseCall, BaseCallList, StepCall, StepCallList, TrueCall, Definition, Statement,
                            StatementList, Program};

static int parser_ast_recursive(ast_t* ast, toklist_t* token_list, size_t* index);
static int parser_graph_recursive(ast_t* ast, FILE* f);

int parser_ast(ast_t* ast, toklist_t* token_list){
    size_t index = 0;
    ast->vardual.vartype = PROGRAM;
    ast->tk = NULL;
    ast->tl_len = 0;
    ast->tl_capacity = 0;
    ast->tl = NULL;
    
    ERROR_RETHROW(parser_ast_recursive(ast, token_list, &index),

        fprintf(stderr, "[!] Error: failed at token %lu\n", index);
    );

    return 0;
}

void parser_ast_delete(ast_t* ast){
    if(ast == NULL)
    {
        return;
    }

    size_t i;
    for (i=0; i<ast->tl_len; ++i)
    {
        parser_ast_delete(&ast->tl[i]);
    }

    if (ast->tk != NULL)
    {
        free(ast->tk);
    }
    
    if (ast->tl != NULL)
    {
        free(ast->tl);
    }

    bzero(ast, sizeof(ast_t));
}

int parser_ast_graph(ast_t* ast, const char* filename){
    FILE* f;
    if ((f = fopen(filename, "w")) == NULL)
        return -1;
    
    fputs("digraph G{", f);
    
    if (parser_graph_recursive(ast, f) != 0){
        fclose(f);
        return -1;
    }

    fputs("}", f);
    fclose(f);
    return 0;    
}

static int parser_graph_recursive(ast_t* ast, FILE* f){
    if (ast->vardual.toktype > NOTOK)
        fprintf(f, "%lu [label=\"%s\", style=\"filled\", fillcolor=\"white\", shape=\"oval\"]\n", (uintptr_t)ast, parser_vartypestr(ast->vardual.vartype));
    else{
        fprintf(f, "%lu [xlabel=\"%s\", label=\"%s\", style=\"filled\", fillcolor=\"red\", shape=\"oval\"]\n", (uintptr_t)ast, tokenizer_typetokstr(ast->vardual.toktype), ast->tk);
    }

    size_t i;
    for (i=0; i<ast->tl_len; ++i){
        if (parser_graph_recursive(&ast->tl[i], f) != 0)
            return -1;
        fprintf(f, "%lu -> %lu\n", (uintptr_t)ast, (uintptr_t)&ast->tl[i]);
    }

    return 0;
}

const char* parser_vartypestr(vartype_t vartype){
    switch(vartype){
        case DELIM_LIST:
            return "DelimList";
        case DEFINE_OP_VAR:
            return "Define Var";
        case L_ROUNDB_VAR:
            return "Left Round Bracket";
        case R_ROUNDB_VAR:
            return "Right Round Bracket";
        case L_SQUAREB_VAR:
            return "Left Square Bracket";
        case R_SQUAREB_VAR:
            return "Right Square Bracket";
        case END_STMT_VAR:
            return "End Statement Var";
        case ARGSEPARATOR:
            return "Argument Separator";
        case NUMBER_VAR:
            return "Number Var";
        case STRING_VAR:
            return "String Var";
        case CHAR_VAR:
            return "Char Var";
        case PARAMETER:
            return "Parameter";
        case PARAMLIST:
            return "ParamList";
        case BASECALL:
            return "BaseCall";
        case BASECALL_LIST:
            return "BaseCall_List";
        case STEPCALL:
            return "StepCall";
        case STEPCALL_LIST:
            return "StepCall_List";
        case TRUECALL:
            return "TrueCall";
        case DEFINITION:
            return "Definition";
        case STATEMENT:
            return "Statement";
        case STATEMENTLIST:
            return "Statement List";
        case PROGRAM:
            return "Program";
        default:
            return NULL;
    }
}

static int parser_ast_expand(ast_t* ast){
    ast_t* new_tl;
    if ((new_tl = reallocarray(ast->tl, ast->tl_capacity*2, sizeof(ast_t))) == NULL)
    {
        return BAD_ALLOCATION;
    }
    
    ast->tl = new_tl;
    ast->tl_capacity *= 2;
    return 0;
}

static int parser_ast_recursive(ast_t* ast, toklist_t* token_list, size_t* index){
    
    // If leaf token
    if (ast->vardual.toktype < NOTOK)
    {
        // Check if the token type is in the production
        if (token_list->list[*index].tt == ast->vardual.toktype)
        {
                       
            // steal token from token_list
            ast->tk = token_list->list[*index].tk;
            token_list->list[*index].tk = NULL;

            ast->tl_len = 0;
            ast->tl_capacity = 0;
            ast->tl = NULL;

            ++(*index);

        }else
        {
            
            return NOT_A_PRODUCTION;
        }

        return OK;
    }

    bool match = false;
    size_t i = 0;
    union vardual_t var;

    ast_t* new_tl;
    if ((new_tl = calloc(10, sizeof(ast_t))) == NULL)
    {
        return BAD_ALLOCATION;
    }

    ast->tl = new_tl;
    ast->tk = NULL;
    ast->tl_len = 0;
    ast->tl_capacity = 10;

    // Take the first variable from the productions array
    var.vartype = (production_map[indexer(ast->vardual.vartype)])[i];
    
    // Loop until the end of the productions array
    while (var.vartype != END_ARR && !match){

        // Loop until the end of the current production or skip it if incorrect
        bool skip_prod = false;
        while (var.vartype != END_PROD){
            
            if (!skip_prod){
                
                // verify capacity
                if (ast->tl_len >= ast->tl_capacity)
                {
                    
                    ERROR_RETHROW(parser_ast_expand(ast),
    
                        parser_ast_delete(ast);
                        return BAD_ALLOCATION;
                    );
                }
                
                // put the vartype of the next production down the tree
                ast->tl[ast->tl_len].vardual.vartype = var.vartype;
                 
                int error_code;
                if ((error_code = parser_ast_recursive(&ast->tl[ast->tl_len], token_list, index)) == OK)
                {
                    ++ast->tl_len;
                    match = true; 
                }
                else if(error_code == NOT_A_PRODUCTION)
                {
                    skip_prod = true;
                    match = false;
                }
                else
                {
                    // ERROR
                    parser_ast_delete(ast);
                    return error_code;
                }
            }

            var.vartype = (production_map[indexer(ast->vardual.vartype)])[++i];
        }

        var.vartype = (production_map[indexer(ast->vardual.vartype)])[++i];
    }
    
    if (!match)
		{
        free(ast->tl);
			  ast->tl = NULL;
			  ast->tl_len = 0;
        return NOT_A_PRODUCTION;
    }

    return OK;
}

// undo ast progress on a certain branch
static void parser_ast_recursive_undo(ast_t* branch, toklist_t* token_list, size_t* index)
{
    if (branch->vardual.toktype < NOTOK)
    {
        // give back token to token_list
        branch->tk = token_list->list[*index].tk;
        token_list->list[*index].tk = NULL;

        // decrease the token_list index
        --(*index);
        return;
    }

    // undo for all sub-branches starting from the right
    size_t i;
    for (i=branch->tl_len; i > 0; --i)
    {
        parser_ast_recursive_undo(&branch->tl[i-1], token_list, index);
    }

    branch->tl_len = 0;
}
