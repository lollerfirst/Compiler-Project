#pragma once

#include <regexparse.h>
#include <stdbool.h>
#include <vector.h>

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
    state_t* states;
    Vector current_states;
} NFA_t;

// Builds the NFA corresponding to the passed parse-tree.
NFA_t NFA_build(const node_t* _parse_tree);
// Destroys the NFA
void NFA_destroy(NFA_t* nfa);
// Checks if the NFA accepts a particular string
bool NFA_accepts(NFA_t* nfa, const char* string);
// Prints the NFA in graphviz format to stdout
int NFA_graph(NFA_t nfa);
