cmake_minimum_required(VERSION 3.14)
project(Compiler C)

include_directories(./include)

find_package(GTest REQUIRED)

add_subdirectory(src)
add_subdirectory(tests)

add_executable(main ./src/main.c)
target_link_libraries(main libcompiler)