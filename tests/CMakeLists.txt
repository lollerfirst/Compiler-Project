cmake_minimum_required(VERSION 3.14)
project(Compiler C)

add_executable(test1 test1.c)
add_executable(test2 test2.c)


target_link_libraries(test1 libcompiler check)
target_link_directories(test1 PUBLIC /usr/local/lib /usr/lib)
target_include_directories(test1 PUBLIC ../include)
target_compile_options(test1 PUBLIC -g)

target_link_libraries(test2 libcompiler check)
target_link_directories(test2 PUBLIC /usr/local/lib /usr/lib)
target_include_directories(test2 PUBLIC ../include)
target_compile_options(test2 PUBLIC -g)