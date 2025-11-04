cmake_minimum_required(VERSION 3.10)

project(lab)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(include)
add_compile_options(-Wall -Wextra -Wpedantic -g)

add_library(hexFuncs src/hexFuncs.c)
add_executable(
        main
        src/main.c
)
target_link_libraries(main m hexFuncs)