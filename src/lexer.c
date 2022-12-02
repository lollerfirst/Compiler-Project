#include <lexer.h>
#include <compiler_errors.h>

#define REGBUFFER_LEN (sizeof(regex_buffer) / sizeof(regex_buffer[0]))

/*
static const char* regex_buffer[] = { 
				"\n+\t+ ",
				":=",
				"\\(",
				"\\)",
				"[",
				"]",
				";",
				",",
				"(0+1+2+3+4+5+6+7+8+9)((0+1+2+3+4+5+6+7+8+9)*)",
				"(a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+$+_)((a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_)*)",
				"\"((a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_+\\\\+/+ +<+>+&+\\++-+#+[+]+=+:+?+^+,+.+;+\\*)*)\"",
 				"'(a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_+\\\\+/+ +<+>+&+\\++-+#+[+]+=+:+?+^+,+.+;+\\*)'",
				
				//trash
 				"'+\"+'(a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_+\\\\+/+ +<+>+&+\\++-+#+[+]+=+:+?+^+,+.+;+\\*)+\"((a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_+\\\\+/+ +<+>+&+\\++-+#+[+]+=+:+?+^+,+.+;+\\*)*)"
};
*/


int tokenizer_init(toklist_t* toklist, const char* nfa_collection_filename){
	toklist->list = NULL;
	toklist->list_capacity = 0;
	toklist->list_size = 0;

	ERROR_RETHROW(nfa_collection_load(
			&(toklist->nfa_collection),
			&(toklist->nfa_collection_size),
			nfa_collection_filename
		)
	);

	return OK;
}

int tokenize(toklist_t* token_list, char* buffer){
	size_t buffer_len = strlen(buffer);
	
	if (buffer_len == 0)
		return INVALID_BUFFER;
	
	// Setting up
	token_list->list_size = 0;
	token_list->list_capacity = ASCII_LEN;

	if ((token_list->list = malloc(
			token_list->list_capacity * sizeof(token_t)
		) ) == NULL)
	{
		return BAD_ALLOCATION;
	}
	
	size_t i = 1;
	size_t prev_i = 0;
	size_t base_i = 0;
	
	toktype_t tt = NOTOK;
	
	 
	while (i <= buffer_len){
		
		char temp_char;
		size_t j;
		bool accepted;
		
		temp_char = buffer[i];
		buffer[i] = '\0';
		

		// allocate new token
		if (token_list->list_size >= token_list->list_capacity)
		{
			size_t new_capacity = token_list->list_capacity * 2;
			token_t* new_list;
			
			// RESIZE
			if ((new_list = reallocarray(
				token_list->list, new_capacity, sizeof(token_t)
				)) == NULL)
			{
				tokenizer_deinit(token_list);
				return BAD_ALLOCATION;
			}
		}
		
		
		
		for (j=0; j<token_list->nfa_collection_size; ++j)
		{
			ERROR_RETHROW(
				nfa_accepts(
					&(token_list->nfa_collection[j]),
					&(buffer[base_i]),
					&accepted
				),
				tokenizer_deinit(token_list)
			);
		}

		if (accepted == false)
		{
			tokenizer_deinit(token_list);
			return INVALID_TOKEN;
		}

		buffer[i] = temp_char;

		if (accepted){ // continue
			++i;
			tt = j;
		}else{

			//means characters are not recognized, throw error
			if (i == prev_i)
			{ 	
				tokenizer_deinit(token_list);
				return INVALID_TOKEN;
			}
			
			// allocating new token
			if ((token_list->list[token_list->list_size].tk = calloc(sizeof(char), i-base_i)) == NULL)
			{
				tokenizer_deinit(token_list);
				return BAD_ALLOCATION;
			}

			token_list->list[token_list->list_size].tt = tt;
			strncpy(token_list->list[token_list->list_size].tk, buffer+base_i, i-base_i-1);
			++token_list->list_size;

			// Setting up for next iteration
			base_i = i-1; //base_i to the first not recognized character
			prev_i = i;   //prev_i to the current i location for control purposes
		}
		
	}


	return 0;
}

void print_tokens(const toklist_t* token_list){

	size_t i;
	for (i=0; i<token_list->list_size; ++i){

		if (token_list->list[i].tk[0] == '\n')
			printf("<%s> : '\\n'\n", tokenizer_typetokstr(token_list->list[i].tt));
		else if (token_list->list[i].tk[0] == '\t')
			printf("<%s> : '\\t'\n", tokenizer_typetokstr(token_list->list[i].tt));
		else
			printf("<%s> : '%s'\n", tokenizer_typetokstr(token_list->list[i].tt), token_list->list[i].tk);
	}
}

void tokenizer_deinit(toklist_t* toklist)
{
	if (toklist->nfa_collection_size > 0 && toklist->nfa_collection != NULL)
	{
		nfa_collection_delete(toklist->nfa_collection, toklist->nfa_collection_size);
		toklist->nfa_collection = NULL;
		toklist->nfa_collection_size = 0;
	}

	if (toklist->list_capacity > 0 && toklist->list != NULL)
	{
		size_t i;
		for (i=0; i<toklist->list_size; ++i)
		{
			if (toklist->list->tk != NULL)
			{
				free(toklist->list->tk);
			}
		}
		
		free(toklist->list);
		toklist->list_capacity = 0;
		toklist->list_size = 0;
	}
}

const char* tokenizer_typetokstr(toktype_t tktype){
	switch (tktype){
		case DELIM:
			return "delimiter";
		case DEFINE_OP:
			return "define-op";
		case L_ROUNDB:
			return "left-roundbracket";
		case R_ROUNDB:
			return "right-roundbracket";
		case L_SQUAREB:
			return "left-squarebracket";
		case R_SQUAREB:
			return "right-squarebracket";
		case END_STMT:
			return "end-statement";
		case ARGSTOP:
			return "argstop";
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
