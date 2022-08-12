#include <NFA_builder.h>
#include <regexparse.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv){
    if (argc != 2){
		puts("USAGE: ./test1 <regexpr>");
		return -1;
	}

    const char* regexpr = argv[1];

    node_t* node = tree_parse(regexpr);
    tree_graph(node);
    NFA_t nfa = NFA_build(node);
    NFA_graph(&nfa);
    
    char string[256];
    
    while(true){
        printf(">");
        int i = 0;
        fgets(string, 256, stdin);
        
        char* rm = strstr(string, "\n");
        if (rm != NULL)
            rm[0] = '\0';

        if (NFA_accepts(&nfa, string))
            printf("Accepted\n");
        else
            printf("Not accepted\n");
    }
    return 0;
}