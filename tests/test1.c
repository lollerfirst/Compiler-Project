#include <regexparse.h>
#include <string.h>
#include <compiler_errors.h>

#include <check.h>

void check_tree_integrity(const node_t* tree)
{
    static int depth = 0;

    ck_assert_msg(tree->op >= NONE && tree->op <= STAR, 
        "Corrupted tree: depth = %d",
        depth
    );

    ck_assert_msg(tree->op != NONE || (tree->c > 0 && tree->c < 128),
        "Invalid value for char while tree->op is NONE: depth = %d",
        depth
    );

    ++depth;

    if (tree->l_child != NULL){
        check_tree_integrity(tree->l_child);
    }

    if (tree->r_child != NULL){
        check_tree_integrity(tree->r_child);
    }

    return;
}

START_TEST(test_regexparser)
{
    static const char* regex_buffer[] = { 
				"(0+1+2+3+4+5+6+7+8+9)((0+1+2+3+4+5+6+7+8+9)*)",
				"(a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+$+_)((a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_)*)",
				"\"((a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t+u+v+w+x+y+z+A+B+C+E+F+G+H+I+L+M+N+O+P+Q+R+S+T+U+V+W+X+Y+Z+0+1+2+3+4+5+6+7+8+9+$+_+\\\\+/+ +<+>+&+\\++-+#+[+]+=+:+?+^+,+.+;+\\*)*)\""
    };
    
    node_t* node;
    int i;
    for (i=0; i<sizeof(regex_buffer); ++i)
    {
        int error_code;
        ck_assert_msg((error_code = tree_parse(&node, regex_buffer[i])) == 0, 
            "Error while parsing the tree: CODE=%d", error_code);
        
        check_tree_integrity(node);
        
        tree_deinit(&node);
        ck_assert_msg(node == NULL,
            "Node pointer is not NULL after deinitialization"
        );
    }
}
END_TEST


int main(int argc, char** argv){
    Suite* suite;
    TCase* test_case;
    int number_failed;

    suite = suite_create("Regexpr");
    test_case = tcase_create("core");

    tcase_add_test(test_case, test_regexparser);
    suite_add_tcase(suite, test_case);
    
    SRunner* suite_runner = srunner_create(suite);
    srunner_run_all(suite_runner, CK_NORMAL);

    number_failed = srunner_ntests_failed(suite_runner);
    srunner_free(suite_runner);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}