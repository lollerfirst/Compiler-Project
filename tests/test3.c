#include <stdio.h>
#include <nfa_builder.h>
#include <compiler_errors.h>
#include <lexer.h>
#include <assert.h>

/* Testing lexer functionalities */

static toklist_t token_list;
static char string_to_tokenize[] = "generic_function(parameter0, parameter1, 101) := base_function(other_generic_function(\"generic string\"))";

void test_tokenizer_init(void)
{
    bzero(&token_list, sizeof(toklist_t));
    assert(tokenizer_init(&token_list, "nfa_collection.dat") == OK);

    assert(token_list.list == NULL);
    assert(token_list.list_capacity == 0);
    assert(token_list.list_size == 0);

    assert(token_list.nfa_collection != NULL);
    assert(token_list.nfa_collection_size > 0);
}

void test_tokenizer_deinit(void)
{
    tokenizer_deinit(&token_list);

    assert(token_list.list == NULL);
    assert(token_list.list_capacity == 0);
    assert(token_list.list_size == 0);
    assert(token_list.nfa_collection == NULL);
    assert(token_list.nfa_collection_size == 0);
}

void test_tokenize(void)
{
    assert(tokenize(&token_list, string_to_tokenize) == OK);

    assert(token_list.list != NULL);
    assert(token_list.list_size > 0);
    assert(token_list.list_capacity >= token_list.list_size);

    print_tokens(&token_list);
}

int main()
{

    printf("[*] Test tokenizer_init():\n");
    test_tokenizer_init();
    printf("[+] Test Successful\n");

    printf("[*] Test tokenize():\n");
    test_tokenize();
    printf("[+] Test Successful\n");

    printf("[*] Test tokenizer_deinit():\n");
    test_tokenizer_deinit();
    printf("[+] Test Successful\n");

    return 0;
}