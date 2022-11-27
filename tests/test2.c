#include <check.h>
#include <nfa_builder.h>
#include <regexparse.h>

static const char* regex_buffer[] = { 
				"(0+1+2+3+4+5+6+7+8+9)((0+1+2+3+4+5+6+7+8+9)*)",
				"(a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+$+_)((a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_)*)",
				"\"((a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_+\\\\+/+ +<+>+&+\\++-+#+[+]+=+:+?+^+,+.+;+\\*)*)\""
};

START_TEST(test_nfa_builder)
{
    nfa_t* nfa;
    node_t* node;
    int i;
    for (i=0; i<3; ++i)
    {
        int error_code;

        tree_parse(&node, regex_buffer[i]);

        ck_assert_msg((error_code = nfa_build(&nfa, node)) == 0,
            "Error while building the nfa. CODE=%d",
            error_code
        );


        nfa_destroy(&nfa);
        tree_deinit(&node);

        ck_assert_ptr_eq(nfa, NULL);
    }

}
END_TEST

int main(int argc, char** argv){
    Suite* suite;
    TCase* test_case;
    int number_failed;

    suite = suite_create("NFA builder");

    test_case = tcase_create("nfa builder");
    tcase_add_test(test_case, test_nfa_builder);
    suite_add_tcase(suite, test_case);
    
    SRunner* suite_runner = srunner_create(suite);
    srunner_run_all(suite_runner, CK_NORMAL);

    number_failed = srunner_ntests_failed(suite_runner);
    srunner_free(suite_runner);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}