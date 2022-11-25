/*

#include <regexparse.h>
#include <nfa_builder.h>

static const char* regexpr = "(0+1+2+3+4+5+6+7+8+9)((0+1+2+3+4+5+6+7+8+9)*)";

int main(){
    node_t* node = tree_parse(regexpr);
    tree_graph(node);
    NFA_t nfa = NFA_build(node);
    NFA_graph(&nfa);

    return 0;
}

*/