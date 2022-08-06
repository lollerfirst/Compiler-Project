#include <DFA_builder.h>
#include <memory.h>

#define I(_i, _j) (_i*ASCII_LEN+_j)
#define DFA_NON_FINAL __INT32_MAX__

static DFA_t DFA_simple(char c);
static DFA_t DFA_concat(DFA_t dfa1, DFA_t dfa2);
static DFA_t DFA_union(DFA_t dfa1, DFA_t dfa2);
static DFA_t DFA_star(DFA_t dfa);
static DFA_t DFA_init(int n_states);
static void DFA_deinit(DFA_t dfa);
static void DFA_finalize(DFA_t dfa);

DFA_t DFA_build(const node_t* parse_tree){
    DFA_t dfa = {0};
    
    switch(parse_tree->op){

        case NONE:
            dfa = DFA_simple(parse_tree->c);
            break;

        case CONCAT:
            DFA_t dfa1;

            //check on the integrity of the parse tree
            #ifdef _DEBUG
            assert(parse_tree->l_child != NULL && parse_tree->r_child);
            #endif

            

        case UNION:
            DFA_t dfa1;
            
            //check on the integrity of the parse tree
            #ifdef _DEBUG
            assert(parse_tree->l_child != NULL && parse_tree->r_child);
            #endif

            dfa = DFA_build(parse_tree->l_child);
            dfa1 = DFA_build(parse_tree->r_child);
            dfa = DFA_union(dfa, dfa1);
            break;

        case STAR:
            dfa = DFA_build(parse_tree->l_child);
            dfa = DFA_star(dfa);
        
        default: break;
    }

    return dfa;
}

static DFA_t DFA_init(int n_states){
    DFA_t dfa = {0};
    const size_t size = sizeof(int)*n_states*ASCII_LEN;

    if (n_states <= 0)
        return dfa;

    dfa.n_states = n_states;
    dfa.transition_table = malloc(size);
    dfa.final_states_table = malloc(sizeof(int)*n_states);

    memset(dfa.transition_table, DFA_NON_FINAL, size);
    return dfa;
}

static DFA_t DFA_simple(char c){
    DFA_t dfa = DFA_init(2);

    dfa.transition_table[I(0, c)] = 1;

    dfa.final_states_table[0] = false;
    dfa.final_states_table[1] = true;

    return dfa;
}

static DFA_t DFA_concat(DFA_t dfa1, DFA_t dfa2){
    DFA_t dfa = DFA_init(dfa1.n_states + dfa2.n_states - 1);

    // Copy DFA1 as is
    memcpy(dfa.transition_table, dfa1.transition_table, dfa.n_states*ASCII_LEN*sizeof(int));

    // Copy DFA2 changing table conn points
    const int dfa1_last_state = dfa1.n_states - 1;

    int i;
    int j;
    for (i = dfa1_last_state; i < dfa.n_states; ++i){
        if ()
    }
}