#include <iostream>

#include "Board.hpp"

int main()
{
    Minesweeper::Board board {5,5,5};
    board.checkTile(0, 0);
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
