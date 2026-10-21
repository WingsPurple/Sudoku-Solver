#include "sudoku_solver.h"

// helper function to compare my solution and the solution
uint8_t test(const char* board_file, const char* solution_file)
{
    sudoku board;
    if (board.read(board_file) == false)
    {
        return -1;
    }
    std::cout << "Starting Board " << "\n";
    board.print_board();
    //board.solve();
    board.solve_with_steps();
    sudoku solution;
    solution.read(solution_file);
    if (solution != board)
    {
        std::cout << "Solution is incorrect \n";
        board.diff(solution);
        board.is_valid();
        board.print_board();
        return 0;
    }
    std::cout << "Solution is correct " << std::endl;
    board.print_board();
    return -1;
}

int main()
{
    //test("Sudoku boards/Board_1_sudoku.txt", "Sudoku boards/Board_1_and_2_solution.txt");
    //test("Sudoku boards/Board_2_single_candidate.txt", "Sudoku boards/Board_1_and_2_solution.txt");
    //test("Sudoku boards/Board_3_naked_pairs.txt", "Sudoku boards/Board_3_solution.txt");
    //test("Sudoku boards/Board_4_hidden_pairs.txt", "Sudoku boards/Board_4_solution.txt");
    test("Sudoku boards/Board_5_naked_triples.txt", "Sudoku boards/Board_5_solution.txt");
    
    /*
    sudoku board;
    char filename[256];
    std::cin >> filename;
    if (board.read(filename) == false)
    {
        return -1;
    }
    std::cout << "Starting Board " << "\n";
    board.print_board();
    //board.solve();
    board.solve_with_steps();
    if (board.is_complete())
    {
        std::cout << "Board is solved " << std::endl;
        board.print_board();
        return 0;
    }
   if (board.is_valid())
    {
        std::cout << "Board is valid but this can't currently solve it " << std::endl;
        board.print_board();
       return 0;
    }
    std::cout << "Board ended in an invalid state oops " << std::endl;
    board.print_board();
    */
    return -1;
}
