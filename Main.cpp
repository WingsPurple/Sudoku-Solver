#include "Soduko_Solver.h"

uint8_t test1_sudoku();
uint8_t test2_single_candidate();
uint8_t test3_naked_pairs();
uint8_t test4_hidden_pairs();
uint8_t test5_naked_triples();

uint8_t main()
{
    //test1_sudoku();
    //test2_single_candidate();
    test3_naked_pairs();
    //test4_hidden_pairs();
    //test5_naked_triples();
    return 0;
}

uint8_t test1_sudoku()
{
    sudoku board;
    if (board.read("Sudoku boards/Board_1_sudoku.txt") == false)
    {
        return -1;
    }
    std::cout << "Starting Board " << "\n";
    board.print_board();
    while (board.solve_step())
    {
        board.solve_step();
    }
    sudoku solution;
    solution.read("Sudoku boards/Board_1_and_2_solution.txt");
    if (solution.board == board.board)
    {
        std::cout << "Solution is correct " << std::endl;
        return 0;
    }
    std::cout << "Solution is incorrect " << std::endl;
    return -1;
}

uint8_t test2_single_candidate()
{
    sudoku board;
    if (board.read("Sudoku boards/Board_2_single_candidate.txt") == false)
    {
        return -1;
    }
    std::cout << "Starting Board " << "\n";
    board.print_board();
    while (board.solve_step())
    {
        board.solve_step();
    }
    sudoku solution;
    solution.read("Sudoku boards/Board_1_and_2_solution.txt");
    if (solution.board == board.board)
    {
        std::cout << "Solution is correct " << std::endl;
        return 0;
    }
    std::cout << "Solution is incorrect " << std::endl;
    return -1;
}

uint8_t test3_naked_pairs()
{
    sudoku board;
    if (board.read("Sudoku boards/Board_3_solution.txt") == false)
    {
        return -1;
    }
    std::cout << "Starting Board " << "\n";
    board.print_board();
    while (board.solve_step())
    {
        board.solve_step();
    }
    sudoku solution;
    solution.read("Sudoku boards/Board_3_solution.txt");
    if (solution.board == board.board)
    {
        std::cout << "Solution is correct " << std::endl;
        return 0;
    }
    std::cout << "Solution is incorrect " << std::endl;
    return -1;
}

uint8_t test4_hidden_pairs()
{
    sudoku board;
    if (board.read("Sudoku boards/Board_4_hidden_pairs.txt") == false)
    {
        return -1;
    }
    std::cout << "Starting Board " << "\n";
    board.print_board();
    while (board.solve_step())
    {
        board.solve_step();
    }
    sudoku solution;
    solution.read("Sudoku boards/Board_4_solution.txt");
    if (solution.board == board.board)
    {
        std::cout << "Solution is correct " << std::endl;
        return 0;
    }
    std::cout << "Solution is incorrect " << std::endl;
    return -1;
}

uint8_t test5_naked_triples()
{
    sudoku board;
    if (board.read("Sudoku boards/Board_5_naked_triples.txt") == false)
    {
        return -1;
    }
    std::cout << "Starting Board " << "\n";
    board.print_board();
    while (board.solve_step())
    {
        board.solve_step();
    }
    sudoku solution;
    solution.read("Sudoku boards/Board_5_solution.txt");
    if (solution.board == board.board)
    {
        std::cout << "Solution is correct " << std::endl;
        return 0;
    }
    std::cout << "Solution is incorrect " << std::endl;
    return -1;
}