cmake_minimum_required(VERSION 3.14)
project(Compiler C)

add_library(libcompiler STATIC 
                regexparse.c
                nfa_builder.c
                lexer.c
                parser.c        
        )