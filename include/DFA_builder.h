#pragma once

#include <regexparse.h>
#include <stdbool.h>

#define ASCII_LEN 128

typedef struct _dfa{
    int n_states;
    int* transition_table;
    bool* final_states_table;
} DFA_t;

DFA_t DFA_build(const node_t* _parse_tree);
bool DFA_accepts(const char* _regexpr);