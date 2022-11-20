cmake_minimum_required(VERSION 3.14)
project(Compiler C)


add_executable(test1 test1.c)
add_executable(test2 test2.c)

target_compile_options(test1 PRIVATE fsanitize=address PRIVATE fsanitize=leak)
target_compile_options(test2 PRIVATE fsanitize=address PRIVATE fsanitize=leak)

target_link_libraries(test1 libcompiler)
target_link_libraries(test2 libcompiler)