#include <lexer.h>

#define REGBUFFER_LEN (sizeof(regex_buffer) / sizeof(regex_buffer[0]))

static const char* regex_buffer[] = { 
				"\n+\t+ ",
				"=",
				"\\++-+\\*+/",
				"==+<+>+!=+<=+>=",
				"(",
				")",
				"[",
				"]",
				"{",
				"}",
				";",
				"if",
				"while",
				"break",
				"else",
				"return",
				"int+char+float",
				"(0+1+2+3+4+5+6+7+8+9)((0+1+2+3+4+5+6+7+8+9)*)",
				"(a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+$+_)((a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_)*)",
				"\"((a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_+\\\\+/+ +<+>+&+\\++-+#+[+]+=+:+?+^+,+.+;+\\*)*)\"",
 				"'(a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_+\\\\+/+ +<+>+&+\\++-+#+[+]+=+:+?+^+,+.+;+\\*)'",
 				"!+'+\"+'(a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_+\\\\+/+ +<+>+&+\\++-+#+[+]+=+:+?+^+,+.+;+\\*)+\"((a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_+\\\\+/+ +<+>+&+\\++-+#+[+]+=+:+?+^+,+.+;+\\*)*)"
};


static NFA_t nfa_buf[REGBUFFER_LEN];

int tokenizer_init(){
	int i;
	node_t* node;
	
	for (i=0; i<(int)REGBUFFER_LEN; ++i){
		
		if ((node = tree_parse(regex_buffer[i])) == NULL)
			return -1;

		if ((nfa_buf[i] = NFA_build(node)).states_len == 0)
			return -1;
		
		tree_deinit(node);
	}

}

int tokenize(toklist_t* token_list, char* buffer){
	size_t buf_len = strlen(buffer);
	
	if (buf_len == 0)
		return -1;
	
	// Setting up
	token_list->len = 0;
	token_list->capacity = 128;
	if ((token_list->list = malloc(token_list->capacity * sizeof(token_t)) ) == NULL)
		return -1;
	
	size_t i;
	size_t prev_i = 0;
	size_t base_i = 0;
	
	toktype_t tt = NOTOK;
	
	 
	for (i=1; i <= buf_len; ++i){
		
		char temp_ch = buffer[i];
		buffer[i] = '\0';
		
		if (token_list->len >= token_list->capacity){
			token_list->capacity *= 2;
			if ((token_list->list = reallocarray(token_list->list, token_list->capacity, sizeof(token_t))) == NULL)
				return -1;
		}
		
		size_t j;
		bool acc;
		
		for (j=0; j<REGBUFFER_LEN; ++j)
			if ((acc = NFA_accepts(&nfa_buf[j], buffer+base_i)))
				break;
		
		buffer[i] = temp_ch;

		if (acc){ // continue
			++i;
			tt = j;
		}else{

			if (i == prev_i) //means characters are not recognized, throw error
				return -1;
			
			// allocating new token
			token_list->list[token_list->len].tk = malloc((i-base_i) * sizeof(char));
			token_list->list[token_list->len].tt = tt;
			strncpy(token_list->list[token_list->len].tk, buffer+base_i, i-base_i-1);
			++token_list->len;

			// Setting up for next iteration
			base_i = i-1; //base_i to the first not recognized character
			prev_i = i;   //prev_i to the current i location for control purposes
		}
		
	}


	return 0;
}

void print_tokens(const toklist_t* token_list){

	size_t i;
	for (i=0; i<token_list->len; ++i){

		if (token_list->list[i].tk[0] == '\n')
			printf("<%s> : '\\n'\n", typetokstr(token_list->list[i].tt));
		else if(token_list->list[i].tk[0] == '\t')
			printf("<%s> : '\\t'\n", typetokstr(token_list->list[i].tt));
		else
			printf("<%s> : '%s'\n", typetokstr(token_list->list[i].tt), token_list->list[i].tk);
	}
}
