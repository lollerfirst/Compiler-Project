#include <interpreter.h>
#include <parser.h>
#include <lexer.h>
#include <unistd.h>
#include <fcntl.h>
#include <compiler_errors.h>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        fprintf(stdout, "USAGE: tomc <textfile>\n");
        return -1;
    }
    
    const char* filename = argv[1];

    int fd;
    if ((fd = open(filename, O_RDONLY)) == -1)
    {
        fprintf(stderr, "FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        return -1;
    }

    if (lseek(fd, 0, SEEK_END) == -1)
    {
        close(fd);
        fprintf(stderr, "FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        return -1;
    }

    size_t length = lseek(fd, 0, SEEK_CUR);
    lseek(fd, 0, SEEK_SET);

    char buffer[length+1];
    bzero(buffer, length+1);

    if (read(fd, buffer, sizeof(char) * length) < (ssize_t) (sizeof(char) * length))
    {
        close(fd);
        fprintf(stderr, "FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        return -1;
    }

    close(fd);

    interpreter_init();
    toklist_t token_list = {0};
    ast_t ast = {0};

    // tokenize the buffer
    ERROR_RETHROW(tokenizer_init(&token_list, "nfa_collection.dat"));
    ERROR_RETHROW(tokenize(&token_list, buffer),
        tokenizer_deinit(&token_list)
    );

    ERROR_RETHROW(parser_ast(&ast, &token_list),
        tokenizer_deinit(&token_list)
    );

    parser_ast_graph(&ast, "ast_graph.gv");

    ERROR_RETHROW(interpret(&ast),
        parser_ast_delete(&ast);
        tokenizer_deinit(&token_list)
    );

    tokenizer_deinit(&token_list);
    parser_ast_delete(&ast);
    interpreter_release();
    return 0;
}