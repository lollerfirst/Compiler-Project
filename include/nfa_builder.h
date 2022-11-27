#ifndef _NFA_BUILDER_H
#define _NFA_BUILDER_H

#include <regexparse.h>
#include <stdbool.h>

#define ASCII_LEN 128

/* 
state_t serves as a map between characters and states.
charset contains the string of mapped characters and
mapped_states is the array containing the indices of the states.
In the event of a single character leading to multiple states, 
the character should be repeated in the charset.   
*/

typedef struct _state{
    ssize_t len;
    ssize_t capacity;
    char* charset;
    int* mapped_state;
    bool final;
} state_t;

/*
    type definition of the NFA
*/
typedef struct _nfa{
    ssize_t states_len;
    ssize_t current_states_len;
    ssize_t current_states_capacity;
    state_t* states;
    int* current_states;
} nfa_t;

// Builds the NFA corresponding to the passed parse-tree.
int nfa_build(nfa_t** nfa, const node_t* _parse_tree);
// Save the NFA to disk
int nfa_save(nfa_t* nfa, const char* filename);
// Load NFA from disk
int nfa_load(nfa_t** nfa, const char* filename);
// Destroys the NFA
void nfa_destroy(nfa_t** nfa);
// Checks if the NFA accepts a particular string
int nfa_accepts(nfa_t* nfa, bool* result, const char* string);
// Prints the NFA in graphviz format to stdout
int nfa_graph(const nfa_t* nfa);

#endif