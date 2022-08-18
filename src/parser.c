#include <parser.h>

/*** WRAPPING TOKENS INTO DELIMITERS ***/

static vartype_t DelimList[] = {DELIM, END_PROD,
                            DELIM, DELIM_LIST, END_PROD
};

static vartype_t Assign_OP[] = {ASSIGN_OP, END_PROD,
                            DELIM_LIST, ASSIGN_OP, END_PROD,
                            ASSIGN_OP, DELIM_LIST, END_PROD,
                            DELIM_LIST, ASSIGN_OP, DELIM_LIST, END_PROD
                            };

static vartype_t Arithm_OP[] = {ALGEBRAIC_OP, END_PROD,
                            DELIM_LIST, ALGEBRAIC_OP, END_PROD,
                            ALGEBRAIC_OP, DELIM_LIST, END_PROD,
                            DELIM_LIST, ALGEBRAIC_OP, DELIM_LIST, END_PROD
                            };

static vartype_t Mul_OP[] = {MUL_OP, END_PROD,
                            DELIM_LIST, ALGEBRAIC_OP, END_PROD,
                            ALGEBRAIC_OP, DELIM_LIST, END_PROD,
                            DELIM_LIST, ALGEBRAIC_OP, DELIM_LIST, END_PROD
                            };

static vartype_t Bool_OP[] = {BOOLEAN_OP, END_PROD,
                            DELIM_LIST, BOOLEAN_OP, END_PROD,
                            BOOLEAN_OP, DELIM_LIST, END_PROD,
                            DELIM_LIST, BOOLEAN_OP, DELIM_LIST, END_PROD
                            };

static vartype_t Left_roundb[] = {L_ROUNDB, END_PROD,
                            DELIM_LIST, L_ROUNDB, END_PROD,
                            L_ROUNDB, DELIM_LIST, END_PROD,
                            DELIM_LIST, L_ROUNDB, DELIM_LIST, END_PROD
                            };

static vartype_t Right_roundb[] = {R_ROUNDB, END_PROD,
                            DELIM_LIST, R_ROUNDB, END_PROD,
                            R_ROUNDB, DELIM_LIST, END_PROD,
                            DELIM_LIST, R_ROUNDB, DELIM_LIST, END_PROD
                            };

static vartype_t Left_squareb[] = {L_SQUAREB, END_PROD,
                            DELIM_LIST, L_SQUAREB, END_PROD,
                            L_SQUAREB, DELIM_LIST, END_PROD,
                            DELIM_LIST, L_SQUAREB, DELIM_LIST, END_PROD
                            };

static vartype_t Right_squareb[] = {R_SQUAREB, END_PROD,
                            DELIM_LIST, R_SQUAREB, END_PROD,
                            R_SQUAREB, DELIM_LIST, END_PROD,
                            DELIM_LIST, R_SQUAREB, DELIM_LIST, END_PROD
                            };

static vartype_t Left_curlyb[] = {L_CURLYB, END_PROD,
                            DELIM_LIST, L_CURLYB, END_PROD,
                            L_CURLYB, DELIM_LIST, END_PROD,
                            DELIM_LIST, L_CURLYB, DELIM_LIST, END_PROD
                            };

static vartype_t Right_curlyb[] = {R_CURLYB, END_PROD,
                            DELIM_LIST, R_CURLYB, END_PROD,
                            R_CURLYB, DELIM_LIST, END_PROD,
                            DELIM_LIST, R_CURLYB, DELIM_LIST, END_PROD
                            };

static vartype_t End_statement[] = {END_STMT, END_PROD,
                            DELIM_LIST, END_STMT, END_PROD,
                            END_STMT, DELIM_LIST, END_PROD,
                            DELIM_LIST, END_STMT, DELIM_LIST, END_PROD
                            };

static vartype_t Arg_separator[] = {ARGSTOP, END_PROD,
                            DELIM_LIST, ARGSTOP, END_PROD,
                            ARGSTOP, DELIM_LIST, END_PROD,
                            DELIM_LIST, ARGSTOP, DELIM_LIST, END_PROD
                            };

static vartype_t Logic_not[] = {LOGIC_NOT, END_PROD,
                            DELIM_LIST, LOGIC_NOT, END_PROD,
                            LOGIC_NOT, DELIM_LIST, END_PROD,
                            DELIM_LIST, LOGIC_NOT, DELIM_LIST, END_PROD
                            };

static vartype_t If_var[] = {IF, END_PROD,
                            DELIM_LIST, IF, END_PROD,
                            IF, DELIM_LIST, END_PROD,
                            DELIM_LIST, IF, DELIM_LIST, END_PROD
                            };

static vartype_t While_var[] = {WHILE, END_PROD,
                            DELIM_LIST, WHILE, END_PROD,
                            WHILE, DELIM_LIST, END_PROD,
                            DELIM_LIST, WHILE, DELIM_LIST, END_PROD
                            };

static vartype_t Break_var[] = {BREAK, END_PROD,
                            DELIM_LIST, BREAK, END_PROD,
                            BREAK, DELIM_LIST, END_PROD,
                            DELIM_LIST, BREAK, DELIM_LIST, END_PROD
                            };

static vartype_t Return_var[] = {RETURN, END_PROD,
                            DELIM_LIST, RETURN, END_PROD,
                            RETURN, DELIM_LIST, END_PROD,
                            DELIM_LIST, RETURN, DELIM_LIST, END_PROD
                            };

static vartype_t Type[] = {TYPE, END_PROD,
                            DELIM_LIST, TYPE, END_PROD,
                            TYPE, DELIM_LIST, END_PROD,
                            DELIM_LIST, TYPE, DELIM_LIST, END_PROD
                            };

static vartype_t Number[] = {NUMBER, END_PROD,
                            DELIM_LIST, NUMBER, END_PROD,
                            NUMBER, DELIM_LIST, END_PROD,
                            DELIM_LIST, NUMBER, DELIM_LIST, END_PROD
                            };

static vartype_t Variable[] = {NAME, END_PROD,
                            DELIM_LIST, NAME, END_PROD,
                            NAME, DELIM_LIST, END_PROD,
                            DELIM_LIST, NAME, DELIM_LIST, END_PROD
                            };

static vartype_t String[] = {STRING, END_PROD,
                            DELIM_LIST, STRING, END_PROD,
                            STRING, DELIM_LIST, END_PROD,
                            DELIM_LIST, STRING, DELIM_LIST, END_PROD
                            };

static vartype_t Char[] = {CHAR, END_PROD,
                            DELIM_LIST, CHAR, END_PROD,
                            CHAR, DELIM_LIST, END_PROD,
                            DELIM_LIST, CHAR, DELIM_LIST, END_PROD
                            };

/*** REAL GRAMMAR PRODUCTIONS ***/
static vartype_t Array[] = {VARIABLE, L_SQUAREB_VAR, NUMBER_VAR, R_SQUAREB_VAR, END_PROD};

static vartype_t Expr[] = {NUMBER_VAR, END_PROD,
                            STRING_VAR, END_PROD,
                            CHAR_VAR, END_PROD,
                            VARIABLE, END_PROD,
                            ARRAY, END_PROD,
                            L_ROUNDB_VAR, EXPR, R_ROUNDB_VAR};

static vartype_t VarList[] = {VARIABLE, ARGSEPARATOR, VARLIST, END_PROD,
                            ARRAY, ARGSEPARATOR, VARLIST, END_PROD,
                            VARIABLE, END_PROD,
                            ARRAY, END_PROD};

static vartype_t ExprList[] = {EXPR, END_PROD,
                            EXPR, ARGSEPARATOR, EXPR_LIST, END_PROD};

static vartype_t Call[] = {VARIABLE, L_ROUNDB_VAR, R_ROUNDB_VAR, END_PROD,
                            VARIABLE, L_ROUNDB_VAR, EXPR_LIST, R_ROUNDB_VAR, END_PROD};