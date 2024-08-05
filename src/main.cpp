#include <iostream>
#include <ftxui/component/screen_interactive.hpp>

#include "Board.hpp"
#include "BoardComponentBase.hpp"
#include "Tile.hpp"

int main() {
    using Minesweeper::Board, Minesweeper::BoardComponentBase, Minesweeper::BoardComponent;
    int num;
    std::cin >> num;
    Board board{static_cast<std::uint8_t>(num), 5, 5};
    ftxui::ScreenInteractive screen{ftxui::ScreenInteractive::Fullscreen()};
    const BoardComponent boardComponent{BoardComponentBase::Create(board, screen.ExitLoopClosure())};
    screen.Loop(Hoverable(boardComponent, boardComponent->hovered()) | ftxui::center | ftxui::flex);
    std::cin.peek();
    return 0;
}
