#include <lexer.h>

#define REGBUFFER_LEN (sizeof(regex_buffer) / sizeof(regex_buffer[0]))

static const char* regex_buffer[] = { 
				"\n+\t+ ",
				"=",
				"\\++-",
				"\\*+/",
				"==+<+>+!=+<=+>=",
				"\\(",
				"\\)",
				"[",
				"]",
				"{",
				"}",
				";",
				",",
				"!",
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
 				"'+\"+'(a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_+\\\\+/+ +<+>+&+\\++-+#+[+]+=+:+?+^+,+.+;+\\*)+\"((a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_+\\\\+/+ +<+>+&+\\++-+#+[+]+=+:+?+^+,+.+;+\\*)*)"
};


static NFA_t* nfa_buf[REGBUFFER_LEN];
static const char* typetokstr(toktype_t);

int tokenizer_init(){
	size_t i;
	node_t* node;
	
	for (i=0; i<REGBUFFER_LEN; ++i){
		
		if ((node = tree_parse(regex_buffer[i])) == NULL)
			return -1;

		if ((nfa_buf[i] = NFA_build(node)) == NULL)
			return -1;
		
		tree_deinit(node);
	}

	return 0;
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
	
	size_t i = 1;
	size_t prev_i = 0;
	size_t base_i = 0;
	
	toktype_t tt = NOTOK;
	
	 
	while (i <= buf_len){
		
		char temp_ch = buffer[i];
		buffer[i] = '\0';
		
		if (token_list->len >= token_list->capacity){
			token_list->capacity *= 2;
			if ((token_list->list = reallocarray(token_list->list, token_list->capacity, sizeof(token_t))) == NULL)
				return -1;
		}
		
		size_t j;
		int acc;
		
		for (j=0; j<REGBUFFER_LEN; ++j)
			if ((acc = NFA_accepts(nfa_buf[j], buffer+base_i)))
				break;
		
		if (acc == -1)
			return -1;
		
		buffer[i] = temp_ch;

		if (acc){ // continue
			++i;
			tt = j;
		}else{

			if (i == prev_i) //means characters are not recognized, throw error
				return -1;
			
			// allocating new token
			if ((token_list->list[token_list->len].tk = calloc(sizeof(char), i-base_i)) == NULL)
				return -1;

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
		else if (token_list->list[i].tk[0] == '\t')
			printf("<%s> : '\\t'\n", typetokstr(token_list->list[i].tt));
		else
			printf("<%s> : '%s'\n", typetokstr(token_list->list[i].tt), token_list->list[i].tk);
	}
}

void tokenizer_deinit(){
	size_t i;
	for (i=0; i<REGBUFFER_LEN; ++i)
		NFA_destroy(nfa_buf[i]);
}

static const char* typetokstr(toktype_t tktype){
	switch (tktype){
		case DELIM:
			return "delimiter";
		case ASSIGN_OP:
			return "assign-op";
		case ALGEBRAIC_OP:
			return "algebraic-op";
		case MUL_OP:
			return "multiplication-op";
		case BOOLEAN_OP:
			return "boolean-op";
		case L_ROUNDB:
			return "left-roundbracket";
		case R_ROUNDB:
			return "right-roundbracket";
		case L_SQUAREB:
			return "left-squarebracket";
		case R_SQUAREB:
			return "right-squarebracket";
		case L_CURLYB:
			return "left-curlybracket";
		case R_CURLYB:
			return "right-curlybracket";
		case END_STMT:
			return "end-statement";
		case ARGSTOP:
			return "argstop";
		case LOGIC_NOT:
			return "logical-not";
		case IF:
			return "if";
		case WHILE:
			return "while";
		case BREAK:
			return "break";
		case ELSE:
			return "else";
		case RETURN:
			return "return";
		case TYPE:
			return "type";
		case NUMBER:
			return "number";
		case NAME:
			return "name";
		case STRING:
			return "string";
		case CHAR:
			return "char";
		default:
			return "no-tok";
	}
}
