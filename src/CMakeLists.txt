cmake_minimum_required(VERSION 3.8)
project(Compiler C)



add_library(libcompiler STATIC ./regexparse.c ./nfa_builder.c ./lexer.c ./parser.c ./interpreter.c)

target_compile_options(libcompiler PUBLIC -Wall -Wextra -pedantic -Werror -fsanitize=address -fsanitize=leak -g)
target_link_options(libcompiler PUBLIC -fsanitize=address PUBLIC -fsanitize=leak)

target_include_directories(libcompiler PUBLIC ../include)