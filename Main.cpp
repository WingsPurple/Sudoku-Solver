#include "Soduko_Solver.h"

uint16_t main()
{
    sudoku board;
    if (board.read("Sudoku boards/Board_4.txt") == false)
    {
        return -1;
    }
    board.print_board();
    while (true)
    {
        board.solve_step();
    }
    
    return 0;
}