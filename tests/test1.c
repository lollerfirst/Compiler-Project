#include <NFA_builder.h>
#include <regexparse.h>
#include <stdio.h>

static char regexpr[] = "(a+b+c+(abd(d*)efg)+h+i)*";

int main(){
    node_t* node = tree_parse(regexpr);
    tree_graph(node);
    NFA_t nfa = NFA_build(node);
    NFA_graph(nfa);
    
    char string[256];
    
    while(true){
        printf(">");
        int i = 0;
        fgets(string, 256, stdin);
        if (NFA_accepts(&nfa, string))
            printf("Accepted\n");
        else
            printf("Not accepted\n");
    }
    return 0;
}