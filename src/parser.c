#include <parser.h>

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
static vartype_t Parameter = {NAME_VAR, END_PROD,
                            NUMBER_VAR, END_PROD,
                            STRING_VAR, END_PROD,
                            CHAR_VAR, END_PROD,
                            END_ARR};

static vartype_t ParamList = {PARAMETER, ARGSEPARATOR, PARAMLIST, END_PROD,
                            PARAMETER, END_PROD,
                            END_ARR};

static vartype_t BaseCall = {NAME_VAR, L_ROUNDB_VAR, PARAMLIST, R_ROUNDB_VAR, END_PROD,
                            END_ARR};

static vartype_t BaseCallList = {BASECALL, ARGSEPARATOR, BASECALL_LIST, END_PROD,
                            BASECALL, END_PROD,
                            END_ARR};

static vartype_t StepCall = {NAME_VAR, L_ROUNDB_VAR, STEPCALL, R_ROUNDB_VAR, END_PROD,
                            BASECALL_LIST, END_PROD,
                            END_ARR};

static vartype_t StepCallList = {STEPCALL, ARGSEPARATOR, STEPCALL_LIST, END_PROD,
                            STEPCALL, END_PROD,
                            END_ARR};

static vartype_t TrueCall = {NAME_VAR, L_ROUNDB_VAR, STEPCALL_LIST, R_ROUNDB_VAR, END_PROD,
                            BASECALL, END_PROD,
                            END_ARR};

static vartype_t Definition = {BASECALL, DEFINE_OP_VAR, TRUECALL, END_PROD,
                            END_ARR};

static vartype_t Statement = {DEFINITION, END_STMT_VAR, END_PROD,
                            TRUECALL, END_STMT_VAR, END_PROD,
                            END_ARR}; 

/*** Mapping Enum to production arrays ***/
static vartype_t* production_map[] = {};

static int parser_ast_rec(AST_t* ast, const toklist_t* token_list, size_t* index);
static int parser_graph_rec(AST_t* ast, FILE* f);

int parser_ast(AST_t* ast, const toklist_t* token_list){
    size_t index = 0;
    ast->vardual.vartype = PROGRAM;
    ast->tk = NULL;
    ast->tl_len = 0;
    ast->tl_capacity = 0;
    
    if (parser_ast_rec(ast, token_list, &index) != 0){
        fprintf(stderr, "[!] Error: failed at char %lu\n", index);
        return -1;
    }

    return 0;
}

void parser_free(AST_t* ast){
    if(ast == NULL)
        return;
    
    size_t i;
    for (i=0; i<ast->tl_len; ++i)
        parser_free(&ast->tl[i]);

    if (ast->tk != NULL)
        free(ast->tk);
    
    if (ast->tl != NULL)
        free(ast->tl);
}

int parser_graph(AST_t* ast, const char* filename){
    FILE* f;
    if ((f = fopen(filename, "w")) == NULL)
        return -1;
    
    fputs("digraph G{", f);
    
    if (parser_graph_rec(ast, f) != 0){
        fclose(f);
        return -1;
    }

    fputs("}", f);
    fclose(f);
    return 0;    
}

static int parser_graph_rec(AST_t* ast, FILE* f){
    if (ast->vardual.toktype > NOTOK)
        fprintf(f, "%lu [label=\"%s\", style=\"filled\", fillcolor=\"white\", shape=\"oval\"]\n", (uintptr_t)ast, parser_vartypestr(ast->vardual.vartype));
    else{
        fprintf(f, "%lu [xlabel=\"%s\", label=\"%s\", style=\"filled\", fillcolor=\"red\", shape=\"oval\"]\n", (uintptr_t)ast, tokenizer_typetokstr(ast->vardual.toktype), ast->tk);
    }

    size_t i;
    for (i=0; i<ast->tl_len; ++i){
        if (parser_graph_rec(&ast->tl[i], f) != 0)
            return -1;
        fprintf(f, "%lu -> %lu\n", (uintptr_t)ast, (uintptr_t)&ast->tl[i]);
    }

    return 0;
}

const char* parser_vartypestr(vartype_t vartype){
    switch(vartype){
        case DELIM_LIST:
            return "DelimList";
        case ASSIGN_OP_VAR:
            return "Assign Var";
        case ARITHM_OP_VAR:
            return "Arithm Var";
        case MUL_OP_VAR:
            return "Mul Var";
        case BOOL_OP_VAR:
            return "Bool Var";
        case L_ROUNDB_VAR:
            return "Left Round Bracket";
        case R_ROUNDB_VAR:
            return "Right Round Bracket";
        case L_SQUAREB_VAR:
            return "Left Square Bracket";
        case R_SQUAREB_VAR:
            return "Right Square Bracket";
        case L_CURLYB_VAR:
            return "Left Curly Bracket";
        case R_CURLYB_VAR:
            return "Right Curly Bracket";
        case END_STMT_VAR:
            return "End Statement Var";
        case ARGSEPARATOR:
            return "Argument Separator";
        case LOGIC_NOT_VAR:
            return "Logic Not Var";
        case IF_VAR:
            return "If Var";
        case WHILE_VAR:
            return "While Var";
        case BREAK_VAR:
            return "Break Var";
        case ELSE_VAR:
            return "Else Var";
        case RETURN_VAR:
            return "Return Var";
        case TYPE_VAR:
            return "Type Var";
        case NUMBER_VAR:
            return "Number Var";
        case VARIABLE:
            return "Variable";
        case STRING_VAR:
            return "String Var";
        case CHAR_VAR:
            return "Char Var";
        case ARRAY:
            return "Array";
        case EXPR:
            return "Expression";
        case VARLIST:
            return "Var List";
        case EXPR_LIST:
            return "Expr List";
        case CALL:
            return "Call";
        case MULEXPR:
            return "Mul Expression";
        case ARITHMEXPR:
            return "Arithm Expression";
        case ASSIGNEXPR:
            return "Assign Expression";
        case BOOLEXPR:
            return "Boolean Expression";
        case ALLEXPR:
            return "All Expression";
        case DECLARATION:
            return "Declaration";
        case STATEMENT:
            return "Statement";
        case STATEMENT_LIST:
            return "Statement List";
        case PROGRAM:
            return "Program";
        default:
            return NULL;
    }
}

static int parser_ast_expand(AST_t* ast){
    if ((ast->tl = reallocarray(ast->tl, ast->tl_capacity*2, sizeof(AST_t))) == NULL)
        return -1;
    
    ast->tl_capacity *= 2;
    return 0; 
}

static int parser_ast_rec(AST_t* ast, const toklist_t* token_list, size_t* index){
    
    // If leaf token
    if (ast->vardual.toktype <= NOTOK){
        if (token_list->list[*index].tt == ast->vardual.toktype){
            size_t str_len = strlen(token_list->list[*index].tk);
            
            if ((ast->tk = calloc(str_len+1, sizeof(char))) == NULL)
                return -1;
            
            strcpy(ast->tk, token_list->list[*index].tk);

            ast->tl_len = 0;
            ast->tl_capacity = 0;
            ast->tl = NULL;

            ++(*index);

        }else
            return -1;

        return 0;
    }

    bool match = false;
    size_t i = 0;
    union vardual_t var;

    if ((ast->tl = calloc(10, sizeof(AST_t))) == NULL)
        return -1;

    ast->tk = NULL;
    ast->tl_len = 0;
    ast->tl_capacity = 10;

    var.vartype = (production_map[I(ast->vardual.vartype)])[i];
    
    // Loop until the end of the productions array
    while (var.vartype != END_ARR && !match){

        // Loop until the end of the current production or skip it if incorrect
        bool skip_prod = false;
        while (var.vartype != END_PROD){
            
            if (!skip_prod){
                if (ast->tl_len >= ast->tl_capacity){
                    if (parser_ast_expand(ast) != 0){
                        parser_free(ast);
                        return -1;
                    }
                }

                ast->tl[ast->tl_len].vardual.vartype = var.vartype;
                
                if (parser_ast_rec(&ast->tl[ast->tl_len], token_list, index) == 0){
                    ++ast->tl_len;
                    match = true;
                }else{
                    skip_prod = true;
                    match = false;
                }
            }

            var.vartype = (production_map[I(ast->vardual.vartype)])[++i];
        }

        var.vartype = (production_map[I(ast->vardual.vartype)])[++i];
    }
    
    if (!match){
        parser_free(ast);
        return -1;
    }

    return 0;
}