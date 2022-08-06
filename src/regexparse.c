#include <regexparse.h>
	
static const char* regexpr;


static node_t* parser_rec(int*, node_t**, int*);
static char graph_rec(node_t* node, FILE* f);
static bool accepts_rec(node_t* node, const char* str, int* index);

/*int main(int argc, char** argv){
	
	if (input_control(argc, argv) == -1)
		return -1;
	
	node_t* root;
	if ( (root = parser() ) == NULL )
		return -1;
	
	if ( graph(root) == -1 )
		return -1;
	
	return 0;
}
*/

node_t* parse(const char* str){
	regexpr = str;
	int i = 0;
	node_t* stack_buf[2048];
	int stack_idx = 0;
	return parser_rec(&i, stack_buf, &stack_idx);
	
}

node_t* parser_rec(int* i, node_t** stack_buf, int* stack_idx){
	
	int n_iter = 0;
	char ch = regexpr[*i];
	node_t* node = NULL;
	bool escape = false;
		
	while(ch != '\0'){
		++(*i);
		
		if (escape){
			node = malloc(sizeof(node_t));
					
					if (n_iter > 0){ 
						node->op = CONCAT;
						node->l_child = stack_buf[--(*stack_idx)];
						node->r_child = malloc(sizeof(node_t));
						node->r_child->c = ch;
						node->r_child->op = NONE;
						node->r_child->l_child = NULL;
						node->r_child->r_child = NULL;				
					}else{ 
						node->c = ch;
						node->op = NONE;
						node->l_child = NULL;
						node->r_child = NULL;
						
					}
					
					stack_buf[(*stack_idx)++] = node;
					escape = false;
		}
		else switch(ch){
			
				case '(':
					if (n_iter > 0){
						node = malloc(sizeof(node_t));
						node->op = CONCAT;
						node->l_child = stack_buf[--(*stack_idx)];
						node->r_child = parser_rec(i, stack_buf, stack_idx);
					}
					else
						node = parser_rec(i, stack_buf, stack_idx);
					
					stack_buf[(*stack_idx)++] = node;
					break;
					
				case ')':
					return stack_buf[--(*stack_idx)];
					
				case '+':
					node = malloc(sizeof(node_t));
					node->op = UNION;
					node->l_child = stack_buf[--(*stack_idx)];
					node->r_child = parser_rec(i, stack_buf, stack_idx);
					
					stack_buf[(*stack_idx)++] = node;
					if (regexpr[(*i)-1] == ')') 
						--(*i);
					break;
					
				case '*':  
					node = malloc(sizeof(node_t));
					node->op = STAR;
					node->r_child = NULL;
					
					if (n_iter > 0)
						node->l_child = stack_buf[--(*stack_idx)];
					else
						node->l_child = NULL;
					
					
					stack_buf[(*stack_idx)++] = node;
					break;
				
				case '\\':
					escape = true;
					--n_iter;
					break;
					
				default:
					node = malloc(sizeof(node_t));
					
					if (n_iter > 0){ 
						node->op = CONCAT;
						node->l_child = stack_buf[--(*stack_idx)];
						node->r_child = malloc(sizeof(node_t));
						node->r_child->c = ch;
						node->r_child->op = NONE;
						node->r_child->l_child = NULL;
						node->r_child->r_child = NULL;				
					}else{ 
						node->c = ch;
						node->op = NONE;
						node->l_child = NULL;
						node->r_child = NULL;
						
					}
					
					stack_buf[(*stack_idx)++] = node;
					
		}
		
		node = NULL;
		ch = regexpr[*i];
		++n_iter;
	}
	
	if ((*stack_idx) > 0) return stack_buf[--(*stack_idx)];
	return NULL;
}

int graph(node_t* node){
	FILE* f;
	if ( (f = fopen("graph.gv", "w")) == NULL)
		return -1;
	
	fputs("digraph G{\n", f);
	if ( graph_rec(node, f) == -1 ){
		fclose(f);
		return -1;
	}
	fputs("}", f);
	
	fclose(f);
}

char graph_rec(node_t* node, FILE* f){
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

bool accepts(node_t* node, const char* str){
	if (node == NULL)
		return false;
		
	int a = 0;
	bool acc = false;
	acc = accepts_rec(node, str, &a);
	
	if (str[a] != '\0')
		acc = false;
		
	return acc;
}

bool accepts_rec(node_t* node, const char* str, int* index){

	bool i1, i2;
	int current;
	
	switch(node->op){
		case NONE:
			i1 = (str[*index] == node->c) ? true : false;
			if(i1){
				++(*index);
				return true;
			}else
				return false;
		case CONCAT:
			i1 = accepts_rec(node->l_child, str, index);
			if (!i1)
				return false;
			return accepts_rec(node->r_child, str, index);
			
		case UNION:
			i1 = accepts_rec(node->l_child, str, index);
			if (i1)
				return true;
			return accepts_rec(node->r_child, str, index);
			
		case STAR:
			if (str[*index] == '\0')
				return true;
				
			i1 = accepts_rec(node->l_child, str, index);
			if (!i1)
				return false;
			
			int current;
			while(i1){
				current = *index;
				i1 = accepts_rec(node->l_child, str, index);
			}
			
			*index = current;
			return true;
		
		default:
			return false;
	}
}
/*
int input_control(int argc,  char** argv){
	if (argc != 2){
		puts("USAGE: ./regpars <regexpr>");
		return -1;
	}
	
	regexpr = argv[1];
	filename = "graph.gv";
	return 0;
}
*/


/*DFA_t* assemble(node_t* node){
	if ( node == NULL)
		return NULL;
		
	DFA_t *left, *right, *this;
	
	if (node->l_child != NULL)
		left = assemble(node->l_child);
	if (node->r_child != NULL)
		right = assemble(node->r_child);
	
	
	switch(node->op){
		case NONE:
			return dfa_simple(node->c);
			
		case CONCAT:
			dfa_concat(left, right);
			dfa_destroy(right);
			return left;
		
		case UNION:
			return dfa_union(left, right);
		
		case STAR:
			return dfa_kleene(left);
		
		default:
			return NULL;
			
	}

}

DFA_t* dfa_simple(char c){
	DFA_t* dfa = malloc(sizeof(DFA_t));
	
}

*/

