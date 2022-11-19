#include <regexparse.h>

/* HELPERS */
static int parser_recursive(node_t**, const char*, int*, bool);
static char graph_rec(node_t* node, FILE* f);
static char eat(const char* regexpr, int* regexpr_index);
static char peek(const char* regexpr, int regexpr_index);
/* ******* */

void tree_deinit(node_t* node){

	if (node->l_child != NULL)
		tree_deinit(node->l_child);

	if (node->r_child != NULL)
		tree_deinit(node->r_child);
	
	free(node);
	return;
}


int tree_parse(node_t** node, const char* str){
	int i=0;
	assert(node != NULL && str != NULL);
	return parser_recursive(node, str, &i, false);
}

static int parser_recursive(node_t** node, const char* regexpr, int* regexpr_index, bool escape){

	int error_code;
	char c = eat(regexpr, regexpr_index);
	char k = peek(regexpr, (*regexpr_index)+1);

	if (!escape){
		switch (c){

			//fallthrough
			case ')':
			case '\0':
				*node = NULL;
				return OK;

			case '+':
			case '*':
				*node = NULL;
				return ILLFORMED_REGEXPR;

			case '\\':
				return parser_recursive(node, regexpr, regexpr_index, true);
	
			case '(':
				if (k == '+' || k == '*')
					return ILLFORMED_REGEXPR;

				// allocate a new concatenating node
				if ((error_code = node_allocate(*node, CONCAT)) != 0){
					return error_code;
				}

				// descend onto the left node
				if ((error_code = parser_recursive((*node)->l_child, regexpr, regexpr_index, false)) != 0){
					node_deallocate(*node);
					return error_code;
				}
				
				// peek if there is a kleene star before descending into right node
				if (peek(regexpr, *regexpr_index) == '*'){
					
					// allocate new kleene node between parent and left child
					node_t* intermediate;
					if ((error_code = node_allocate(intermediate, STAR)) != 0)
						return error_code;

					intermediate->l_child = (*node)->l_child;
					(*node)->l_child = intermediate;

					// eat out eventual additional stars
					while((c = peek(regexpr, *regexpr_index)) == '*'){
						(void) eat(regexpr, regexpr_index);
					}

				} // Otherwise check if there is a '+' ahead
				else if (peek(regexpr, *regexpr_index) == '+'){

					//this becomes a union node
					(*node)->op = UNION;
					(void) eat(regexpr, regexpr_index);
				}

				// descend into right node
				if ((error_code = parser_rec((*node)->r_child, regexpr, regexpr_index, false)) != 0){
					node_deallocate(*node);
					return error_code;
				}
				
				return OK;
			
			default:
				break;
		}
	}

	// allocate a new concatenating node
	if (node_allocate(*node, CONCAT) != 0){
		return BAD_ALLOCATION;
	}

	// allocate new left child
	if (node_allocate((*node)->l_child, NONE) != 0){
		node_deallocate((*node));
		return BAD_ALLOCATION;
	}

	// set the left child to the character literal
	(*node)->l_child->c = c;

	// peek if there is a '+' or a '*' behind
	if (peek(regexpr, *regexpr_index) == '*'){
		
		// allocate new kleene node between parent and left child
		node_t* intermediate;
		if ((error_code = node_allocate(intermediate, STAR)) == 0){
			return error_code;
		}

		intermediate->l_child = (*node)->l_child;
		(*node)->l_child = intermediate;

		// eat out eventual additional stars
		while((c = peek(regexpr, *regexpr_index)) == '*'){
			(void) eat(regexpr, regexpr_index);
		}

	} // Otherwise check if there is a '+' ahead
	else if (peek(regexpr, *regexpr_index) == '+'){

		//this becomes a union node
		(*node)->op = UNION;
		(void) eat(regexpr, regexpr_index);
	}

	// descend into right node
	if ((error_code = parser_rec((*node)->r_child, regexpr, regexpr_index, false)) != 0){
		node_deallocate((*node)->l_child);
		node_deallocate(*node);
		return error_code;
	}

	return OK;
}

int tree_graph(node_t* node){
	FILE* f;
	if ( (f = fopen("tree_graph.gv", "w")) == NULL)
		return -1;
	
	fputs("digraph G{\n", f);
	if ( graph_rec(node, f) == -1 ){
		fclose(f);
		return -1;
	}
	fputs("}", f);
	
	fclose(f);
	return 0;
}

static char graph_rec(node_t* node, FILE* f){
	if (node == NULL) return -1;
	
	char ch;
	char* bg_color = "white";
	switch(node->op){
		case NONE:
			ch = node->c;
			break;
		case UNION:
			ch = '+';
			break;
		case CONCAT:
			ch = '~';
			break;
		case STAR:
			ch = '*';
			break;
		default: return -1;
	}
	
	if(node->op != NONE) 
		bg_color = "red";
		
	fprintf(f, "%ld [label=\"%c\", style=\"filled\", fillcolor=\"%s\", shape=\"oval\"]\n", (unsigned long)node, ch, bg_color);
	
	char l = graph_rec(node->l_child, f);
	if (l != -1) 
		fprintf(f, "%lu -> %lu\n", (unsigned long)node, (unsigned long)node->l_child);
		
	char r = graph_rec(node->r_child, f);
	if (r != -1)
		fprintf(f, "%lu -> %lu\n", (unsigned long)node, (unsigned long)node->r_child);
	
	return ch;
}
