#include <NFA_builder.h>
#include <stdlib.h>

#ifdef _DEBUG
#include <assert.h>
#include <stdio.h>
#endif

static NFA_t NFA_simple(char);
static NFA_t NFA_concat(NFA_t, NFA_t);
static NFA_t NFA_union(NFA_t, NFA_t);
static NFA_t NFA_star(NFA_t);
static NFA_t NFA_init(int);
static void NFA_deinit(NFA_t);
static int NFA_state_init(state_t*, bool);
static void NFA_state_deinit(state_t*);
static int NFA_state_addsymbol(state_t*, char, int);
static int NFA_state_extend(state_t*);
static void NFA_delta(NFA_t*, char);

/*** EXPORTED ***/

NFA_t NFA_build(const node_t* parse_tree){
    NFA_t nfa = {0};
    NFA_t nfa1 = {0};
    
    switch(parse_tree->op){

        case NONE:
            nfa = NFA_simple(parse_tree->c);
            break;

        case CONCAT:

            //check on the integrity of the parse tree
            #ifdef _DEBUG
            assert(parse_tree->l_child != NULL && parse_tree->r_child != NULL);
            #endif

            nfa = NFA_build(parse_tree->l_child);
            nfa1 = NFA_build(parse_tree->r_child);
            nfa = NFA_concat(nfa, nfa1);
            break;

        case UNION:
            
            //check on the integrity of the parse tree
            #ifdef _DEBUG
            assert(parse_tree->l_child != NULL && parse_tree->r_child != NULL);
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

bool NFA_accepts(NFA_t* nfa, const char* string){
    int i;
    nfa->current_states = malloc(sizeof(int) * nfa->states_len);
    nfa->current_states_capacity = nfa->states_len;

    // INITIAL STATE
    nfa->current_states[0] = 0;
    nfa->current_states_len = 1;
    
    for (i=0; string[i] != '\0'; ++i)
        NFA_delta(nfa, string[i]);

    for (i=0; i < nfa->current_states_len; ++i){

        int idx = nfa->current_states[i];
        if (nfa->states[idx].final)
            return true;
    }

    free(nfa->current_states);
    return false;
}


void NFA_destroy(NFA_t* nfa){
    int i;
    for (i=0; i<nfa->states_len; ++i)
        NFA_state_deinit(&nfa->states[i]);
    
    NFA_deinit(*nfa);
}

int NFA_graph(const NFA_t* nfa){
	FILE* f;
	if ( (f = fopen("nfa_graph.gv", "w")) == NULL)
		return -1;
	
	fputs("digraph G{", f);
	int i;
    for (i=0; i<nfa->states_len; ++i){
        const char* bgcolor = (nfa->states[i].final) ? "red" : "white";
        fprintf(f, "%d [label=\"%d\", style=\"filled\", fillcolor=\"%s\", shape=\"oval\"]\n", i, i, bgcolor);
        
        int j;
        for (j=0; j<nfa->states[i].len; ++j){
            fprintf(f, "%d -> %d [label=\"%c\"]\n", i, nfa->states[i].mapped_state[j], nfa->states[i].charset[j]);
        }
    }
	fputs("}", f);
	
	fclose(f);
    return 0;
}

/*** INTERNAL ***/


static void NFA_delta(NFA_t* nfa, char c){
    int* next_states = malloc(sizeof(int) * nfa->current_states_len);
    ssize_t next_states_len = 0;
    ssize_t next_states_capacity = nfa->current_states_len;

    int i;
    for (i=0; i<nfa->current_states_len; ++i){
        const int k = nfa->current_states[i];

        int j;        
        for (j=0; j<nfa->states[k].len; ++j)

            if (nfa->states[k].charset[j] == c){
                
                if (next_states_len >= next_states_capacity){
                    next_states = reallocarray(next_states, next_states_capacity * 2, sizeof(int));
                    next_states_capacity *= 2;
                }

                next_states[next_states_len++] = nfa->states[k].mapped_state[j];
            }
    }

    free(nfa->current_states);
    nfa->current_states = next_states;
    nfa->current_states_len = next_states_len;
    nfa->current_states_capacity = next_states_capacity;

}

static NFA_t NFA_init(int n_states){
    NFA_t nfa = {0};
    if ( (nfa.states = malloc(sizeof(state_t)*n_states)) == NULL)
        return nfa;
    
    nfa.states_len = n_states;
    return nfa;
}

static NFA_t NFA_simple(char c){
    NFA_t nfa = NFA_init(2);
    
    NFA_state_init(&nfa.states[0], false);
    NFA_state_init(&nfa.states[1], true);

    NFA_state_addsymbol(&nfa.states[0], c, 1);
    return nfa;
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
    i = nfa.states_len - 1;
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
    i = nfa.states_len - 1;
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