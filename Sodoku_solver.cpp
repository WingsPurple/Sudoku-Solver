#include "Soduko_Solver.h"

bool sudoku::read(const std::string& filename)
{
    std::ifstream file(filename);
    // read board from a file
    if (file.is_open())
    {
        while (file.good())
        {
            uint16_t temp;
            file >> temp;
            board.push_back(temp);
        }
        file.close();
    }
    else
    {
        std::cerr << "Can't find input file " << filename << std::endl;
        return false;
    }
    if (board.size() != 81)
    {
        std::cout << "Invalid board size" << std::endl;
        // reset board to be empty
        board.resize(0, 0);
        return false;
    }
    return true;
}

void sudoku::print_board() const
{
    for (uint16_t i = 0; i < 81; i++)
    {
        std::cout << (*this)[i] << " ";
        if ((i + 1) % 9 == 0)
        {
            std::cout << '\n';
            if ((i + 1) % 27 == 0 && i != 80)
            {
                std::cout << "----------------------" << '\n';
            }
        }
        else if (i % 3 == 2)
        {
            std::cout << "| ";
        }
    }
    std::cout << std::endl;
}

void sudoku::solve()
{
    if (is_solvable())
    {
        bool solving = true;
        fill_notations_by_sudoku();
        while (solving)
        {
            solving = std::get<0>(solve_by_naked_pairs());
            solving |= solve_by_sudoku().first;
            solving |= solve_by_single_candidate().first;
        }
        print_board();
    }
}

void sudoku::solve_step()
{
    if (is_solvable())
    {
        // write all basic notations
        fill_notations_by_sudoku();
        // check for naked pairs
        auto [first, cell1, cell2] = solve_by_naked_pairs();
        if (first)
        {
            std::cout << "Found a naked pair: " << cell1 << " " << cell2  << std::endl;
           // print_board();
            // return;
        }
        // check for sudoku
        std::pair<bool, uint16_t> found_something = solve_by_sudoku();
        if (found_something.first)
        {
            std::cout << "Solved by sudoku at cell: " << found_something.second + 1 << std::endl;
            print_board();
            return;
        }
        // check for single candidate
        found_something = solve_by_single_candidate();
        if (found_something.first)
        {
            std::cout << "Solved by single candidate at cell: " << found_something.second + 1 << std::endl;
            print_board();
            return;
        }
    }
}

bool sudoku::is_solvable() const
{
    // std::cout << "Board is unsolvable" << std::endl;
    return true;
}

void sudoku::write_notation(const uint16_t cell, const uint16_t number, const uint16_t state)
{
    if (state == 0 || state == 1)
    {
        // clear the bit and then set it to the state value
        // since the number will always be 1-9 we only have to add 3 to the bit position to shift it correctly
        board[cell] = (board[cell] & ~(1 << (number + 3))) | (state << (number + 3));
    }
}

void sudoku::update_notation(const uint16_t cell, const uint16_t number)
{
    uint16_t col = cell % 9;
    uint16_t row = cell / 9;
    // iterate through the column
    for (uint16_t i = col; i < 81; i += 9)
    {
        write_notation(i, number, 0);
    }
    // iterate through the row
    for (uint16_t i = 0; i < 9; i++)
    {
        write_notation(row * 9 + i, number, 0);
    }
    // iterate through the square
    uint16_t s_col = (cell % 9) / 3;
    uint16_t s_row = (cell / 9) / 3;
    for (uint16_t i = 0; i < 3; i++)
    {
        for (uint16_t j = 0; j < 3; j++)
        {
            uint16_t index = (s_row * 3 + i) * 9 + (s_col * 3 + j);
            write_notation(index, number, 0);
        }
    }
}

bool sudoku::get_notation(const uint16_t cell, const uint16_t number) const
{
    // since the number will always be 1-9 we only have to add 3 to the bit position to shift it correctly
    return board[cell] >> (number + 3) & 1;
}

std::pair<bool, uint16_t> sudoku::solve_by_sudoku()
{
    // iterate through the board
    // if a cell is filled new solutions may have become available so we check again
    for (uint16_t i = 0; i < 81; i++)
    {
        // an empty cell is found
        if ((*this)[i] == 0)
        {
            std::pair<bool, uint16_t> found_something = solve_by_column(i % 9, i);
            if (found_something.first)
            {
                update_notation(found_something.second, (*this)[found_something.second]);
                return found_something;
            }
            found_something = solve_by_row(i / 9, i);
            if (found_something.first)
            {
                update_notation(found_something.second, (*this)[found_something.second]);
                return found_something;
            }
            // top left square is 0, 0, top middle square is 1, 0, top right square is 2, 0
            // middle left square is 0, 1, middle square is 1, 1, middle right square is 2, 1
            // bottom left square is 0, 2, bottom middle square is 1, 2, bottom right square is 2, 2
            found_something = solve_by_square((i % 9) / 3, (i / 9) / 3, i);
            if (found_something.first)
            {
                update_notation(found_something.second, (*this)[found_something.second]);
                return found_something;
            }
        }
    }
    return {false, -1};
}

std::pair<bool, uint16_t> sudoku::solve_by_row(const uint16_t row, const uint16_t cell)
{
    std::vector<uint16_t> temp;
    uint16_t index = row * 9;
    // iterate through row
    for (uint16_t i = index; i < index + 9; i++)
    {
        // skip empty cells
        if ((*this)[i] == 0)
        {
            continue;
        }
        // find every unique number in that row
        if (std::find(temp.begin(), temp.end() , (*this)[i]) == temp.end())
        {
            temp.push_back((*this)[i]);
        }
        // if there are 8 unique numbers, the empty cell is the ninth
        if (temp.size() == 8)
        {
            for (uint16_t number = 1; number <= 9; number++)
            {
                if (std::find(temp.begin(), temp.end(), number) == temp.end())
                {
                    board[cell] = number;
                    return {true, cell};
                }
            }
        }
    }
    return {false, -1};
}

std::pair<bool, uint16_t> sudoku::solve_by_column(const uint16_t col, const uint16_t cell)
{
    std::vector<uint16_t> temp;
    // iterate through column
    for (uint16_t i = col; i < 81; i += 9)
    {
        // skip empty cells
        if ((*this)[i] == 0)
        {
            continue;
        }
        // find every unique number in that column
        if (std::find(temp.begin(), temp.end() , (*this)[i]) == temp.end())
        {
            temp.push_back((*this)[i]);
        }
        // if there are 8 unique numbers, the empty cell is the ninth
        if (temp.size() == 8)
        {
            for (uint16_t number = 1; number <= 9; number++)
            {
                if (std::find(temp.begin(), temp.end(), number) == temp.end())
                {
                    board[cell] = number;
                    return {true, cell};
                }
            }
        }
    }
    return {false, -1};
}

std::pair<bool, uint16_t> sudoku::solve_by_square(const uint16_t col, const uint16_t row, const uint16_t cell)
{
    std::vector<uint16_t> temp;
    // iterate through rows
    for (uint16_t i = 0; i < 3; i++)
    {
        // iterate within row
        for (uint16_t j = 0; j < 3; j++)
        {
            // index is the cell in the square we are checking
            uint16_t index = (row * 3 + i) * 9 + (col * 3 + j);
            // skip empty cells
            if ((*this)[index] == 0)
            {
                continue;
            }
            if (std::find(temp.begin(), temp.end(), (*this)[index]) == temp.end())
            {
                temp.push_back((*this)[index]);
            }
        }
    }
    // if there are 8 unique numbers, the empty cell is the ninth
    if (temp.size() == 8)
    {
        for (uint16_t number = 1; number <= 9; number++)
        {
            if (std::find(temp.begin(), temp.end() , number) == temp.end())
            {
                board[cell] = number;
                return {true, cell};
            }
        }
    }
    return {false, -1};
}


void sudoku::fill_notations_by_sudoku()
{
    std::vector<std::vector<uint16_t>> possibilities(81, std::vector<uint16_t>(9, 1));
    // iterate through all columns
    for (uint16_t col = 0; col < 9; col++)
    {
        for (uint16_t i = col; i < 81; i += 9)
        {
            // if we encounter a number in the column remove it from the list of possible numbers
            if ((*this)[i] != 0)
            {
                for (uint16_t j = col; j < 81; j += 9)
                {
                    possibilities[j][(*this)[i] - 1] = 0;
                }
            }
        }
    }
    // iterate through all rows
    for (uint16_t row = 0; row < 9; row++)
    {
        for (uint16_t i = 0; i < 9; i++)
        {
            // if we encounter a number in the row remove it from the list of possible numbers
            if ((*this)[row * 9 + i] != 0)
            {
                for (uint16_t j = 0; j < 9; j++)
                {
                    possibilities[row * 9 + j][(*this)[row * 9 + i] - 1] = 0;
                }
            }
        }
    }
    // iterate through all squares
    for (uint16_t cell = 0; cell < 81; cell ++)
    {
        // calculate the current row and column
        uint16_t col = (cell % 9) / 3;
        uint16_t row = (cell / 9) / 3;
        // iterate through rows of the square
        for (uint16_t i = 0; i < 3; i++)
        {
            // iterate within row
            for (uint16_t j = 0; j < 3; j++)
            {
                // index is the cell in the square we are checking
                uint16_t index = (row * 3 + i) * 9 + (col * 3 + j);
                // if we encounter a number in the square remove it from the list of possible numbers
                if ((*this)[index] != 0)
                {
                    // iterate through rows of the square
                    for (uint16_t k = 0; k < 3; k++)
                    {
                        // iterate within row
                        for (uint16_t l = 0; l < 3; l++)
                        {
                            uint16_t write_index = (row * 3 + k) * 9 + (col * 3 + l);
                            possibilities[write_index][(*this)[index] - 1] = 0;
                        }
                    }
                }
            }
        }
        // write the notations for that cell
        for (uint16_t i = 0; i < 9; i++)
        {
            if (possibilities[cell][i] != 0 && (*this)[cell] == 0)
            {
                // finally write the notations for the cell
                write_notation(cell, i + 1, possibilities[cell][i]);
            }
        }
    }
}

void sudoku::clear_notations()
{
    for (int i = 0; i < 81; i++)
    {
        if ((board[i] & MASK) == 0)
        {
            board[i] = 0;
        }
    }
}

std::pair<bool, uint16_t> sudoku::solve_by_single_candidate()
{
    // iterate through all numbers
    for (uint16_t number = 1; number <= 9; number++)
    {
        // check all columns
        for (uint16_t col = 0; col < 9; col++)
        {
            uint16_t counter = 0;
            uint16_t cell = 0;
            for (uint16_t i = col; i < 81; i += 9)
            {
                if ((*this)[i] == number)
                {
                    break;
                }
                if (get_notation(i, number))
                {
                    counter += 1;
                    cell = i;
                }
            }
            if (counter == 1)
            {
                board[cell] = number;
                update_notation(cell, number);
                return {true, cell};
            }
        }
        // check all rows
        for (uint16_t row = 0; row < 9; ++row)
        {
            uint16_t counter = 0;
            uint16_t cell = 0;
            uint16_t found = 0;
            for (uint16_t i = 0; i < 9; i ++)
            {
                cell = row * 9 + i;
                if ((*this)[cell] == number)
                {
                    break;
                }
                if (get_notation(cell, number))
                {
                    counter += 1;
                    found = cell;
                }
            }
            if (counter == 1)
            {
                board[found] = number;
                update_notation(found, number);
                return {true, found};
            }
        }
        // check all squares
        // iterate through all squares
        for (uint16_t cell = 0; cell < 81; cell += 7)
        {
            // calculate the current row and column
            uint16_t col = (cell % 9) / 3;
            uint16_t row = (cell / 9) / 3;
            // iterate through rows of the square
            uint16_t index = 0;
            uint16_t counter = 0;
            uint16_t found = 0;
            bool skip = false;
            for (uint16_t i = 0; i < 3; i++)
            {
                if (skip)
                {
                    break;
                }
                // iterate within row
                for (uint16_t j = 0; j < 3; j++)
                {
                    // index is the cell in the square we are checking
                    index = (row * 3 + i) * 9 + (col * 3 + j);
                    // if the number already exists in the square, skip this square
                    if ((*this)[index] == number)
                    {
                        skip = true;
                        break;
                    }
                    if (get_notation(index, number))
                    {
                        counter += 1;
                        found = index;
                    }
                }
            }
            if (counter == 1)
            {
                board[found] = number;
                update_notation(found, number);
                return {true, found};
            }
        }
    }
    return {false, -1};
}


std::tuple<bool, uint16_t, uint16_t> sudoku::solve_by_naked_pairs()
{
    // iterate through whole board
    for (uint16_t cell = 0; cell < 81; cell++)
    {
        uint16_t count = 0;
        std::vector<uint16_t> pair1{};
        for (uint16_t i = 1; i <= 9; i++)
        {
            if (get_notation(cell, i) == true)
            {
                count += 1;
                pair1.push_back(i);
            }
        }
        // if a cell is found with exactly 2 notations try to find another with the same exact 2 notations
        if (count == 2)
        {
            uint16_t row = cell / 9;
            uint16_t col = cell % 9;
            uint16_t s_row = (cell / 9) / 3;
            uint16_t s_col = (cell % 9) / 3;
            // check current column
            for (uint16_t i = col; i < 81; i += 9)
            {
                if (i == cell)
                {
                    continue;
                }
                count = 0;
                std::vector<uint16_t> pair2{};
                for (uint16_t j = 1; j <= 9; j++)
                {
                    if (get_notation(i, j) == true)
                    {
                        count += 1;
                        pair2.push_back(j);
                    }
                }
                // if an identical pair is found remove the found notations from the rest of the column
                if (count == 2 && pair1 == pair2)
                {
                    for (uint16_t k = col; k < 81; k += 9)
                    {
                        if (k == cell || k == i)
                        {
                            continue;
                        }
                        write_notation(k, pair1.front(), 0);
                        write_notation(k, pair1.back(), 0);
                    }
                    return {true, cell, i};
                }
            }
            // check current row
            for (uint16_t i = 0; i < 9; i++)
            {
                count = 0;
                std::vector<uint16_t> pair2{};
                uint16_t index = s_row * 9 + i;
                // ignore the cell we already found
                if (index == cell)
                {
                    continue;
                }
                for (uint16_t j = 1; j <= 9; j++)
                {
                    if (get_notation(index, j) == true)
                    {
                        count += 1;
                        pair2.push_back(j);
                    }
                }
                // if an identical pair is found remove the found notations from the rest of the row
                if (count == 2 && pair1 == pair2)
                {
                    for (uint16_t k = 0; k < 9; k++)
                    {
                        if (row * 9 + k == cell || row * 9 + k == index)
                        {
                            continue;
                        }
                        write_notation(row * 9 + k, pair1.front(), 0);
                        write_notation(row * 9 + k, pair1.back(), 0);
                    }
                    return {true, cell, index};
                }
            }
            // check current square
            for (uint16_t i = 0; i < 3; i++)
            {
                // iterate within row
                for (uint16_t j = 0; j < 3; j++)
                {
                    count = 0;
                    std::vector<uint16_t> pair2{};
                    // index is the cell in the square we are checking
                    uint16_t index = (s_row * 3 + i) * 9 + (s_col * 3 + j);
                    // ignore the cell we already found
                    if (cell == index)
                    {
                        continue;
                    }
                    for (uint16_t k = 1; k <= 9; k++)
                    {
                        if (get_notation(index, k) == true)
                        {
                            count += 1;
                            pair2.push_back(k);
                        }
                    }
                    // if an identical pair is found remove the found notations from the rest of the square
                    if (count == 2 && pair1 == pair2)
                    {
                        for (uint16_t k = 0; k < 3; k++)
                        {
                            // iterate within row
                            for (uint16_t l = 0; l < 3; l++)
                            {
                                uint16_t final_index = (s_row * 3 + k) * 9 + (s_col * 3 + l);
                                if (final_index == cell || final_index == index)
                                {
                                    continue;
                                }
                                write_notation(final_index, pair1.front(), 0);
                                write_notation(final_index, pair1.back(), 0);
                            }
                        }
                        return {true, cell, index};
                    }
                }
            }
        }
    }
    return {false, -1, -1};
}

/*
bool sudoku::solve_by_hidden_pairs()
{
}

bool sudoku::solve_by_naked_triples()
{
}

bool sudoku::solve_by_hidden_triples()
{
}

bool sudoku::solve_by_pointing_pairs()
{
}

bool sudoku::solve_by_pointing_triples()
{
}

bool sudoku::solve_by_x_wing()
{
}

bool sudoku::solve_by_y_wing()
{
}

bool sudoku::solve_by_swordfish()
{
}

*/
