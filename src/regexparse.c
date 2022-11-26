#include <regexparse.h>
#include <compiler_errors.h>
#include <assert.h>

#define node_deallocate(__PTR) \
	(free(__PTR))

#define peek(_REGEXPR, _INDEX) \
	(_REGEXPR[_INDEX])

#define eat(_REGEXPR, _INDEX) \
	(_REGEXPR[(*(_INDEX))++])


/* HELPERS */
static int parser_recursive(node_t**, const char*, int*, bool);
static char graph_rec(node_t* node, FILE* f);
static int node_allocate(node_t**, op_t);
/* ******* */

void tree_deinit(node_t** node){
	assert(node != NULL);
	assert(*node != NULL);

	if ((*node)->l_child != NULL){
		tree_deinit(&(*node)->l_child);
	}

	if ((*node)->r_child != NULL){
		tree_deinit(&(*node)->r_child);
	}
	
	free(*node);
	*node = NULL;

	return;
}


int tree_parse(node_t** node, const char* str){
	int i=0;
	assert(str != NULL);
	return parser_recursive(node, str, &i, false);
}

static int parser_recursive(node_t** node, const char* regexpr, int* regexpr_index, bool escape){

	char c, k;
	if (peek(regexpr, *regexpr_index) == '\0')
	{	
		*node = NULL;
		return OK;
	}
	
	c = eat(regexpr, regexpr_index);
	k = peek(regexpr, *regexpr_index);

	if (!escape){
		switch (c){
			case ')':
				*node = NULL;
				return OK;

			case '+':
			case '*':
				*node = NULL;
				return ILLFORMED_REGEXPR;

			case '\\':
				return parser_recursive(node, regexpr, regexpr_index, true);
	
			case '(':
				if (k == '+' || k == '*' || k == '\0')
				{
					*node = NULL;
					return ILLFORMED_REGEXPR;
				}

				// allocate a new concatenating node
				ERROR_RETHROW(node_allocate(node, CONCAT));

				// descend onto the left node
				ERROR_RETHROW(
					parser_recursive(&(*node)->l_child, regexpr, regexpr_index, false),
					tree_deinit(node);
				);

				// peek if there is a '+' or a '*' behind
				k = peek(regexpr, *regexpr_index);
				if (k == '*')
				{
					
					// allocate new kleene node between parent and left child
					node_t* intermediate;
					ERROR_RETHROW(
						node_allocate(&intermediate, STAR),
						tree_deinit(node)
					);

					intermediate->l_child = (*node)->l_child;
					(*node)->l_child = intermediate;

					// eat out eventual additional stars
					do
					{
						eat(regexpr, regexpr_index);
					}
					while(peek(regexpr, *regexpr_index) == '*');

				} // Otherwise check if there is a '+' ahead
				else if (k == '+')
				{

					//this becomes a union node
					(*node)->op = UNION;
					eat(regexpr, regexpr_index);
				}

				// descend into right node
				ERROR_RETHROW(
					parser_recursive(&(*node)->r_child, regexpr, regexpr_index, false),
					tree_deinit(node);
				);
				
				return OK;
			
			default:
				break;
		}
	}

	// allocate a new concatenating node
	ERROR_RETHROW(node_allocate(node, CONCAT));

	// allocate new left child
	ERROR_RETHROW(
		node_allocate(&(*node)->l_child, NONE),
		tree_deinit(node)
	);

	// set the left child to the character literal
	(*node)->l_child->c = c;

	// peek if there is a '+' or a '*' behind
	if (k == '*'){
		
		// allocate new kleene node between parent and left child
		node_t* intermediate;
		ERROR_RETHROW(
			node_allocate(&intermediate, STAR),
			tree_deinit(node)
		);

		intermediate->l_child = (*node)->l_child;
		(*node)->l_child = intermediate;

		// eat out eventual additional stars
		do
		{
			eat(regexpr, regexpr_index);
		}
		while(peek(regexpr, *regexpr_index) == '*');

	} // Otherwise check if there is a '+' ahead
	else if (k == '+'){

		//this becomes a union node
		(*node)->op = UNION;
		eat(regexpr, regexpr_index);
	}

	// descend into right node
	ERROR_RETHROW(
		parser_recursive(&(*node)->r_child, regexpr, regexpr_index, false),
		tree_deinit(node)
	);

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

static int node_allocate(node_t** node, op_t op){
	assert(node != NULL);
	
	node_t* n;
	if ((n = malloc(sizeof(node_t))) == NULL)
		return BAD_ALLOCATION;
	
	n->op = op;
	n->l_child = NULL;
	n->r_child = NULL;
	*node = n;

	return OK;
}