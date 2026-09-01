cmake_minimum_required(VERSION 3.20)

project(Sudoku_Solver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Sudoku_Solver Main.cpp sudoku_solver.cpp sudoku_solver.h)


