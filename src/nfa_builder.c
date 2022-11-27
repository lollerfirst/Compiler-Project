#include <nfa_builder.h>
#include <stdlib.h>
#include <compiler_errors.h>

#ifdef _DEBUG
#include <assert.h>
#include <stdio.h>
#endif

static int nfa_simple(nfa_t**, char);
static int nfa_concat(nfa_t** restrict, nfa_t* restrict);
static int nfa_union(nfa_t** restrict, nfa_t* restrict);
static int nfa_star(nfa_t*);
static int nfa_init(nfa_t**, ssize_t);
static void nfa_deinit(nfa_t*);
static int nfa_state_init(state_t*, bool);
static void nfa_state_deinit(state_t*);
static int nfa_state_addsymbol(state_t*, char, int);
static int nfa_state_extend(state_t*);
static int nfa_delta(nfa_t*, char);

/*** EXPORTED ***/

int nfa_build(nfa_t** nfa_left, const node_t* parse_tree){
    nfa_t *nfa_right;
    
    switch(parse_tree->op){

        case NONE:
            ERROR_RETHROW(nfa_simple(nfa_left, parse_tree->c));
            break;

        case CONCAT:

            //check on the integrity of the parse tree
            #ifdef _DEBUG
            assert(parse_tree->l_child != NULL && parse_tree->r_child != NULL);
            #endif

            ERROR_RETHROW(nfa_build(nfa_left, parse_tree->l_child));
            ERROR_RETHROW(nfa_build(&nfa_right, parse_tree->r_child), nfa_destroy(*nfa_left)); 
            ERROR_RETHROW(nfa_concat(nfa_left, nfa_right), nfa_destroy(*nfa_left); nfa_destroy(nfa_right));

    
            break;

        case UNION:
            
            //check on the integrity of the parse tree
            #ifdef _DEBUG
            assert(parse_tree->l_child != NULL && parse_tree->r_child != NULL);
            #endif

            ERROR_RETHROW(nfa_build(nfa_left, parse_tree->l_child));
            ERROR_RETHROW(nfa_build(&nfa_right, parse_tree->r_child), nfa_destroy(*nfa_left));
            ERROR_RETHROW(nfa_union(nfa_left, nfa_right), nfa_destroy(*nfa_left); nfa_destroy(nfa_right));
            
            break;

        case STAR:
            #ifdef _DEBUG
            assert(parse_tree->l_child != NULL);
            #endif

            ERROR_RETHROW(nfa_build(nfa_left, parse_tree->l_child));
            ERROR_RETHROW(nfa_star(*nfa_left), nfa_destroy(*nfa_left));
            
        default: break;
    }

    return 0;
}

int nfa_accepts(nfa_t* nfa, bool* result, const char* string){
    *result = false;

    int i;
    if ((nfa->current_states = malloc(sizeof(int) * nfa->states_len)) == NULL)
    {
        return BAD_ALLOCATION;
    }

    nfa->current_states_capacity = nfa->states_len;

    // INITIAL STATE
    nfa->current_states[0] = 0;
    nfa->current_states_len = 1;
    
    for (i=0; string[i] != '\0'; ++i)
    {
        ERROR_RETHROW(
            nfa_delta(nfa, string[i]),
            free(nfa->current_states)
        );  
    }

    for (i=0; i < nfa->current_states_len; ++i)
    {

        int idx = nfa->current_states[i];
        if (nfa->states[idx].final)
        {
            *result = true;
        }
    }

    free(nfa->current_states);
    return OK;
}


void nfa_destroy(nfa_t** nfa){

    if (nfa == NULL)
    {
        return;
    }

    if (*nfa == NULL)
    {
        return;
    }

    int i;
    for (i=0; i<(*nfa)->states_len; ++i)
        nfa_state_deinit(&(*nfa)->states[i]);
    
    nfa_deinit(nfa);
    *nfa = NULL;
}

int nfa_graph(const nfa_t* nfa){
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


static int nfa_delta(nfa_t* nfa, char c){
    int* next_states;
    if ((next_states = malloc(sizeof(int) * nfa->current_states_len)) == NULL)
        return -1;

    ssize_t next_states_len = 0;
    ssize_t next_states_capacity = nfa->current_states_len;

    int i;
    for (i=0; i<nfa->current_states_len; ++i){
        const int k = nfa->current_states[i];

        int j;        
        for (j=0; j<nfa->states[k].len; ++j){

            if (nfa->states[k].charset[j] == c)
            {
                
                if (next_states_len >= next_states_capacity)
                {
                    if ((next_states = reallocarray(next_states, next_states_capacity * 2, sizeof(int))) == NULL)
                    {
                        free(next_states);
                        return BAD_ALLOCATION;
                    }
                    next_states_capacity *= 2;
                }

                next_states[next_states_len++] = nfa->states[k].mapped_state[j];
            }
        }
    }

    free(nfa->current_states);
    nfa->current_states = next_states;
    nfa->current_states_len = next_states_len;
    nfa->current_states_capacity = next_states_capacity;

    return 0;

}

static int nfa_init(nfa_t** nfa, ssize_t n_states){
    nfa_t* tmp_nfa;
    if ((tmp_nfa = calloc(1, sizeof(nfa_t))) == NULL)
    {
        return BAD_ALLOCATION;
    }
    
    if ((tmp_nfa->states = calloc(n_states, sizeof(state_t))) == NULL)
    {
        free(tmp_nfa);
        return BAD_ALLOCATION;
    }
    
    *nfa = tmp_nfa;
    return 0;
}

static int nfa_simple(nfa_t** nfa, char c){
    nfa_t* temp;
    ERROR_RETHROW(nfa_init(&temp, 3));
    
    ERROR_RETHROW(
        nfa_state_init(&temp->states[0], false),
        nfa_destroy(temp);
    );
    
    ERROR_RETHROW(
        nfa_state_init(&temp->states[1], true),
        nfa_destroy(temp);
    );
    
    ERROR_RETHROW(
        nfa_state_addsymbol(&temp->states[0], c, 1),
        nfa_destroy(temp);
    );

    *nfa = temp;
    return 0;
}

static int nfa_concat(nfa_t** restrict nfa_left, nfa_t* restrict nfa_right){
    nfa_t *nfa1 = *nfa_left;
    nfa_t *nfa2 = nfa_right;

    // new nfa
    nfa_t* nfa;

    ERROR_RETHROW(
        nfa_init(&nfa, nfa1->states_len + nfa2->states_len - 1)
    );

    // MOVE nfa1 AS IS
    int i;
    for (i=0; i<nfa1->states_len; ++i){
        nfa->states[i] = nfa1->states[i];
    }

    // MOVE nfa2 LESS INITIAL STATE AND CHANGING STATES' NUMBERS
    int j;
    i = nfa->states_len - 1;
    for (j=nfa2->states_len - 1; j>0; --j){
        nfa->states[i] = nfa2->states[j];
        
        int l;
        for (l=0; l<nfa->states[i].len; ++l){
            nfa->states[i].mapped_state[l] += nfa1->states_len - 1;
        }
        
        --i;
    }

    // CONNECT FINAL STATES OF nfa1 TO nfa2
    for (i=0; i<nfa1->states_len; ++i){
        
        if ( nfa->states[i].final ){
            
            for (j=0; j<nfa2->states[0].len; ++j)
                ERROR_RETHROW(
                        nfa_state_addsymbol(
                                &nfa->states[i], nfa2->states[0].charset[j],
                                nfa2->states[0].mapped_state[j] + nfa1->states_len - 1
                                ),
                        nfa_destroy(nfa)
                );
            
            nfa->states[i].final = nfa2->states[0].final; //state is final only if the initial state of nfa2 was
        }
    }

    // CLEANING UP
    nfa_state_deinit(&nfa2->states[0]);
    nfa_deinit(nfa1);
    nfa_deinit(nfa2);

    *nfa_left = nfa;
    return 0;
}

static int nfa_union(nfa_t** restrict nfa_left, nfa_t* restrict nfa_right){
    nfa_t* nfa1 = *nfa_left;
    nfa_t* nfa2 = nfa_right;

    nfa_t* nfa;
    ERROR_RETHROW(nfa_init(&nfa, nfa1->states_len + nfa2->states_len - 1));

    // MOVE nfa1 AS IS
    int i;
    for (i=0; i<nfa1->states_len; ++i)
    {
        nfa->states[i] = nfa1->states[i];
    }

    // MOVE nfa2 LESS INITIAL STATE AND CHANGING STATES' NUMBERS
    int j;
    i = nfa->states_len - 1;
    for (j=nfa2->states_len - 1; j>0; --j)
    {
        nfa->states[i] = nfa2->states[j];
        
        int l;
        for (l=0; l<nfa->states[i].len; ++l)
            nfa->states[i].mapped_state[l] += nfa1->states_len-1;
        
        --i;
    }

    // INITIAL STATE OF NFA1 IS INITIAL STATE OF NFA2: COPY NFA2 INITIAL STATE TRANSITIONS INTO THE NEW INITIAL STATE
    for (i=0; i<nfa2->states[0].len; ++i){
        ERROR_RETHROW(
            nfa_state_addsymbol(
                &nfa->states[0], 
                nfa2->states[0].charset[i], 
                nfa2->states[0].mapped_state[i] + nfa1->states_len - 1
            ),
            nfa_destroy(nfa)
        );
    }        

    // CLEANING UP
    nfa_state_deinit(&nfa2->states[0]);
    nfa_deinit(nfa1);
    nfa_deinit(nfa2);

    return 0;
}

static int nfa_star(nfa_t* nfa){
    //COPY THE INITIAL STATE TRANSITIONS INTO EVERY FINAL STATE
    int i;
    for (i=1; i<nfa->states_len; ++i){
        if (nfa->states[i].final){
            int j;
            for (j=0; j<nfa->states[0].len; ++j)
            {
                ERROR_RETHROW(
                    nfa_state_addsymbol(
                        &nfa->states[i], 
                        nfa->states[0].charset[j], 
                        nfa->states[0].mapped_state[j]
                    )
                );
            }
        }
    }
    
    nfa->states[0].final = true; //Accept null strings
    return 0;
}

static void nfa_deinit(nfa_t* nfa){
    if (nfa != NULL && nfa->states != NULL){
        free(nfa->states);
        nfa->states = NULL;
        free(nfa);
    }
    
}

static int nfa_state_init(state_t* state, bool final){
    state->len = 0;
    state->capacity = 0;
    state->final = false;
    
    if ((state->charset = malloc(sizeof(char) * 5)) == NULL)
    {
        return BAD_ALLOCATION;
    }

    if ((state->mapped_state = malloc(sizeof(int) * 5)) == NULL)
    {
        free(state->charset);
        return BAD_ALLOCATION;
    }
    
    state->capacity = 5;
    state->final = final;
    return OK;
}

static int nfa_state_extend(state_t* state){
    size_t new_capacity = state->capacity * 2;

    if ((state->charset = reallocarray(state->charset, new_capacity, sizeof(char))) == NULL)
    {
        return BAD_ALLOCATION;
    }

    if ((state->mapped_state = reallocarray(state->mapped_state, new_capacity, sizeof(int))) == NULL)
    {
        free(state->charset);
        return BAD_ALLOCATION;
    }

    state->capacity = new_capacity;
    return 0;
}

static int nfa_state_addsymbol(state_t* state, char c, int ns){
    if (state->len >= state->capacity)
    {
        ERROR_RETHROW(nfa_state_extend(state));
    }

    state->charset[state->len] = c;
    state->mapped_state[state->len] = ns;
    ++state->len;

    return 0;
}

static void nfa_state_deinit(state_t* state){
    if (state->charset != NULL) {
        free(state->charset);
    }

    if (state->mapped_state != NULL) {
        free(state->mapped_state);
    }

    state->charset = NULL;
    state->mapped_state = NULL;

    state->capacity = 0;
    state->len = 0;
    state->final = false;
}