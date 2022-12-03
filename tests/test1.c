#include <regexparse.h>
#include <string.h>
#include <compiler_errors.h>
#include <nfa_builder.h>
#include <assert.h>
#include <stdio.h>


static const char* regex_buffer[] = { 
				"(0+1+2+3+4+5+6+7+8+9)(0+1+2+3+4+5+6+7+8+9)*",
				"(a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+$+_)((a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_)*)",
				"\"((a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_+\\\\+/+ +<+>+&+\\++-+#+[+]+=+:+?+^+,+.+;+\\*)*)\"",
                ":+'+\"+('(a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_+\\\\+/+ +<+>+&+\\++-+#+[+]+=+:+?+^+,+.+;+\\*))+(\"(a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_+\\\\+/+ +<+>+&+\\++-+#+[+]+=+:+?+^+,+.+;+\\*)*)"
};

void check_tree_integrity(const node_t* tree)
{

    assert(tree != NULL);

    assert(tree->op >= NONE && tree->op <= STAR);

    assert(tree->op != NONE || (tree->c > 0 && tree->c < 128));

    if (tree->op == CONCAT || tree->op == UNION)
    {
        assert(tree->l_child != NULL);
        assert(tree->r_child != NULL);
    }else if (tree->op == STAR)
    {
        assert(tree->l_child != NULL);
    }
    
    if (tree->l_child != NULL){
        check_tree_integrity(tree->l_child);
    }

    if (tree->r_child != NULL){
        check_tree_integrity(tree->r_child);
    }

    return;
}

void test_regexpr()
{
    
    node_t* node;
    int i;
    for (i=0; i<4; ++i)
    {
        
        assert(tree_parse(&node, regex_buffer[i]) == OK);
        
        check_tree_integrity(node);
        
        tree_deinit(&node);
        assert(node == NULL);
    }
}

int main(){
    printf("[*] Test Regexpr:\n");
    
    test_regexpr();
    
    printf("[+] Test Successful\n");

    
    return 0;
}