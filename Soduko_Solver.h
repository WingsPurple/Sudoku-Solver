/*  Sudoku Solver by Wings Purple
 *  Start date: July 15th 2026
 *  Personal coding project for fun
 *  Goals:
 *  Show hints for what techniques can currently be used
 *  Show the solution to a board one step at a time while showing how each technique was used
 *  Be fully accurate
 *  Be able to determine if it is a solvable/valid sudoku board
 *  Be able to solve all possible valid sudoku boards
 *  Be performant (low memory usage, fast calculations, low file size)
 *  Be available and functional through a website
 *  Have fun and learn :3
 *
 *  Each solve function stops the moment it finds a single success
 *  Priority is as follows: Column > Row > Square
 *  Board 1 requires sudoku elimination 
 *  Board 2 requires single candidate 
 *  Board 3 and 4 require naked pairs
 *  Board 5 requires naked triples
 *
 *  Personal notes:
 *  I have not looked at any other sudoku solvers so there may be other already existing solutions that logic better
 *  but I am doing this for me so :3
 *  
 *  I am using a custom datatype to represent the whole cell which when using bitwise operations
 *  would require 13 bits per cell 4 for the number and 9 for the notations
 *  this can be done with a short/uint16_t (16 bits) allowing you to represent the whole board
 *  in a single array/vector of size 81 for 81 * 16 = 1296 bits or 162 bytes
 *
 *  Alternatively since each cell will only ever hold a value of 0-9 thus only requiring 4 bits
 *  you could use a 2d std::vector of chars (8 bit) which may be more straightforward to implement
 *  1 dimension for the board and another for the notations this would be up to 81 * 9 * 8 = 5832 bits or 729 bytes
 *  a 4.5 times increase in memory usage or 729 - 162 = 567 bytes more memory usage
 *  The 2d vector is still used in the fill_notations_by_sudoku function
 *
 *  Since bitwise operations are pretty fast I am hoping it won't significantly increase the runtime
 *  of the program compared to the 2d vector idea and just letting me benefit fro the smaller memory usage
 *
 *  From what I understand everything gets converted to 64 bits in the alu so the only memory it saves would be in ram
 */
#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>

#define MASK 0x000F

class sudoku
{
public:
    // initialize empty board
    sudoku()
    {
        board.resize(0, 0);
    }

    // the first 4 bits of the uint16_t represent the number in the cell
    // the next 9 represent the notations for numbers 1-9
    // if it is a possible candidate that bit is set to 1 otherwise it is set to 0
    std::vector<uint16_t> board;

    // return only the 4 bits that represent the number
    // not really happy having to use (*this)[] when reading and board[] when writing
    // not sure how to make it so that I can just use one or the other
    // it ddoes make it so writing automatically removes the notations which is nice
    uint16_t operator[](size_t index) {
        return board[index] & MASK;
    }
    
    uint16_t operator[](size_t index) const {
        return board[index] & MASK;
    }

    // read in the board from a file
    bool read(const std::string& filename);

    // keep going until the board is solved
    void solve();

    // solve one step
    void solve_step();

    // prints the current board
    void print_board() const;

    // write a notation for a cell
    void write_notation(const uint16_t cell, const uint16_t number, const uint16_t state);

    void update_notation(const uint16_t cell, const uint16_t number);

    // read a notation for a cell
    bool get_notation(const uint16_t cell, const uint16_t number) const;

    // fills in all notations using only sudoku (row, column, square elimination)
    void fill_notations_by_sudoku();

    // removes all notations from empty cells
    void clear_notations();

    // determines if the current board is solvable
    bool is_solvable() const;

    // checks for single candidate in column, row, and square ( does not use notations )
    std::pair<bool, uint16_t>  solve_by_sudoku();

    std::pair<bool, uint16_t> solve_by_column(uint16_t col, uint16_t cell);

    std::pair<bool, uint16_t> solve_by_row(uint16_t row, uint16_t cell);

    std::pair<bool, uint16_t> solve_by_square(uint16_t col, uint16_t row, uint16_t cell);
    
    
    // checks one square at a time to see if there is only one valid location for a number 
    // relies on notations
    std::pair<bool, uint16_t> solve_by_single_candidate();

    // uses candidate elimination
    // with the knowledge that elsewhere in that square is occupied with a naked pair
    // (two numbers that can both only be in the same two cells in the square)
    std::tuple<bool, uint16_t, uint16_t> solve_by_naked_pairs();

    // same as naked pair but with harder to spot pairs
    bool solve_by_hidden_pairs();

    // uses candidate elimination by identifying triples within a square
    // (three numbers that all only appear as candidates in the same three cells within the square)
    bool solve_by_naked_triples();

    // same as naked triple but with harder to spot triples
    bool solve_by_hidden_triples();

    // does candidate elimination using naked/hidden pairs
    bool solve_by_pointing_pairs();

    // does candidate elimination using naked/hidden triples
    bool solve_by_pointing_triples();

    bool solve_by_x_wing();

    bool solve_by_y_wing();

    bool solve_by_swordfish();
};
