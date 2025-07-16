cmake_minimum_required(VERSION 3.22)
project(mcc C)

set(CMAKE_C_STANDARD 99)

add_executable(mcc src/mcc/mcc.c
        src/mcc/vm.h
        src/mcc/vm.c
        src/mcc/lexer.h
        src/mcc/lexer.c
        src/mcc/parser.h
        src/mcc/parser.c)
