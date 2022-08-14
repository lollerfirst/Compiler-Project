#include <lexer.h>
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
	
	tokenizer_init();
	toklist_t token_list = {0};
	tokenize(&token_list, buffer);
	print_tokens(&token_list);
		
	return 0;
	
}
