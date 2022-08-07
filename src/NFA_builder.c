#include <NFA_builder.h>

static NFA_t NFA_simple(char);
static NFA_t NFA_concat(NFA_t, NFA_t);
static NFA_t NFA_union(NFA_t, NFA_t);
static NFA_t NFA_star(NFA_t);
static void NFA_destroy(NFA_t);

NFA_t NFA_build(const node_t* parse_tree){
    NFA_t nfa = {0};
    
    switch(parse_tree->op){

        case NONE:
            nfa = NFA_simple(parse_tree->c);
            break;

        case CONCAT:
            NFA_t nfa1;

            //check on the integrity of the parse tree
            #ifdef _DEBUG
            assert(parse_tree->l_child != NULL && parse_tree->r_child);
            #endif

            nfa = NFA_build(parse_tree->l_child);
            nfa1 = NFA_build(parse_tree->r_child);
            nfa = NFA_union(nfa, nfa1);

        case UNION:
            NFA_t nfa1;
            
            //check on the integrity of the parse tree
            #ifdef _DEBUG
            assert(parse_tree->l_child != NULL && parse_tree->r_child);
            #endif

            nfa = NFA_build(parse_tree->l_child);
            nfa1 = NFA_build(parse_tree->r_child);
            nfa = NFA_union(nfa, nfa1);
            break;

        case STAR:
            nfa = NFA_build(parse_tree->l_child);
            nfa = NFA_star(nfa);
        
        default: break;
    }

    return nfa;
}

static NFA_t NFA_init(int n_states){
    NFA_t nfa = {0};
    if ( nfa.states = malloc(sizeof(state_t)*n_states) )
        return nfa;
    
    nfa.states_len = n_states;
    return nfa;
}

static NFA_t NFA_simple(char c){
    NFA_t nfa = NFA_init(2);

    if ( (nfa.states[0].charset = malloc(sizeof(char))) == NULL &&
         (nfa.states[0].mapped_state = malloc(sizeof(int))) == NULL ){
        
        NFA_deinit(nfa);
        return nfa;
    }
    
    nfa.states[0].charset[0] = c;
    nfa.states[0].mapped_state[0] = 1;
    
    nfa.states[1].charset = NULL;
    nfa.states[1].mapped_state = NULL;

    nfa.states[0].len = 1;
    nfa.states[0].final = false;

    nfa.states[1].len = 0;
    nfa.states[1].final = true;
}
