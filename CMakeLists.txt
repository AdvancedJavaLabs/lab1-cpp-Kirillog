cmake_minimum_required(VERSION 3.25)
project(lab1-cpp)

set(CMAKE_CXX_FLAGS_ASAN            "-g -fsanitize=undefined,thread -fno-sanitize-recover=all -D_GLIBCXX_DEBUG"
        CACHE STRING "Compiler flags in asan build"
        FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG ${CMAKE_CXX_FLAGS_ASAN} -Wall -Wextra -Wpedantic -Werror -Wsign-compare")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

add_executable(main main.cpp Graph.cpp RandomGraphGenerator.cpp)
