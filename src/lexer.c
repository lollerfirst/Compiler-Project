#include <lexler.h>

#define REGBUFFER_LEN (sizeof(regex_buffer) / sizeof(regex_buffer[0]))

static const char* regex_buffer[] = { 
				"\n+\t+ ",
				"=",
				"\\++-+\\*+/",
				"==+<+>+!=+<=+>=",
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
 				"!+'+'(a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_+\\\\+/+ +<+>+&+\\++-+#+[+]+=+:+?+^+,+.+;+\\*)+\"((a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_+\\\\+/+ +<+>+&+\\++-+#+[+]+=+:+?+^+,+.+;+\\*)*)"
};


static node_t* dfa_buf[REGBUFFER_LEN];

void tokenizer_init(){
	int i;
	for (i=0; i<REGBUFFER_LEN; ++i)
		dfa_buf[i] = parse(regex_buffer[i]);
	
	//graph(dfa_buf[12]);

}

token_t* tokenize(char* buffer){

	if (buffer[0] == '\0')
		return NULL;
		
	size_t capacity = 128;
	token_t* token_list = calloc(capacity, sizeof(token_t));
	
	int i = 1;
	int base_i = 0;
	int n_token = 0;
	toktype_t tt = NOTOK;
	bool prev_acc = false;
	
	while(buffer[i] != '\0'){
		
		char ch = buffer[i];
		buffer[i] = '\0';
		
		if(n_token >= capacity){
			capacity *= 2;
			token_list = realloc(token_list, sizeof(token_t)*capacity);
		}
		
		int j;
		bool acc;
		
		for(j=0; j<REGBUFFER_LEN; ++j)
			if((acc = accepts(dfa_buf[j], buffer+base_i)))
				break;
		
		buffer[i] = ch;
		
		if (acc){
			++i;
			prev_acc = true;
			tt = j;
		}
		else{
			token_list[n_token].tk = calloc((i-base_i), sizeof(char));
			token_list[n_token].tt = tt;

			if(!prev_acc){
				strncpy(token_list[n_token].tk, buffer+base_i, i-base_i);
				++i;
			}else
				strncpy(token_list[n_token].tk, buffer+base_i, i-base_i-1);
			
			prev_acc = false;
			tt = NOTOK;
			
			n_token++;
			base_i = i-1;
			
			
		}
	}
	
	if(n_token >= capacity){
			capacity *= 2;
			token_list = realloc(token_list, sizeof(token_t)*capacity);
	}
		
	int j;
	
	for(j=0; j<REGBUFFER_LEN; ++j)
		if(accepts(dfa_buf[j], buffer+base_i))
			break;
	
	token_list[n_token].tk = calloc((i-base_i+1), sizeof(char));
	token_list[n_token].tt = tt;
	strcpy(token_list[n_token].tk, buffer+base_i);
	
	return token_list;
}

void print_tokens(token_t* token_list){
	int i = 0;
	while(token_list[i].tk != NULL){
		if (token_list[i].tk[0] == '\n')
			printf("<%s> : '\\n'\n", typetokstr(token_list[i].tt));
		else if(token_list[i].tk[0] == '\t')
			printf("<%s> : '\\t'\n", typetokstr(token_list[i].tt));
		else
			printf("<%s> : '%s'\n", typetokstr(token_list[i].tt), token_list[i].tk);
			
		++i;
	}
}

const char* typetokstr(toktype_t tktype){
	switch (tktype){
		case DELIM:
			return "Delimiter";
		case ASSIGN_OP:
			return "Assign-OP";
		case ALGEBRAIC_OP:
			return "Algebraic-OP";
		case BOOLEAN_OP:
			return "Boolean-OP";
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
			return "Type";
		case NUMBER:
			return "Number";
		case NAME:
			return "Name";
		case STRING:
			return "String";
		case CHAR:
			return "Char";
		default:
			return "no-tok";
	}
}
