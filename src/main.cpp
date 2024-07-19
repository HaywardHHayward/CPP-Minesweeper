#include <iostream>

#include "Board.hpp"
#include "Tile.hpp"

int main() {
    Minesweeper::Board board{5, 5, 5};
    board.checkTile(0, 0);
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            std::string tileRepr;
            Minesweeper::Tile& tile = board.at(i, j);
            if (tile.isMine()) {
                tileRepr = "M";
            } else {
                tileRepr = std::to_string(tile.getSurroundingMines());
            }
            std::cout << tileRepr << " ";
        }
        std::cout << std::endl;
    }
    return 0;
}
