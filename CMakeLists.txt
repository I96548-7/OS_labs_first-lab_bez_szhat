cmake_minimum_required(VERSION 3.5.0)
project(lab1 VERSION 0.1.0 LANGUAGES C)
include_directories(
    headers
)
add_executable(archivate archivate.c ./src/archivator.c ./src/tree.c ./src/console_functions.c)
add_executable(dearchivate dearchivate.c ./src/archivator.c ./src/tree.c ./src/console_functions.c)
