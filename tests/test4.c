#include <parser.h>
#include <lexer.h>
#include <stdio.h>
#include <compiler_errors.h>
#include <assert.h>

static ast_t ast;
static toklist_t token_list;

static char program[] = "\
    generic_function(parameter0, parameter1, 101) := base_function(other_generic_function(\"generic string\"));\
    irrelevant_name1(parameter0, parameter2, parameter3) := add(parameter0, generic_function(parameter2, parameter3, 101));\
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