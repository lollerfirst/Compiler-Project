#include <parser.h>
#include <lexer.h>
#include <stdio.h>
#include <compiler_errors.h>
#include <assert.h>

static ast_t ast;
static toklist_t token_list;

static char program[] = "\
    generic_function(parameter0) := next(parameter0);\
    generic_function(parameter1) := next(parameter1);\
    generic_function(parameter2) := next(parameter2);\
    generic_function(parameter3) := next(parameter3);\
    \
";

void setup()
{
    tokenizer_init(&token_list, "nfa_collection.dat");
    tokenize(&token_list, program);
    print_tokens(&token_list);
}

void test_parser_ast()
{
    assert(parser_ast(&ast, &token_list) == OK);
    parser_ast_graph(&ast, "ast_graph.gv");

    parser_ast_delete(&ast);
}

void teardown()
{
    tokenizer_deinit(&token_list);
}

int main()
{
    printf("[*] Setting up...\n");
    setup();

    printf("[*] Testing parser_ast:\n");
    test_parser_ast();
    printf("[+] Test successful\n");

    printf("[*] Cleaning up...\n");
    teardown();

    return 0;
}