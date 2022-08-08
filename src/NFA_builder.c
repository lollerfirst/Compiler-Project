#include <NFA_builder.h>
#include <assert.h>
#include <stdlib.h>

static NFA_t NFA_simple(char);
static NFA_t NFA_concat(NFA_t, NFA_t);
static NFA_t NFA_union(NFA_t, NFA_t);
static NFA_t NFA_star(NFA_t);
static NFA_t NFA_init(int);
static int NFA_state_init(state_t*, bool);
static void NFA_state_deinit(state_t*);
static int NFA_state_addsymbol(state_t*, char, int);
static int NFA_state_extend(state_t*);

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
            break;

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
            #ifdef _DEBUG
            assert(parse_tree->l_child != NULL);
            #endif
            nfa = NFA_build(parse_tree->l_child);
            nfa = NFA_star(nfa);
        
        default: break;
    }

    return nfa;
}

bool NFA_accepts(NFA_t nfa, const char* string){
    if (nfa.current_states = malloc(sizeof(int) * nfa.states_len) == NULL)
        return false;
    
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
    
    NFA_state_init(&nfa.states[0], false);
    NFA_state_init(&nfa.states[1], true);

    NFA_state_addsymbol(&nfa.states[0], c, 1);
}

static NFA_t NFA_concat(NFA_t nfa1, NFA_t nfa2){
    NFA_t nfa = NFA_init(nfa1.states_len + nfa2.states_len - 1);

    // MOVE NFA1 AS IS
    int i;
    for (i=0; i<nfa1.states_len; ++i){
        nfa.states[i] = nfa1.states[i];
    }

    // MOVE NFA2 LESS INITIAL STATE AND CHANGING STATES' NUMBERS
    int j;
    int i = nfa.states_len - 1;
    for (j=nfa2.states_len - 1; j>0; --j){
        nfa.states[i] = nfa2.states[j];
        
        int l;
        for (l=0; l<nfa.states[i].len; ++l)
            nfa.states[i].mapped_state[l] += nfa1.states_len - 1;
        
        
        --i;
    }

    // CONNECT FINAL STATES OF NFA1 TO NFA2
    for (i=0; i<nfa1.states_len; ++i)
        if ( nfa.states[i].final ){
            for (j=0; j<nfa2.states[0].len; ++j)
                NFA_state_addsymbol(&nfa.states[i], nfa2.states[0].charset[j], nfa2.states[0].mapped_state[j] + nfa1.states_len - 1);
            
            nfa.states[i].final = false;
        };

    // CLEANING UP
    NFA_state_deinit(&nfa2.states[0]);
    NFA_deinit(nfa1);
    NFA_deinit(nfa2);
    return nfa;
}

static NFA_t NFA_union(NFA_t nfa1, NFA_t nfa2){
    NFA_t nfa = NFA_init(nfa1.states_len + nfa2.states_len - 1);

    // MOVE NFA1 AS IS
    int i;
    for (i=0; i<nfa1.states_len; ++i){
        nfa.states[i] = nfa1.states[i];
    }

    // MOVE NFA2 LESS INITIAL STATE AND CHANGING STATES' NUMBERS
    int j;
    int i = nfa.states_len - 1;
    for (j=nfa2.states_len - 1; j>0; --j){
        nfa.states[i] = nfa2.states[j];
        
        int l;
        for (l=0; l<nfa.states[i].len; ++l)
            nfa.states[i].mapped_state[l] += nfa1.states_len - 1;
        
        
        --i;
    }

    // INITIAL STATE OF NF1 IS INITIAL STATE OF NF2: COPY DFA2 INITIAL STATE TRANSITIONS INTO THE NEW INITIAL STATE
    for (i=0; i<nfa2.states[0].len; ++i)
        NFA_state_addsymbol(&nfa.states[0], nfa2.states[0].charset[i], nfa2.states[0].mapped_state[i] + nfa1.states_len - 1);

    // CLEANING UP
    NFA_state_deinit(&nfa2.states[0]);
    NFA_deinit(nfa1);
    NFA_deinit(nfa2);

    return nfa;
}

static NFA_t NFA_star(NFA_t nfa){
    //COPY THE INITIAL STATE TRANSITIONS INTO EVERY FINAL STATE
    int i;
    for (i=1; i<nfa.states_len; ++i){
        if (nfa.states[i].final){
            int j;
            for (j=0; j<nfa.states[0].len; ++j)
                NFA_state_addsymbol(&nfa.states[i], nfa.states[0].charset[j], nfa.states[0].mapped_state[j]);
        }
    }

    return nfa;
}

static void NFA_deinit(NFA_t nfa){
    if (nfa.states != NULL)
        free(nfa.states);
    if (nfa.current_states != NULL)
        free(nfa.current_states);
}

static int NFA_state_init(state_t* state, bool final){
    state->len = 0;
    state->capacity = 0;
    state->final = false;
    
    if ((state->charset = malloc(sizeof(char) * 5)) == NULL)
        return -1;
    if ((state->mapped_state = malloc(sizeof(int) * 5)) == NULL)
        return -1;
    
    state->capacity = 5;
    state->final = final;
    return 0;
}

static int NFA_state_extend(state_t* state){
    size_t new_capacity = state->capacity * 2;

    if ((state->charset = reallocarray(state->charset, new_capacity, sizeof(char))) == NULL)
        return -1;
    if ((state->mapped_state = reallocarray(state->mapped_state, new_capacity, sizeof(int))) == NULL)
        return -1;

    state->capacity = new_capacity;
    return 0;
}

static int NFA_state_addsymbol(state_t* state, char c, int ns){
    if (state->len >= state->capacity)
        if (NFA_state_extend(state) != 0)
            return -1;
    
    state->charset[state->len] = c;
    state->mapped_state[state->len] = ns;
    ++state->len;

    return 0;
}

static void NFA_state_deinit(state_t* state){
    if (state->charset != NULL)
        free(state->charset);
    if (state->mapped_state != NULL)
        free(state->mapped_state);

    state->charset = NULL;
    state->mapped_state = NULL;

    state->capacity = 0;
    state->len = 0;
    state->final = false;
}