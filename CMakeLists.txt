cmake_minimum_required(VERSION 3.8)
project(Compiler C)


set(CMAKE_C_STANDARD 23)

enable_testing()

add_definitions(-D_DEBUG)

add_subdirectory(src)
add_subdirectory(tests)

#add_executable(main ./src/main.c)
#target_link_libraries(main libcompiler)
#target_include_directories(main PUBLIC ./include)