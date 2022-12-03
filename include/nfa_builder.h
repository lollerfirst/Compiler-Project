#ifndef _NFA_BUILDER_H
#define _NFA_BUILDER_H

#include <regexparse.h>
#include <stdbool.h>
#include <stddef.h>

#define ASCII_LEN 128

/* 
state_t serves as a map between characters and states.
charset contains the string of mapped characters and
mapped_states is the array containing the indices of the states.
In the event of a single character leading to multiple states, 
the character should be repeated in the charset.   
*/

typedef struct _state{
    size_t len;
    size_t capacity;
    char* charset;
    int* mapped_state;
    bool final;
} state_t;

/*
    type definition of the NFA
*/
typedef struct _nfa{
    size_t states_len;
    size_t current_states_len;
    size_t current_states_capacity;
    state_t* states;
    int* current_states;
} nfa_t;

// Builds the NFA corresponding to the passed parse-tree.
int nfa_build(nfa_t* nfa, const node_t* _parse_tree);
// Save a NFA collection to disk
int nfa_collection_save(const nfa_t* nfa_collection, size_t count, const char* filename);
// Load NFA collection from disk into nfa, its length into len
int nfa_collection_load(nfa_t** nfa, size_t* len, const char* filename);
// Destroys the NFA
void nfa_destroy(nfa_t* nfa);
// Checks if the NFA accepts a particular string
int nfa_accepts(nfa_t* nfa, const char* string, bool* result);

/* DEBUG */
// Prints the NFA in graphviz format to stdout
int nfa_graph(const nfa_t* nfa);
// Compares 2 NFA and prints first found difference
void nfa_compare(const nfa_t* restrict nfa1, const nfa_t* restrict nfa2);
/* *** */

// Delete a collection loaded with nfa_collection_load
void nfa_collection_delete(nfa_t* nfa_list, size_t len);

#endif