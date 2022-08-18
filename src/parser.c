#include <parser.h>

/*** DEFINING THE GRAMMAR ***/
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