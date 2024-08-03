#include <iostream>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include "Board.hpp"
#include "BoardComponentBase.hpp"
#include "Tile.hpp"
#include "TileComponentBase.hpp"

int main() {
    int num;
    std::cin >> num;
    Minesweeper::Board board{static_cast<std::uint8_t>(num), 5, 5};
    const Minesweeper::BoardComponent boardComponent{Minesweeper::BoardComponentBase::Create(board)};
    ftxui::ScreenInteractive screen{ftxui::ScreenInteractive::Fullscreen()};
    screen.Loop(Hoverable(boardComponent, boardComponent->hovered()) | ftxui::center | ftxui::flex);
    std::cin.peek();
    return 0;
}
