#include <lexler.h>
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
	token_t* token_list = tokenize(buffer);
	print_tokens(token_list);
	
	/*
	node_t* dfa2 = parse(regex_buffer[NAME]);
	graph(dfa2);
	
	char token[] = "var2";
	
	if (accepts(dfa2, token))
		printf("%s é un token riconosciuto\n", token);
	else
		printf("%s non é un token riconosciuto\n", token);
	*/
		
	return 0;
	
}
