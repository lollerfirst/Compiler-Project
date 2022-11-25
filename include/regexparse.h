#ifndef _REGEXPARSE_H_
#define _REGEXPARSE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


typedef enum __op{
	NONE,
	CONCAT,
	UNION,
	STAR
} op_t;

typedef struct __node{
	op_t op;
	char c;
	struct __node* l_child;
	struct __node* r_child;
} node_t;

int tree_parse(node_t** node, const char* str);
void tree_deinit(node_t**);
int tree_graph(node_t*);

#endif
