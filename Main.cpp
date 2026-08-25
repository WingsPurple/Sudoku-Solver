#include "Soduko_Solver.h"

uint16_t main()
{
    sudoku board;
    if (board.read("Sudoku boards/Board_2.txt") == false)
    {
        return -1;
    }
    board.print_board();
    while (true)
    {
        board.solve();
    }
    
    return 0;
}