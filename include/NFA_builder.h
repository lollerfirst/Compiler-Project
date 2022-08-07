#pragma once

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
    int len;
    char* charset;
    int* mapped_state;
    bool final;
} state_t;

/*
    type definition of the NFA
*/
typedef struct _nfa{
    int states_len;
    int current_states_len;
    state_t* states;
    int* current_states;
} NFA_t;

// Builds the NFA corresponding to the passed parse-tree.
NFA_t NFA_build(const node_t* _parse_tree);
// Checks if the NFA accepts a particular string
bool NFA_accepts(const char* string);