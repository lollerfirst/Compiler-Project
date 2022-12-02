#include <nfa_builder.h>
#include <regexparse.h>
#include <compiler_errors.h>
#include <assert.h>
#include <stdio.h>

static const char* regex_buffer[] = { 
				"(0+1+2+3+4+5+6+7+8+9)((0+1+2+3+4+5+6+7+8+9)*)",
				"(a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+$+_)((a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_)*)",
				"\"((a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_+\\\\+/+ +<+>+&+\\++-+#+[+]+=+:+?+^+,+.+;+\\*)*)\""
};

static node_t* node[3];
static nfa_t nfa_collection[3];

static nfa_t* nfa_loaded_list;
static size_t list_len;

void setup()
{
    int i;
    for (i=0; i<3; ++i)
    {
        tree_parse(&node[i], regex_buffer[i]);
    }
}

void teardown()
{
    int i;
    for (i=0; i<3; ++i)
    {
        tree_deinit(&node[i]);
    }
}

void test_nfa_builder()
{

    size_t i;
    for (i=0; i<3; ++i)
    {
        assert(nfa_build(&nfa_collection[i], node[i]) == OK);
        
        nfa_t nfa = nfa_collection[i];
        assert(nfa.states_len > 0);
        
        size_t j;
        for (j=0; j<nfa.states_len; ++j)
        {
            assert(nfa.states[j].len <= nfa.states[j].capacity);
            if (nfa.states[j].len > 0)
            {
                assert(nfa.states[j].charset != NULL);
                assert(nfa.states[j].mapped_state != NULL);
            }
        }
    }

}

void test_nfa_accepts()
{
    bool result;

    // 2 correct tokens, 1 wrong
    static const char* tokens1[] = {"5", "110", "00ab"};
    
    result = false;
    assert(nfa_accepts(&nfa_collection[0], tokens1[0], &result) == OK);
    assert(result == true);

    assert(nfa_accepts(&nfa_collection[0], tokens1[1], &result) == OK);
    assert(result == true);

    assert(nfa_accepts(&nfa_collection[0], tokens1[2], &result) == OK);
    assert(result == false);

    // second nfa
    // 2 correct tokens, 1 wrong
    static const char* tokens2[] = {"name", "_$name$_", "00name"};
    
    result = false;
    assert(nfa_accepts(&nfa_collection[1], tokens2[0], &result) == OK);
    assert(result == true);

    assert(nfa_accepts(&nfa_collection[1], tokens2[1], &result) == OK);
    assert(result == true);

    assert(nfa_accepts(&nfa_collection[1], tokens2[2], &result) == OK);
    assert(result == false);

    //third nfa
    // 2 correct tokens, 1 wrong
    static const char* tokens3[] = {"\"I am a string\"", "\"I+am_also[a]$tr1ng\"", "I am not a string"};

    result = false;
    assert(nfa_accepts(&nfa_collection[2], tokens3[0], &result) == OK);
    assert(result == true);

    assert(nfa_accepts(&nfa_collection[2], tokens3[1], &result) == OK);
    assert(result == true);

    assert(nfa_accepts(&nfa_collection[2], tokens3[2], &result) == OK);
    assert(result == false);

}


void test_collection_save()
{
    assert(nfa_collection_save(nfa_collection, 3UL, "nfa_collection.dat") == OK);
}

void test_collection_load()
{
    nfa_loaded_list = NULL;
    list_len = 0;

    assert(nfa_collection_load(&nfa_loaded_list, &list_len, "nfa_collection.dat") == OK);

    assert(list_len > 0);

    size_t i;
    for (i=0; i<list_len; ++i)
    {
        assert(nfa_loaded_list[i].states_len > 0);
        assert(nfa_loaded_list[i].states != NULL);

        size_t j;
        for (j=0; j<nfa_loaded_list[i].states_len; ++j)
        {
            if (nfa_loaded_list[i].states[j].len > 0)
            {
                assert(nfa_loaded_list[i].states[j].charset != NULL);
                assert(nfa_loaded_list[i].states[j].mapped_state != NULL);
            }
        }
    }
}

void test_collection_delete()
{
    nfa_collection_delete(nfa_loaded_list, list_len);
}

void test_nfa_destroy()
{
    int i;
    for (i=0; i<3; ++i)
    {
        nfa_destroy(&nfa_collection[i]);
    }
}

int main()
{
    printf("[*] Setting up...\n");
    setup();

    printf("[*] Test nfa_builder:\n");
    test_nfa_builder();
    printf("[+] Test Successful\n");

    printf("[*] Test nfa_accepts:\n");
    test_nfa_accepts();
    printf("[+] Test Successful\n");

    printf("[*] Test nfa_collection_save:\n");
    test_collection_save();
    printf("[+] Test Successful\n");

    printf("[*] Test nfa_destroy\n");
    test_nfa_destroy();
    printf("[+] Test Successful\n");

    printf("[*] Test nfa_collection_load:\n");
    test_collection_load();
    printf("[+] Test Successful\n");

    printf("[*] Test nfa_collection_delete:\n");
    test_collection_delete();
    printf("[+] Test Successful\n");

    printf("[*] Cleaning up...\n");
    teardown();
    
    return 0;
}