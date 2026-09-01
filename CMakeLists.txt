cmake_minimum_required(VERSION 3.20)

project(Sudoku_Solver)

add_compile_options(-S)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(Sudoku_Solver Main.cpp sudoku_solver.cpp sudoku_solver.h)


