#include "sudoku_solver.h"

bool sudoku::read(const std::string& filename)
{
    // read board from a file
    if (std::ifstream file(filename); file.is_open())
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
    if (board.size() != U8_SC(81))
    {
        std::cout << "Invalid board size" << std::endl;
        // reset board to be empty
        board.resize(0, 0);
        return false;
    }
    return true;
}

void sudoku::write(const uint8_t cell, const uint16_t number, const type t)
{
    board[cell] = number;
    update_notation(cell);
    print_board(cell, t);
}

void sudoku::print_board(const uint8_t highlight, const type t) const
{
    if (t == SUDOKU)
    {
        std::cout << "Solved By Sudoku: \n";
    }
    if (t == SINGLE)
    {
        std::cout << "Solved By Single Candidate: \n";
    }
    for (uint8_t i = 0; i < U8_SC(81); i++)
    {
        // highlight the recently written number as red
        if (i == highlight)
        {
            std::cout << "\033[31m" << (*this)[i] << "\033[0m ";
        }
        else
        {
            std::cout << (*this)[i] << " ";
        }
        if ((i + U8_SC(1)) % U8_SC(9) == U8_SC(0))
        {
            std::cout << '\n';
            if ((i + U8_SC(1)) % U8_SC(27) == 
                U8_SC(0) && i != U8_SC(80))
            {
                std::cout << "----------------------" << '\n';
            }
        }
        else if (i % U8_SC(3) == U8_SC(2))
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
            solve_by_naked_pairs();
            //solve_by_naked_triples();
            solving = solve_by_sudoku();
            solving |= solve_by_single_candidate();
            // if nothing is found check if a second loop of pairs/triples finds anything
            if (solving == false)
            {
                solve_by_naked_pairs();
                //solve_by_naked_triples();
                solving = solve_by_sudoku();
                solving |= solve_by_single_candidate();
            }
        }
    }
}

bool sudoku::solve_step()
{
    if (is_solvable())
    {
        fill_notations_by_sudoku();
        solve_by_naked_pairs();
        solve_by_naked_triples();
        if (solve_by_sudoku())
        {
            return true;
        }
        if (solve_by_single_candidate())
        {
            return true;
        }
    }
    return false;
}

bool sudoku::is_solvable() const
{
    // temporarily set to always true
    // std::cout << "Board is unsolvable" << std::endl;
    return true;
}

void sudoku::write_notation(const uint8_t cell, const uint8_t number, const uint8_t state)
{
    if (state == U8_SC(0))
    {
        board[cell] &= (U16_SC(~(U8_SC(1) << (U8_SC(3) + number))));
    }
    else if (state == U8_SC(1))
    {
        board[cell] |= (U16_SC(U8_SC(1) << (U8_SC(3) + number)));
    }
}

void sudoku::update_notation(const uint8_t cell)
{
    const uint8_t number = U8_SC((*this)[cell]);
    const uint8_t col = cell % U8_SC(9);
    const uint8_t row = cell / U8_SC(9);
    // iterate through the column
    for (uint8_t i = col; i < U8_SC(81); i += U8_SC(9))
    {
        write_notation(i, number, 0);
    }
    // iterate through the row
    for (uint8_t i = 0; i < U8_SC(9); i++)
    {
        write_notation(row * U8_SC(9) + i, number, 0);
    }
    // iterate through the square
    const uint8_t s_col = (cell % U8_SC(9)) / U8_SC(3);
    const uint8_t s_row = (cell / U8_SC(9)) / U8_SC(3);
    for (uint8_t i = 0; i < U8_SC(3); i++)
    {
        for (uint8_t j = 0; j < U8_SC(3); j++)
        {
            const uint8_t index = (s_row * U8_SC(3) + i) *
                U8_SC(9) + (s_col * U8_SC(3) + j);
            write_notation(index, number, 0);
        }
    }
}

bool sudoku::get_notation(const uint8_t cell, const uint8_t number) const
{
    // since the number will always be 1-9 we only have to add 3 to the bit position to shift it correctly
    return board[cell] >> (number + U8_SC(3)) & 1;
}

bool sudoku::solve_by_sudoku()
{
    // iterate through the board
    // if a cell is filled new solutions may have become available so we check again
    for (uint8_t i = 0; i < U8_SC(81); i++)
    {
        // an empty cell is found
        if ((*this)[i] == U8_SC(0))
        {
            bool found = solve_by_column(i % U8_SC(9), i);
            if (found)
            {
                return found;
            }
            found = solve_by_row(i / U8_SC(9), i);
            if (found)
            {
                return found;
            }
            // top left square is 0, 0, top middle square is 1, 0, top right square is 2, 0
            // middle left square is 0, 1, middle square is 1, 1, middle right square is 2, 1
            // bottom left square is 0, 2, bottom middle square is 1, 2, bottom right square is 2, 2
            found = solve_by_square(
                (i % U8_SC(9)) / U8_SC(3), 
                (i / U8_SC(9)) / U8_SC(3), i);
            if (found)
            {
                return found;
            }
        }
    }
    return false;
}

bool sudoku::solve_by_row(const uint8_t row, const uint8_t cell)
{
    std::vector<uint16_t> temp;
    const uint8_t index = row * U8_SC(9);
    // iterate through row
    for (uint8_t i = index; i < index + U8_SC(9); i++)
    {
        // skip empty cells
        if ((*this)[i] == U8_SC(0))
        {
            continue;
        }
        // find every unique number in that row
        if (std::find(temp.begin(), temp.end() , (*this)[i]) == temp.end())
        {
            temp.push_back((*this)[i]);
        }
        // if there are 8 unique numbers, the empty cell is the ninth
        if (temp.size() == U8_SC(8))
        {
            for (uint8_t number = 1; number <= U8_SC(9); number++)
            {
                if (std::find(temp.begin(), temp.end(), number) == temp.end())
                {
                    write(cell, number, SUDOKU);
                    return true;
                }
            }
        }
    }
    return false;
}

bool sudoku::solve_by_column(const uint8_t col, const uint8_t cell)
{
    std::vector<uint16_t> temp;
    // iterate through column
    for (uint8_t i = col; i < U8_SC(81); i += U8_SC(9))
    {
        // skip empty cells
        if ((*this)[i] == U8_SC(0))
        {
            continue;
        }
        // find every unique number in that column
        if (std::find(temp.begin(), temp.end() , (*this)[i]) == temp.end())
        {
            temp.push_back((*this)[i]);
        }
        // if there are 8 unique numbers, the empty cell is the ninth
        if (temp.size() == U8_SC(8))
        {
            for (uint8_t number = 1; number <= U8_SC(9); number++)
            {
                if (std::find(temp.begin(), temp.end(), number) == temp.end())
                {
                    write(cell, number, SUDOKU);
                    return true;
                }
            }
        }
    }
    return false;
}

bool sudoku::solve_by_square(const uint8_t col, const uint8_t row, const uint8_t cell)
{
    std::vector<uint16_t> temp;
    // iterate through rows
    for (uint8_t i = 0; i < U8_SC(3); i++)
    {
        // iterate within row
        for (uint8_t j = 0; j < U8_SC(3); j++)
        {
            // index is the cell in the square we are checking
            const uint8_t index = 
                (row * U8_SC(3) + i) * U8_SC(9) + (col * U8_SC(3) + j);
            // skip empty cells
            if ((*this)[index] == U8_SC(0))
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
    if (temp.size() == U8_SC(8))
    {
        for (uint8_t number = 1; number <= U8_SC(9); number++)
        {
            if (std::find(temp.begin(), temp.end() , number) == temp.end())
            {
                write(cell, number, SUDOKU);
                return true;
            }
        }
    }
    return false;
}


void sudoku::fill_notations_by_sudoku()
{
    std::vector possibilities(U8_SC(81), std::vector<uint8_t>(9, 1));
    // iterate through all columns
    for (uint8_t col = 0; col < U8_SC(9); col++)
    {
        for (uint8_t i = col; i < U8_SC(81); i += U8_SC(9))
        {
            // if we encounter a number in the column remove it from the list of possible numbers
            if ((*this)[i] != U8_SC(0))
            {
                for (uint8_t j = col; j < U8_SC(81); j += U8_SC(9))
                {
                    possibilities[j][(*this)[i] - 1] = U8_SC(0);
                }
            }
        }
    }
    // iterate through all rows
    for (uint8_t row = 0; row < U8_SC(9); row++)
    {
        for (uint8_t i = 0; i < U8_SC(9); i++)
        {
            // if we encounter a number in the row remove it from the list of possible numbers
            if ((*this)[row * U8_SC(9) + i] != U8_SC(0))
            {
                for (uint8_t j = 0; j < U8_SC(9); j++)
                {
                    possibilities[row * U8_SC(9) + j]
                    [(*this)[row * U8_SC(9) + i] - 1] = U8_SC(0);
                }
            }
        }
    }
    // iterate through all squares
    for (uint8_t cell = 0; cell < U8_SC(81); cell++)
    {
        // calculate the current row and column of the square
        // top left square is 0, 0, top middle square is 1, 0, top right square is 2, 0
        // middle left square is 0, 1, middle square is 1, 1, middle right square is 2, 1
        // bottom left square is 0, 2, bottom middle square is 1, 2, bottom right square is 2, 2
        const uint16_t s_col = (cell % U8_SC(9)) / U8_SC(3);
        const uint16_t s_row = (cell / U8_SC(9)) / U8_SC(3);
        // iterate through rows of the square
        for (uint8_t i = 0; i < U8_SC(3); i++)
        {
            // iterate within row
            for (uint8_t j = 0; j < U8_SC(3); j++)
            {
                // index is the cell in the square we are checking
                const uint16_t index = (s_row * U8_SC(3) + i) * U8_SC(9) + 
                    (s_col * U8_SC(3) + j);
                // if we encounter a number in the square remove it from the list of possible numbers
                if ((*this)[index] != U8_SC(0))
                {
                    // iterate through rows of the square
                    for (uint8_t k = 0; k < U8_SC(3); k++)
                    {
                        // iterate within row
                        for (uint8_t l = 0; l < U8_SC(3); l++)
                        {
                            const uint16_t write_index = (s_row * U8_SC(3) + k) * 
                                U8_SC(9) + (s_col * U8_SC(3) + l);
                            possibilities[write_index][(*this)[index] - 1] = U8_SC(0);
                        }
                    }
                }
            }
        }
        // write the notations for that cell
        for (uint8_t i = 0; i < U8_SC(9); i++)
        {
            if (possibilities[cell][i] != U8_SC(0) && (*this)[cell] == U8_SC(0))
            {
                write_notation(cell, i + 1, possibilities[cell][i]);
            }
        }
    }
}

void sudoku::clear_notations()
{
    for (int i = U8_SC(0); i < U8_SC(81); i++)
    {
        if ((board[i] & MASK) == U8_SC(0))
        {
            board[i] = U8_SC(0);
        }
    }
}

bool sudoku::solve_by_single_candidate()
{
    // iterate through all numbers
    for (uint8_t number = 1; number <= U8_SC(9); number++)
    {
        // check all columns
        for (uint8_t col = 0; col < U8_SC(9); col++)
        {
            uint8_t counter = 0;
            // using 100 as the out of bounds/unset value since I can't do -1
            // this is redundant as the base value never gets read but better be safe 
            uint8_t cell = 100;
            for (uint8_t i = col; i < U8_SC(81); i += U8_SC(9))
            {
                // if the number is already in the column skip to the next one
                if ((*this)[i] == number)
                {
                    break;
                }
                if (get_notation(i, number))
                {
                    counter++;
                    cell = i;
                }
            }
            // if the number is only found once as a notation then write it to the cell
            if (counter == U8_SC(1))
            {
                write(cell, number, SINGLE);
                return true;
            }
        }
        // check all rows
        for (uint8_t row = 0; row < U8_SC(9); row++)
        {
            uint8_t counter = 0;
            uint8_t found = 0;
            for (uint8_t i = 0; i < U8_SC(9); i++)
            {
                const uint8_t cell = row * U8_SC(9) + i;
                // if the number is already in the row skip to the next one
                if ((*this)[cell] == number)
                {
                    break;
                }
                if (get_notation(cell, number))
                {
                    counter++;
                    found = cell;
                }
            }
            // if the number is only found once as a notation then write it to the cell
            if (counter == U8_SC(1))
            {
                write(found, number, SINGLE);
                return true;
            }
        }
        // check all squares
        // iterate through all squares
        for (uint8_t cell = 0; cell < U8_SC(81); cell += U8_SC(7))
        {
            // calculate the current row and column
            const uint8_t col = (cell % U8_SC(9)) / U8_SC(3);
            const uint8_t row = (cell / U8_SC(9)) / U8_SC(3);
            // iterate through rows of the square
            uint8_t counter = 0;
            uint8_t found = 0;
            bool skip = false;
            for (uint8_t i = 0; i < U8_SC(3); i++)
            {
                if (skip)
                {
                    break;
                }
                // iterate within row
                for (uint8_t j = 0; j < U8_SC(3); j++)
                {
                    // index is the cell in the square we are checking
                    const uint8_t index = (row * U8_SC(3) + i) *
                        U8_SC(9) + (col * U8_SC(3) + j);
                    // if the number already exists in the square, skip this square
                    if ((*this)[index] == number)
                    {
                        skip = true;
                        break;
                    }
                    if (get_notation(index, number))
                    {
                        counter++;
                        found = index;
                    }
                }
            }
            // if the number is only found once as a notation then write it to the cell
            if (counter == U8_SC(1))
            {
                write(found, number, SINGLE);
                return true;
            }
        }
    }
    return false;
}

void sudoku::solve_by_naked_pairs()
{
    // iterate through whole board
    for (uint8_t cell = 0; cell < U8_SC(81); cell++)
    {
        // make a list of all notations in the current cell
        uint8_t count = 0;
        std::vector<uint8_t> pair1{};
        for (uint8_t i = 1; i <= U8_SC(9); i++)
        {
            if (get_notation(cell, i) == true)
            {
                count++;
                pair1.push_back(i);
            }
        }
        // if a cell is found with exactly 2 notations try to find another with the same exact 2 notations
        if (count == U8_SC(2))
        {
            bool found = false;
            // using 100 as the out of bounds/unset value since I can't do -1
            uint8_t found1 = 100;
            uint8_t found2 = 100;
            const uint8_t row = cell / U8_SC(9);
            const uint8_t col = cell % U8_SC(9);
            // calculate the current row and column of the square
            // top left square is 0, 0, top middle square is 1, 0, top right square is 2, 0
            // middle left square is 0, 1, middle square is 1, 1, middle right square is 2, 1
            // bottom left square is 0, 2, bottom middle square is 1, 2, bottom right square is 2, 2
            const uint8_t s_row = (cell / U8_SC(9)) / U8_SC(3);
            const uint8_t s_col = (cell % U8_SC(9)) / U8_SC(3);
            // check current column
            for (uint8_t i = col; i < U8_SC(81); i += U8_SC(9))
            {
                // ignore the cell we already found
                if (i == cell)
                {
                    continue;
                }
                // make a list of all notations in the current cell
                count = U8_SC(0);
                std::vector<uint8_t> pair2{};
                for (uint8_t j = 1; j <= U8_SC(9); j++)
                {
                    if (get_notation(i, j) == true)
                    {
                        count++;
                        pair2.push_back(j);
                    }
                }
                // if an identical pair is found remove the found notations from the rest of the column
                if (count == U8_SC(2) && pair1 == pair2)
                {
                    for (uint8_t k = col; k < U8_SC(81); k += U8_SC(9))
                    {
                        if (k == cell || k == i)
                        {
                            continue;
                        }
                        write_notation(k, pair1.front(), 0);
                        write_notation(k, pair1.back(), 0);
                    }
                    found1 = i;
                    found = true;
                    std::cout << "Found a naked pair col: " << 
                        U16_SC(cell) << " " << U16_SC(i)  << '\n';
                }
            }
            // we can skip the row check if the pair is found in a column
            if (found == false)
            {
                // check current row
                for (uint8_t i = 0; i < U8_SC(9); i++)
                {
                    // make a list of all notations in the current cell
                    count = U8_SC(0);
                    std::vector<uint8_t> pair2{};
                    const uint8_t index = row * U8_SC(9) + i;
                    // ignore the cell we already found
                    if (index == cell)
                    {
                        continue;
                    }
                    for (uint8_t j = 1; j <= U8_SC(9); j++)
                    {
                        if (get_notation(index, j) == true)
                        {
                            count += 1;
                            pair2.push_back(j);
                        }
                    }
                    // if an identical pair is found remove the found notations from the rest of the row
                    if (count == U8_SC(2) && pair1 == pair2)
                    {
                        for (uint8_t k = 0; k < U8_SC(9); k++)
                        {
                            const uint8_t temp = row * U8_SC(9) + k;
                            if (temp == cell || temp == index)
                            {
                                continue;
                            }
                            write_notation(temp, pair1.front(), 0);
                            write_notation(temp, pair1.back(), 0);
                        }
                        found2 = index;
                        std::cout << "Found a naked pair row: " << 
                            U16_SC(cell) << " " << U16_SC(index)  << '\n';
                    }
                }
            }
            // check current square
            for (uint8_t i = 0; i < U8_SC(3); i++)
            {
                // iterate within row
                for (uint8_t j = 0; j < U8_SC(3); j++)
                {
                    // make a list of all notations in the current cell
                    count = U8_SC(0);
                    std::vector<uint8_t> pair2{};
                    // index is the cell in the square we are checking
                    const uint8_t index = (s_row * U8_SC(3) + i) *
                        U8_SC(9) + (s_col * U8_SC(3) + j);
                    // ignore the cells we already found
                    if (cell == index || index == found1 || index == found2)
                    {
                        continue;
                    }
                    for (uint8_t k = 1; k <= U8_SC(9); k++)
                    {
                        if (get_notation(index, k) == true)
                        {
                            count++;
                            pair2.push_back(k);
                        }
                    }
                    // if an identical pair is found remove the found notations from the rest of the square
                    if (count == U8_SC(2) && pair1 == pair2)
                    {
                        // iterate through rows
                        for (uint8_t k = 0; k < U8_SC(3); k++)
                        {
                            // iterate within row
                            for (uint8_t l = 0; l < U8_SC(3); l++)
                            {
                                const uint8_t final_index = (s_row * U8_SC(3) + k) * 
                                    U8_SC(9) + (s_col * U8_SC(3) + l);
                                if (final_index == cell || final_index == index)
                                {
                                    continue;
                                }
                                write_notation(final_index, pair1.front(), 0);
                                write_notation(final_index, pair1.back(), 0);
                            }
                        }
                        std::cout << "Found a naked pair sq: " << 
                            U16_SC(cell) << " " << U16_SC(index)  << '\n';
                    }
                }
            }
        }
    }
}

// solve_by_naked_pairs, solve_by_naked_triples are stumping me with what's wrong
// I am considering turning them into separate functions for col row square 
void sudoku::solve_by_naked_triples()
{
    // iterate through whole board
    for (uint8_t cell = 0; cell < U8_SC(81); cell++)
    {
        // make a list of all notations in the current cell
        uint8_t count = 0;
        std::vector<uint8_t> triple1{};
        for (uint8_t i = 1; i <= U8_SC(9); i++)
        {
            if (get_notation(cell, i) == true)
            {
                count++;
                triple1.push_back(i);
            }
        }
        // if a cell is found with exactly 3 notations try to find 2 more with at least 2 of the same 3 notations
        if (count == U8_SC(3))
        {
            bool found = false;
            // using 100 as the out of bounds/unset value since I can't do -1
            uint8_t cell2 = 100;
            const uint8_t row = cell / U8_SC(9);
            const uint8_t col = cell % U8_SC(9);
            // calculate the current row and column of the square
            // top left square is 0, 0, top middle square is 1, 0, top right square is 2, 0
            // middle left square is 0, 1, middle square is 1, 1, middle right square is 2, 1
            // bottom left square is 0, 2, bottom middle square is 1, 2, bottom right square is 2, 2
            const uint8_t s_row = (cell / U8_SC(9)) / U8_SC(3);
            const uint8_t s_col = (cell % U8_SC(9)) / U8_SC(3);
            // check current column
            for (uint8_t i = col; i < U8_SC(81); i += U8_SC(9))
            {
                // ignore the cells we already found
                if (i == cell || i == cell2)
                {
                    continue;
                }
                // make a list of all notations in the current cell
                count = U8_SC(0);
                std::vector<uint8_t> triple2{};
                for (uint8_t j = 1; j <= U8_SC(9); j++)
                {
                    if (get_notation(i, j) == true)
                    {
                        count++;
                        triple2.push_back(j);
                    }
                }
                if (count > U8_SC(1) && count < U8_SC(4)
                    && tiple_or_quad(triple1, triple2) == true)
                {
                    // if a cell with 2/3 or 3/3 notations is found remember it
                    if (cell2 == U8_SC(100))
                    {
                        cell2 = i;
                    }
                    // if a third is found remove the notations for the 3 numbers from the rest of the column
                    if (cell2 != U8_SC(100) && cell2 != i)
                    {
                        found = true;
                        for (uint8_t k = col; k < U8_SC(81); k += U8_SC(9))
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
                        std::cout << "Found a naked triple at " << U16_SC(cell) << " " 
                            << U16_SC(cell2) << " " << U16_SC(i) << '\n';
                    }
                }
            }
            // we can skip the row check if the pair is found in a column
            if (found == false)
            {
                // check current row
                for (uint8_t i = 0; i < U8_SC(9); i++)
                {
                    // make a list of all notations in the current cell
                    count = U8_SC(0);
                    std::vector<uint8_t> triple2{};
                    const uint8_t index = row * U8_SC(9) + i;
                    // ignore the cells we already found
                    if (index == cell || index == cell2)
                    {
                        continue;
                    }
                    for (uint8_t j = 1; j <= U8_SC(9); j++)
                    {
                        if (get_notation(index, j) == true)
                        {
                            count++;
                            triple2.push_back(j);
                        }
                    }
                    if (count > U8_SC(1) && count < U8_SC(4)
                        && tiple_or_quad(triple1, triple2) == true)
                    {
                        // if a cell with 2/3 or 3/3 notations is found remember it
                        if (cell2 == U8_SC(100))
                        {
                            cell2 = index;
                        }
                        // if a third is found remove the notations for the 3 numbers from the rest of the row
                        if (cell2 != U8_SC(100) && cell2 != index)
                        {
                            for (uint8_t k = 0; k < U8_SC(9); k++)
                            {
                                const uint8_t temp = row * U8_SC(9) + k;
                                if (temp == cell || temp == index || temp == cell2)
                                {
                                    continue;
                                }
                                for (const auto t : triple1)
                                {
                                    write_notation(temp, t, 0);
                                }
                            }
                            std::cout << "Found a naked triple at " << U16_SC(cell) << " " 
                                << U16_SC(cell2) << " " << U16_SC(index) << '\n';
                        }
                    }
                }
            }
            // check current square
            for (uint8_t i = 0; i < U8_SC(3); i++)
            {
                // iterate within row
                for (uint8_t j = 0; j < U8_SC(3); j++)
                {
                    // make a list of all notations in the current cell
                    count = U8_SC(0);
                    std::vector<uint8_t> triple2{};
                    // index is the cell in the square we are checking
                    const uint8_t index = (s_row * U8_SC(3) + i) *
                        U8_SC(9) + (s_col * U8_SC(3) + j);
                    // ignore the cells we already found
                    if (cell == index || cell2 == index)
                    {
                        continue;
                    }
                    for (uint8_t k = 1; k <= U8_SC(9); k++)
                    {
                        if (get_notation(index, k) == true)
                        {
                            count++;
                            triple2.push_back(k);
                        }
                    }
                    if (count > U8_SC(1) && count < U8_SC(4)
                        && tiple_or_quad(triple1, triple2) == true)
                    {
                        // if a cell with 2/3 or 3/3 notations is found remember it
                        if (cell2 == U8_SC(100))
                        {
                            cell2 = index;
                        }
                        // if a third is found remove the notations for the 3 numbers from the rest of the square
                        if (cell2 != U8_SC(100) && cell2 != index)
                        {
                            // iterate through rows
                            for (uint8_t k = 0; k < U8_SC(3); k++)
                            {
                                // iterate within row
                                for (uint8_t l = 0; l < U8_SC(3); l++)
                                {
                                    const uint8_t final_index = (s_row * U8_SC(3) + k) * 
                                        U8_SC(9) + (s_col * U8_SC(3) + l);
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
                            std::cout << "Found a naked triple at " << U16_SC(cell) << " " 
                                << U16_SC(cell2) << " " << U16_SC(index) << '\n';
                        }
                    }
                }
            }
        }
    }
}

/*
std::pair<bool, std::vector<uint16_t>> sudoku::solve_by_naked_quads()
{
    return {false, std::vector<uint16_t>{}};
}
*/

bool sudoku::tiple_or_quad(const std::vector<uint8_t>& list, const std::vector<uint8_t>& candidate)
{
    uint8_t count = 0;
    for (const auto i : list)
    {
        for (const auto j : candidate)
        {
            if (i == j)
            {
                count++;
                break;
            }
        }
    }
    if (count > U8_SC(1))
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
