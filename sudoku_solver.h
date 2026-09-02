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
 *  of the program compared to the 2d vector idea and just letting me benefit from the smaller memory usage
 *  
 *  Using 8 bit for everything that can be 8 bit probably not speed optimal but this is minimum ram usage challenge
 *  I imposed on myself so :3
 *  Might try to do a minimum run time version after
 *
 *  From what I understand everything gets converted to 64 bits in the ALU so the only memory it saves would be in ram
 */
#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <cstdint>
#include <algorithm>

#define MASK 0x0F
#define U8_SC static_cast<uint8_t>
#define U16_SC static_cast<uint16_t>

class sudoku
{
    enum type : uint8_t
    {
        NONE,
        SUDOKU,
        SINGLE
    };
    // the first 4 bits of the uint16_t represent the number in the cell
    // the next 9 represent the notations for numbers 1-9
    // if it is a possible candidate that bit is set to 1 otherwise it is set to 0
    std::vector<uint16_t> board;
    
    // write to a cell
    void write(uint8_t cell, uint16_t number, type t);

    // write a notation for a cell
    void write_notation(const uint8_t cell, const uint8_t number, const uint8_t state);

    // helper function for writing to a cell
    // deletes all notation of the relevant number in that row/column/square
    void update_notation(const uint8_t cell);

    // read a notation for a cell
    bool get_notation(const uint8_t cell, const uint8_t number) const;

    // fills in all notations using only sudoku (row, column, square elimination)
    void fill_notations_by_sudoku();

    // removes all notations from empty cells
    void clear_notations();

    // checks for single candidate in column, row, and square ( does not use notations )
    bool solve_by_sudoku();

    bool solve_by_column(uint8_t col, uint8_t cell);

    bool solve_by_row(uint8_t row, uint8_t cell);

    bool solve_by_square(uint8_t col, uint8_t row, uint8_t cell);
    
    
    // checks one square at a time to see if there is only one valid location for a number 
    // relies on notations
    bool solve_by_single_candidate();

    // uses candidate elimination
    // with the knowledge that elsewhere in that square is occupied with a naked pair
    // (two numbers that can both only be in the same two cells in the square)
    void solve_by_naked_pairs();

    // uses candidate elimination by identifying triples
    // (three numbers that all only appear as candidates in the same three cells within the column/row/square)
    // only cell needs to contain all three the other two cells just need to contain at least two of the same three 
    // as the one with all three
    void solve_by_naked_triples();
    
    // uses candidate elimination by identifying quads
    // (four numbers that all only appear as candidates in the same four cells within the column/row/square)
    // only cell needs to contain all four the other two cells just need to contain at least two of the same four 
    // as the one with all four
    std::pair<bool, std::vector<uint16_t>> solve_by_naked_quads();
    
    // helper function for triples and quads
    // checks if at least 2 of the notations are the same between vectors
    static bool tiple_or_quad(const std::vector<uint8_t>& list, const std::vector<uint8_t>& candidate);
    
    // same as naked pair but other notations are in the cells with the pairs
    void solve_by_hidden_pairs();

    // same as naked triples but other notations are in the cells with the pairs
    bool solve_by_hidden_triples();
    
    // same as naked pair but other notations are in the cells with the pairs
    bool solve_by_hidden_quads();

    bool solve_by_x_wing();

    bool solve_by_y_wing();

    bool solve_by_swordfish();
public:
    // initialize empty board
    sudoku()
    {
        board.resize(0, 0);
    }
    
    // initialize from file
    explicit sudoku(const std::string& filename)
    {
        board.resize(0, 0);
        read(filename);
    }
    
    // read in the board from a file
    bool read(const std::string& filename);
    
    // prints the current board
    void print_board(uint8_t highlight = 100, type t = NONE) const;
    
    void diff(const sudoku& rhs) const;
    
    // determines if the current board is solvable
    bool is_solvable() const;
    
    // keep going until the board is solved
    void solve();

    // solve one step
    bool solve_step();

    // return only the 4 bits that represent the number
    // not really happy having to use (*this)[] when reading and board[] when writing
    // not sure how to make it so that I can just use one or the other
    // it does make it so writing automatically removes the notations which is nice
    uint16_t operator[](const size_t index) {
        return board[index] & MASK;
    }
    
    // const version
    uint16_t operator[](const size_t index) const {
        return board[index] & MASK;
    }
    
    // overload comparison operator so we can compare boards
    bool operator==(const sudoku& rhs) const
    {
        return board == rhs.board;
    }
    
    // overload comparison operator so we can compare boards
    bool operator!=(const sudoku& rhs) const
    {
        return board != rhs.board;
    }
};
