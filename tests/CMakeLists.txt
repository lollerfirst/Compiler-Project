cmake_minimum_required(VERSION 3.8)
project(Compiler C)

cmake_policy(SET CMP0043 NEW)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-D_DEBUG)
endif()

add_executable(test1 test1.c)
add_executable(test2 test2.c)
add_executable(test3 test3.c)
add_executable(test4 test4.c)
add_executable(build_collection build_collection.c)


target_link_libraries(test1 libcompiler)
target_include_directories(test1 PUBLIC ../include)
target_compile_options(test1 PUBLIC -g)

target_link_libraries(test2 libcompiler)
target_include_directories(test2 PUBLIC ../include)
target_compile_options(test2 PUBLIC -g)

target_link_libraries(test3 libcompiler)
target_include_directories(test3 PUBLIC ../include)
target_compile_options(test3 PUBLIC -g)

target_link_libraries(test4 libcompiler)
target_include_directories(test4 PUBLIC ../include)
target_compile_options(test4 PUBLIC -g)

target_link_libraries(build_collection libcompiler)
target_include_directories(build_collection PUBLIC ../include)
target_compile_options(build_collection PUBLIC -g)