#include <iostream>
#include <ftxui/component/screen_interactive.hpp>

#include "Board.hpp"
#include "BoardComponentBase.hpp"

int main() {
    using Minesweeper::Board, Minesweeper::BoardComponentBase, Minesweeper::BoardComponent;
    ftxui::ScreenInteractive screen{ftxui::ScreenInteractive::Fullscreen()};
    int selected = 0;
    const std::vector<std::string> entries{"Beginner", "Intermediate", "Expert", "Custom"};
    ftxui::MenuOption option;
    option.on_enter = screen.ExitLoopClosure();
    ftxui::Component menu = Menu(&entries, &selected, option);
    screen.Loop(Renderer(menu, [menu] {
        return ftxui::vbox({ftxui::text("Choose your difficulty"), ftxui::separator(), menu->Render()}) | ftxui::border
               | ftxui::center;
    }));
    std::shared_ptr<Board> board;
    switch (selected) {
        case 0:
            board = std::make_shared<Board>(9, 9, 10);
            break;
        case 1:
            board = std::make_shared<Board>(16, 16, 40);
            break;
        case 2:
            board = std::make_shared<Board>(16, 30, 99);
            break;
        case 3:
            // TODO create custom board entry
            break;
        default:
            return EXIT_FAILURE;
    }
    const BoardComponent boardComponent{BoardComponentBase::Create(board, screen.ExitLoopClosure())};
    screen.Loop(Hoverable(boardComponent, boardComponent->hovered()) | ftxui::center);
    std::cin.peek();
    return 0;
}
