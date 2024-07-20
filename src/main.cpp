#include <iostream>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

#include "Board.hpp"
#include "BoardComponent.hpp"
#include "Tile.hpp"
#include "TileComponent.hpp"

int main() {
    Minesweeper::Board board{5, 5, 5};
    board.checkTile(0, 0);
    auto document = Minesweeper::BoardComponent(board).Render();
    auto screen = ftxui::Screen::Create(ftxui::Dimension::Fit(document));
    Render(screen, document);
    screen.Print();
    std::cout << std::endl;
    auto document2 = Minesweeper::BoardComponent(board).Render();
    Render(screen, document2);
    screen.Print();
    return 0;
}
