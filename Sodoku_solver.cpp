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
    if (board.size() !=static_cast<uint8_t>(81))
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
    for (uint8_t i = 0; i < static_cast<uint8_t>(81); i++)
    {
        std::cout << (*this)[i] << " ";
        if ((i + static_cast<uint8_t>(1)) % static_cast<uint8_t>(9) == static_cast<uint8_t>(0))
        {
            std::cout << '\n';
            if ((i + static_cast<uint8_t>(1)) % static_cast<uint8_t>(27) == 
                static_cast<uint8_t>(0) && i != static_cast<uint8_t>(80))
            {
                std::cout << "----------------------" << '\n';
            }
        }
        else if (i % static_cast<uint8_t>(3) == static_cast<uint8_t>(2))
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
            solving = std::get< static_cast<uint8_t>(0)>(solve_by_naked_pairs());
            solving |= solve_by_sudoku().first;
            solving |= solve_by_single_candidate().first;
        }
        print_board();
    }
}

bool sudoku::solve_step()
{
    if (is_solvable())
    {
        // write all basic notations
        fill_notations_by_sudoku();
        // check for naked pairs
        if (auto [first, cell1, cell2] = solve_by_naked_pairs(); first)
        {
            std::cout << "Found a naked pair: " << cell1 << " " << cell2  << std::endl;
        }
        // check for naked triples
        if (auto [found, vect] = solve_by_naked_triples(); found)
        {
            std::cout << "Found a naked triple at " << vect[0] << " " << vect[1] << " " << vect[2] << std::endl;
        }
        // check for sudoku
        std::pair<bool, uint16_t> found_something = solve_by_sudoku();
        if (found_something.first)
        {
            std::cout << "Solved by sudoku at cell: " << found_something.second + 1 << std::endl;
            print_board();
            return true;
        }
        // check for single candidate
        found_something = solve_by_single_candidate();
        if (found_something.first)
        {
            std::cout << "Solved by single candidate at cell: " << found_something.second + 1 << std::endl;
            print_board();
            return true;
        }
    }
    return false;
}

bool sudoku::is_solvable() const
{
    // std::cout << "Board is unsolvable" << std::endl;
    return true;
}

void sudoku::write_notation(const uint8_t cell, const uint8_t number, const uint8_t state)
{
    if (state == static_cast<uint8_t>(0) || state == static_cast<uint8_t>(1))
    {
        // clear the bit and then set it to the state value
        // since the number will always be 1-9 
        // we only have to add static_cast<uint8_t>(3) to the bit position to shift it correctly
        board[cell] = (board[cell] & ~(static_cast<uint8_t>(1) << (number + static_cast<uint8_t>(3)))) | 
            (state << (number + static_cast<uint8_t>(3)));
    }
}

void sudoku::update_notation(const uint8_t cell)
{
    const uint8_t number = static_cast<uint8_t>((*this)[cell]);
    const uint8_t col = cell % static_cast<uint8_t>(9);
    const uint8_t row = cell / static_cast<uint8_t>(9);
    // iterate through the column
    for (uint8_t i = col; i < static_cast<uint8_t>(81); i += static_cast<uint8_t>(9))
    {
        write_notation(i, number, 0);
    }
    // iterate through the row
    for (uint8_t i = 0; i < static_cast<uint8_t>(9); i++)
    {
        write_notation(row * static_cast<uint8_t>(9) + i, number, 0);
    }
    // iterate through the square
    const uint8_t s_col = (cell %static_cast<uint8_t>(9)) / static_cast<uint8_t>(3);
    const uint8_t s_row = (cell /static_cast<uint8_t>(9)) / static_cast<uint8_t>(3);
    for (uint8_t i = 0; i < static_cast<uint8_t>(3); i++)
    {
        for (uint8_t j = 0; j < static_cast<uint8_t>(3); j++)
        {
            const uint8_t index = (s_row * static_cast<uint8_t>(3) + i) *
                static_cast<uint8_t>(9) + (s_col * static_cast<uint8_t>(3) + j);
            write_notation(index, number, 0);
        }
    }
}

bool sudoku::get_notation(const uint8_t cell, const uint8_t number) const
{
    // since the number will always be 1-9 we only have to add 3 to the bit position to shift it correctly
    return board[cell] >> (number + static_cast<uint8_t>(3)) & 1;
}

std::pair<bool, uint8_t> sudoku::solve_by_sudoku()
{
    // iterate through the board
    // if a cell is filled new solutions may have become available so we check again
    for (uint8_t i = 0; i < static_cast<uint8_t>(81); i++)
    {
        // an empty cell is found
        if ((*this)[i] == static_cast<uint8_t>(0))
        {
            std::pair<bool, uint8_t> found_something = solve_by_column(i % static_cast<uint8_t>(9), i);
            if (found_something.first)
            {
                update_notation(found_something.second);
                return found_something;
            }
            found_something = solve_by_row(i / static_cast<uint8_t>(9), i);
            if (found_something.first)
            {
                update_notation(found_something.second);
                return found_something;
            }
            // top left square is 0, 0, top middle square is 1, 0, top right square is 2, 0
            // middle left square is 0, 1, middle square is 1, 1, middle right square is 2, 1
            // bottom left square is 0, 2, bottom middle square is 1, 2, bottom right square is 2, 2
            found_something = solve_by_square(
                (i % static_cast<uint8_t>(9)) / static_cast<uint8_t>(3), 
                (i / static_cast<uint8_t>(9)) / static_cast<uint8_t>(3), i);
            if (found_something.first)
            {
                update_notation(found_something.second);
                return found_something;
            }
        }
    }
    return {false, -1};
}

std::pair<bool, uint8_t> sudoku::solve_by_row(const uint8_t row, const uint8_t cell)
{
    std::vector<uint16_t> temp;
    const uint8_t index = row * static_cast<uint8_t>(9);
    // iterate through row
    for (uint8_t i = index; i < index + static_cast<uint8_t>(9); i++)
    {
        // skip empty cells
        if ((*this)[i] == static_cast<uint8_t>(0))
        {
            continue;
        }
        // find every unique number in that row
        if (std::find(temp.begin(), temp.end() , (*this)[i]) == temp.end())
        {
            temp.push_back((*this)[i]);
        }
        // if there are 8 unique numbers, the empty cell is the ninth
        if (temp.size() == static_cast<uint8_t>(8))
        {
            for (uint8_t number = 1; number <= static_cast<uint8_t>(9); number++)
            {
                if (std::find(temp.begin(), temp.end(), number) == temp.end())
                {
                    board[cell] = number;
                    return {true, cell};
                }
            }
        }
    }
    return {false, static_cast<uint8_t>(-1)};
}

std::pair<bool, uint8_t> sudoku::solve_by_column(const uint8_t col, const uint8_t cell)
{
    std::vector<uint16_t> temp;
    // iterate through column
    for (uint8_t i = col; i < static_cast<uint8_t>(81); i += static_cast<uint8_t>(9))
    {
        // skip empty cells
        if ((*this)[i] == static_cast<uint8_t>(0))
        {
            continue;
        }
        // find every unique number in that column
        if (std::find(temp.begin(), temp.end() , (*this)[i]) == temp.end())
        {
            temp.push_back((*this)[i]);
        }
        // if there are 8 unique numbers, the empty cell is the ninth
        if (temp.size() == static_cast<uint8_t>(8))
        {
            for (uint8_t number = 1; number <= static_cast<uint8_t>(9); number++)
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

std::pair<bool, uint8_t> sudoku::solve_by_square(const uint8_t col, const uint8_t row, const uint8_t cell)
{
    std::vector<uint16_t> temp;
    // iterate through rows
    for (uint8_t i = 0; i < static_cast<uint8_t>(3); i++)
    {
        // iterate within row
        for (uint8_t j = 0; j < static_cast<uint8_t>(3); j++)
        {
            // index is the cell in the square we are checking
            const uint8_t index = 
                (row * static_cast<uint8_t>(3) + i) * static_cast<uint8_t>(9) + (col * static_cast<uint8_t>(3) + j);
            // skip empty cells
            if ((*this)[index] == static_cast<uint8_t>(0))
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
    if (temp.size() == static_cast<uint8_t>(8))
    {
        for (uint8_t number = 1; number <= static_cast<uint8_t>(9); number++)
        {
            if (std::find(temp.begin(), temp.end() , number) == temp.end())
            {
                board[cell] = number;
                return {true, cell};
            }
        }
    }
    return {false, static_cast<uint8_t>(-1)};
}


void sudoku::fill_notations_by_sudoku()
{
    std::vector possibilities(static_cast<uint8_t>(81), std::vector<uint8_t>(9, 1));
    // iterate through all columns
    for (uint8_t col = 0; col < static_cast<uint8_t>(9); col++)
    {
        for (uint8_t i = col; i < static_cast<uint8_t>(81); i += static_cast<uint8_t>(9))
        {
            // if we encounter a number in the column remove it from the list of possible numbers
            if ((*this)[i] != static_cast<uint8_t>(0))
            {
                for (uint8_t j = col; j < static_cast<uint8_t>(81); j += static_cast<uint8_t>(9))
                {
                    possibilities[j][(*this)[i] - 1] = static_cast<uint8_t>(0);
                }
            }
        }
    }
    // iterate through all rows
    for (uint8_t row = 0; row < static_cast<uint8_t>(9); row++)
    {
        for (uint8_t i = 0; i < static_cast<uint8_t>(9); i++)
        {
            // if we encounter a number in the row remove it from the list of possible numbers
            if ((*this)[row * static_cast<uint8_t>(9) + i] != static_cast<uint8_t>(0))
            {
                for (uint8_t j = 0; j < static_cast<uint8_t>(9); j++)
                {
                    possibilities[row * static_cast<uint8_t>(9) + j]
                    [(*this)[row * static_cast<uint8_t>(9) + i] - 1] = static_cast<uint8_t>(0);
                }
            }
        }
    }
    // iterate through all squares
    for (uint8_t cell = 0; cell < static_cast<uint8_t>(81); cell ++)
    {
        // calculate the current row and column
        const uint16_t col = (cell % static_cast<uint8_t>(9)) / static_cast<uint8_t>(3);
        const uint16_t row = (cell / static_cast<uint8_t>(9)) / static_cast<uint8_t>(3);
        // iterate through rows of the square
        for (uint8_t i = 0; i < static_cast<uint8_t>(3); i++)
        {
            // iterate within row
            for (uint8_t j = 0; j < static_cast<uint8_t>(3); j++)
            {
                // index is the cell in the square we are checking
                const uint16_t index = (row * static_cast<uint8_t>(3) + i) * static_cast<uint8_t>(9) + 
                    (col * static_cast<uint8_t>(3) + j);
                // if we encounter a number in the square remove it from the list of possible numbers
                if ((*this)[index] != static_cast<uint8_t>(0))
                {
                    // iterate through rows of the square
                    for (uint8_t k = 0; k < static_cast<uint8_t>(3); k++)
                    {
                        // iterate within row
                        for (uint8_t l = 0; l < static_cast<uint8_t>(3); l++)
                        {
                            const uint16_t write_index = (row * static_cast<uint8_t>(3) + k) * 
                                static_cast<uint8_t>(9) + (col * static_cast<uint8_t>(3) + l);
                            possibilities[write_index][(*this)[index] - 1] = static_cast<uint8_t>(0);
                        }
                    }
                }
            }
        }
        // write the notations for that cell
        for (uint8_t i = 0; i < static_cast<uint8_t>(9); i++)
        {
            if (possibilities[cell][i] != static_cast<uint8_t>(0) && (*this)[cell] == static_cast<uint8_t>(0))
            {
                // finally write the notations for the cell
                write_notation(cell, i + 1, possibilities[cell][i]);
            }
        }
    }
}

void sudoku::clear_notations()
{
    for (int i = static_cast<uint8_t>(0); i < static_cast<uint8_t>(81); i++)
    {
        if ((board[i] & MASK) == static_cast<uint8_t>(0))
        {
            board[i] = static_cast<uint8_t>(0);
        }
    }
}

std::pair<bool, uint16_t> sudoku::solve_by_single_candidate()
{
    // iterate through all numbers
    for (uint8_t number = 1; number <= static_cast<uint8_t>(9); number++)
    {
        // check all columns
        for (uint8_t col = 0; col < static_cast<uint8_t>(9); col++)
        {
            uint8_t counter = 0;
            uint8_t cell = 0;
            for (uint8_t i = col; i < static_cast<uint8_t>(81); i += static_cast<uint8_t>(9))
            {
                if ((*this)[i] == number)
                {
                    break;
                }
                if (get_notation(i, number))
                {
                    counter += static_cast<uint8_t>(1);
                    cell = i;
                }
            }
            if (counter == static_cast<uint8_t>(1))
            {
                board[cell] = number;
                update_notation(cell);
                return {true, cell};
            }
        }
        // check all rows
        for (uint8_t row = 0; row < static_cast<uint8_t>(9); ++row)
        {
            uint8_t counter = 0;
            uint8_t found = 0;
            for (uint8_t i = 0; i < static_cast<uint8_t>(9); i ++)
            {
                const uint8_t cell = row * static_cast<uint8_t>(9) + i;
                if ((*this)[cell] == number)
                {
                    break;
                }
                if (get_notation(cell, number))
                {
                    counter += static_cast<uint8_t>(1);
                    found = cell;
                }
            }
            if (counter == static_cast<uint8_t>(1))
            {
                board[found] = number;
                update_notation(found);
                return {true, found};
            }
        }
        // check all squares
        // iterate through all squares
        for (uint8_t cell = 0; cell < static_cast<uint8_t>(81); cell += 7)
        {
            // calculate the current row and column
            const uint8_t col = (cell % static_cast<uint8_t>(9)) / static_cast<uint8_t>(3);
            const uint8_t row = (cell / static_cast<uint8_t>(9)) / static_cast<uint8_t>(3);
            // iterate through rows of the square
            uint8_t counter = 0;
            uint8_t found = 0;
            bool skip = false;
            for (uint8_t i = 0; i < static_cast<uint8_t>(3); i++)
            {
                if (skip)
                {
                    break;
                }
                // iterate within row
                for (uint8_t j = 0; j < static_cast<uint8_t>(3); j++)
                {
                    // index is the cell in the square we are checking
                    const uint8_t index = (row * static_cast<uint8_t>(3) + i) *
                        static_cast<uint8_t>(9) + (col * static_cast<uint8_t>(3) + j);
                    // if the number already exists in the square, skip this square
                    if ((*this)[index] == number)
                    {
                        skip = true;
                        break;
                    }
                    if (get_notation(index, number))
                    {
                        counter += static_cast<uint8_t>(1);
                        found = index;
                    }
                }
            }
            if (counter == static_cast<uint8_t>(1))
            {
                board[found] = number;
                update_notation(found);
                return {true, found};
            }
        }
    }
    return {false, -1};
}


std::tuple<bool, uint16_t, uint16_t> sudoku::solve_by_naked_pairs()
{
    // iterate through whole board
    for (uint8_t cell = 0; cell < static_cast<uint8_t>(81); cell++)
    {
        uint8_t count = 0;
        std::vector<uint8_t> pair1{};
        for (uint8_t i = 1; i <= static_cast<uint8_t>(9); i++)
        {
            if (get_notation(cell, i) == true)
            {
                count += static_cast<uint8_t>(1);
                pair1.push_back(i);
            }
        }
        // if a cell is found with exactly 2 notations try to find another with the same exact 2 notations
        if (count == static_cast<uint8_t>(2))
        {
            const uint8_t row = cell /static_cast<uint8_t>(9);
            const uint8_t col = cell %static_cast<uint8_t>(9);
            const uint8_t s_row = (cell /static_cast<uint8_t>(9)) / static_cast<uint8_t>(3);
            const uint8_t s_col = (cell %static_cast<uint8_t>(9)) / static_cast<uint8_t>(3);
            // check current column
            for (uint8_t i = col; i < static_cast<uint8_t>(81); i += static_cast<uint8_t>(9))
            {
                if (i == cell)
                {
                    continue;
                }
                count = static_cast<uint8_t>(0);
                std::vector<uint8_t> pair2{};
                for (uint8_t j = 1; j <= static_cast<uint8_t>(9); j++)
                {
                    if (get_notation(i, j) == true)
                    {
                        count += static_cast<uint8_t>(1);
                        pair2.push_back(j);
                    }
                }
                // if an identical pair is found remove the found notations from the rest of the column
                if (count == static_cast<uint8_t>(2) && pair1 == pair2)
                {
                    for (uint8_t k = col; k < static_cast<uint8_t>(81); k += static_cast<uint8_t>(9))
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
            for (uint8_t i = 0; i < static_cast<uint8_t>(9); i++)
            {
                count = static_cast<uint8_t>(0);
                std::vector<uint8_t> pair2{};
                const uint8_t index = row * static_cast<uint8_t>(9) + i;
                // ignore the cell we already found
                if (index == cell)
                {
                    continue;
                }
                for (uint8_t j = 1; j <= static_cast<uint8_t>(9); j++)
                {
                    if (get_notation(index, j) == true)
                    {
                        count += 1;
                        pair2.push_back(j);
                    }
                }
                // if an identical pair is found remove the found notations from the rest of the row
                if (count == static_cast<uint8_t>(2) && pair1 == pair2)
                {
                    for (uint8_t k = 0; k < static_cast<uint8_t>(9); k++)
                    {
                        if (row * static_cast<uint8_t>(9) + k == cell || row * static_cast<uint8_t>(9) + k == index)
                        {
                            continue;
                        }
                        write_notation(row * static_cast<uint8_t>(9) + k, pair1.front(), 0);
                        write_notation(row * static_cast<uint8_t>(9) + k, pair1.back(), 0);
                    }
                    return {true, cell, index};
                }
            }
            // check current square
            for (uint8_t i = 0; i < static_cast<uint8_t>(3); i++)
            {
                // iterate within row
                for (uint8_t j = 0; j < static_cast<uint8_t>(3); j++)
                {
                    count = static_cast<uint8_t>(0);
                    std::vector<uint8_t> pair2{};
                    // index is the cell in the square we are checking
                    const uint8_t index = (s_row * static_cast<uint8_t>(3) + i) *
                        static_cast<uint8_t>(9) + (s_col * static_cast<uint8_t>(3) + j);
                    // ignore the cell we already found
                    if (cell == index)
                    {
                        continue;
                    }
                    for (uint8_t k = 1; k <= static_cast<uint8_t>(9); k++)
                    {
                        if (get_notation(index, k) == true)
                        {
                            count += static_cast<uint8_t>(1);
                            pair2.push_back(k);
                        }
                    }
                    // if an identical pair is found remove the found notations from the rest of the square
                    if (count == static_cast<uint8_t>(2) && pair1 == pair2)
                    {
                        // iterate through rows
                        for (uint8_t k = 0; k < static_cast<uint8_t>(3); k++)
                        {
                            // iterate within row
                            for (uint8_t l = 0; l < static_cast<uint8_t>(3); l++)
                            {
                                const uint8_t final_index = (s_row * static_cast<uint8_t>(3) + k) * 
                                    static_cast<uint8_t>(9) + (s_col * static_cast<uint8_t>(3) + l);
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

std::pair<bool, std::vector<uint16_t>> sudoku::solve_by_naked_triples()
{
    // iterate through whole board
    for (uint8_t cell = 0; cell < static_cast<uint8_t>(81); cell++)
    {
        uint8_t count = 0;
        std::vector<uint8_t> triple1{};
        for (uint8_t i = 1; i <= static_cast<uint8_t>(9); i++)
        {
            if (get_notation(cell, i) == true)
            {
                count += static_cast<uint8_t>(1);
                triple1.push_back(i);
            }
        }
        // if a cell is found with exactly 3 notations try to find 2 more with at least 2 of the same 3 notations
        if (count == static_cast<uint8_t>(3))
        {
            // cell 100 is not a valid cell so it is used instead of -1
            uint8_t cell2 = 100;
            const uint8_t row = cell /static_cast<uint8_t>(9);
            const uint8_t col = cell %static_cast<uint8_t>(9);
            const uint8_t s_row = (cell /static_cast<uint8_t>(9)) / static_cast<uint8_t>(3);
            const uint8_t s_col = (cell %static_cast<uint8_t>(9)) / static_cast<uint8_t>(3);
            // check current column
            for (uint8_t i = col; i < static_cast<uint8_t>(81); i += static_cast<uint8_t>(9))
            {
                if (i == cell)
                {
                    continue;
                }
                count = static_cast<uint8_t>(0);
                std::vector<uint8_t> triple2{};
                for (uint8_t j = 1; j <= static_cast<uint8_t>(9); j++)
                {
                    if (get_notation(i, j) == true)
                    {
                        count += static_cast<uint8_t>(1);
                        triple2.push_back(j);
                    }
                }
                if (count > static_cast<uint8_t>(1) && count < static_cast<uint8_t>(4)
                    && tiple_or_quad(triple1, triple2) == true)
                {
                    // if a cell with 2/3 or 3/3 notations is found remember it
                    if (cell2 == 100)
                    {
                        cell2 = i;
                    }
                    // if a third is found remove the notations for the 3 numbers from the rest of the column
                    if (cell2 != 100 && cell2 != i)
                    {
                        for (uint8_t k = col; k < static_cast<uint8_t>(81); k += static_cast<uint8_t>(9))
                        {
                            if (k == cell || k == i || k == cell2)
                            {
                                continue;
                            }
                            for (const auto t : triple1)
                            {
                                write_notation(k, t, 0);
                            }
                        }
                        return {true, std::vector{static_cast<uint16_t>(cell), 
                            static_cast<uint16_t>(cell2), static_cast<uint16_t>(i)}};
                    }
                }
            }
            // check current row
            for (uint8_t i = 0; i < static_cast<uint8_t>(9); i++)
            {
                count = static_cast<uint8_t>(0);
                std::vector<uint8_t> triple2{};
                const uint8_t index = row * static_cast<uint8_t>(9) + i;
                // ignore the cell we already found
                if (index == cell)
                {
                    continue;
                }
                for (uint8_t j = 1; j <= static_cast<uint8_t>(9); j++)
                {
                    if (get_notation(index, j) == true)
                    {
                        count += static_cast<uint8_t>(1);
                        triple2.push_back(j);
                    }
                }
                if (count > static_cast<uint8_t>(1) && count < static_cast<uint8_t>(4)
                    && tiple_or_quad(triple1, triple2) == true)
                {
                    // if a cell with 2/3 or 3/3 notations is found remember it
                    if (cell2 == 100)
                    {
                        cell2 = index;
                    }
                    // if a third is found remove the notations for the 3 numbers from the rest of the row
                    if (cell2 != 100 && cell2 != index)
                    {
                        for (uint8_t k = 0; k < static_cast<uint8_t>(9); k++)
                        {
                            const uint8_t temp = row * static_cast<uint8_t>(9) + k;
                            if (temp == cell || temp == index || temp == cell2)
                            {
                                continue;
                            }
                            for (const auto t : triple1)
                            {
                                write_notation(temp, t, 0);
                            }
                        }
                        return {true, std::vector{static_cast<uint16_t>(cell), 
                            static_cast<uint16_t>(cell2), static_cast<uint16_t>(index)}};
                    }
                }
            }
            // check current square
            for (uint8_t i = 0; i < static_cast<uint8_t>(3); i++)
            {
                // iterate within row
                for (uint8_t j = 0; j < static_cast<uint8_t>(3); j++)
                {
                    count = static_cast<uint8_t>(0);
                    std::vector<uint8_t> triple2{};
                    // index is the cell in the square we are checking
                    const uint8_t index = (s_row * static_cast<uint8_t>(3) + i) *
                        static_cast<uint8_t>(9) + (s_col * static_cast<uint8_t>(3) + j);
                    // ignore the cell we already found
                    if (cell == index)
                    {
                        continue;
                    }
                    for (uint8_t k = 1; k <= static_cast<uint8_t>(9); k++)
                    {
                        if (get_notation(index, k) == true)
                        {
                            count += static_cast<uint8_t>(1);
                            triple2.push_back(k);
                        }
                    }
                    if (count > static_cast<uint8_t>(1) && count < static_cast<uint8_t>(4)
                        && tiple_or_quad(triple1, triple2) == true)
                    {
                        // if a cell with 2/3 or 3/3 notations is found remember it
                        if (cell2 == 100)
                        {
                            cell2 = index;
                        }
                        // if a third is found remove the notations for the 3 numbers from the rest of the square
                        if (cell2 != 100 && cell2 != index)
                        {
                            // iterate through rows
                            for (uint8_t k = 0; k < static_cast<uint8_t>(3); k++)
                            {
                                // iterate within row
                                for (uint8_t l = 0; l < static_cast<uint8_t>(3); l++)
                                {
                                    const uint8_t final_index = (s_row * static_cast<uint8_t>(3) + k) * 
                                        static_cast<uint8_t>(9) + (s_col * static_cast<uint8_t>(3) + l);
                                    if (final_index == cell || final_index == index || final_index == cell2)
                                    {
                                        continue;
                                    }
                                    for (const auto t : triple1)
                                    {
                                        write_notation(final_index, t, 0);
                                    }
                                }
                            }
                            return {true, std::vector{static_cast<uint16_t>(cell), 
                                static_cast<uint16_t>(cell2), static_cast<uint16_t>(index)}};
                        }
                    }
                }
            }
        }
    }
    return {false, std::vector<uint16_t>{}};
}

std::pair<bool, std::vector<uint16_t>> sudoku::solve_by_naked_quads()
{
    return {false, std::vector<uint16_t>{}};
}

bool sudoku::tiple_or_quad(const std::vector<uint8_t>& list, const std::vector<uint8_t>& candidate)
{
    uint8_t count = 0;
    for (const auto i : list)
    {
        for (const auto j : candidate)
        {
            if (i == j)
            {
                count += 1;
                break;
            }
        }
    }
    if (count > 1)
    {
        return true;
    }
    return false;
}

/*

bool sudoku::solve_by_hidden_pairs()
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
