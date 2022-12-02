#include <nfa_builder.h>
#include <regexparse.h>
#include <compiler_errors.h>

#define REGEXBUFFER_LEN (sizeof(regex_buffer) / sizeof(regex_buffer[0]))

static const char* regex_buffer[] = { 
				"\n+\t+ ",
				":=",
				"\\(",
				"\\)",
				"[",
				"]",
				";",
				",",
				"(0+1+2+3+4+5+6+7+8+9)(0+1+2+3+4+5+6+7+8+9)*",
				"(a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+$+_)(a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_)*",
				"\"((a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_+\\\\+/+ +<+>+&+\\++-+#+[+]+=+:+?+^+,+.+;+\\*)*)\"",
 				"'(a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_+\\\\+/+ +<+>+&+\\++-+#+[+]+=+:+?+^+,+.+;+\\*)'",
				
				//trash
 				":+'+\"+'(a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_+\\\\+/+ +<+>+&+\\++-+#+[+]+=+:+?+^+,+.+;+\\*)+\"(a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_+\\\\+/+ +<+>+&+\\++-+#+[+]+=+:+?+^+,+.+;+\\*)*"
};

int main()
{
    nfa_t collection[REGEXBUFFER_LEN];

    size_t i;
    for (i=0; i<REGEXBUFFER_LEN; ++i)
    {
        node_t* tree;
        ERROR_RETHROW(tree_parse(&tree, regex_buffer[i]));

        ERROR_RETHROW(nfa_build(&collection[i], tree));
        tree_deinit(&tree);
    }

    ERROR_RETHROW(nfa_collection_save(collection, REGEXBUFFER_LEN, "nfa_collection.dat"));

    for (i=0; i<REGEXBUFFER_LEN; ++i)
    {
        nfa_destroy(&collection[i]);
    }

    return 0;
}