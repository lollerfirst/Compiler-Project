#include <lexer.h>
#include <parser.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>


int main(int argc, char** argv){
	if (argc != 2){
		puts("USAGE: ./lexler <target_text_file>");
		return -1;
	}
	
	// MAP FILE TO ADDRESS SPACE
	FILE* f;
	if ( (f = fopen(argv[1], "r")) == NULL){
		printf("Error: %s", strerror(errno));
		return -1;
	}
	
	char* buffer;
	if ( (buffer = mmap(NULL, 4096UL, PROT_READ | PROT_WRITE, MAP_PRIVATE, fileno(f), 0)) == NULL){
		printf("Error: %s", strerror(errno));
		return -1;
	}
	// *************************
	
	if (tokenizer_init() != 0)
		return -1;

	toklist_t token_list = {0};
	if (tokenize(&token_list, buffer))
		return -1;

	print_tokens(&token_list);
	
	AST_t ast;
	if (parser_ast(&ast, &token_list) != 0)
		return -1;
	if (parser_graph(&ast, "ast_graph.gv") != 0)
		return -1;

	tokenizer_deinit();
	parser_free(&ast);
	return 0;
	
}
