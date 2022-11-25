cmake_minimum_required(VERSION 3.14)
project(Compiler C)



add_library(libcompiler STATIC ./regexparse.c)

target_compile_options(libcompiler PUBLIC -fsanitize=address PUBLIC -fsanitize=leak)
target_link_options(libcompiler PUBLIC -fsanitize=address PUBLIC -fsanitize=leak)

target_include_directories(libcompiler PUBLIC ../include)