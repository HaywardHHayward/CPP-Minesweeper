#include <iostream>
#include <ftxui/component/screen_interactive.hpp>

#include "Board.hpp"
#include "BoardComponentBase.hpp"

#if defined(_MSC_VER)
#define UNREACHABLE() __assume(0)
#elif defined(__GNUC__)
#define UNREACHABLE() __builtin_unreachable()
#else
#define UNREACHABLE() 
#endif

enum Difficulty {
    beginner = 0,
    intermediate = 1,
    expert = 2,
    custom = 3
};

int main() {
    using Minesweeper::Board, Minesweeper::BoardComponentBase, Minesweeper::BoardComponent;
    ftxui::ScreenInteractive screen{ftxui::ScreenInteractive::Fullscreen()};
    int selected = 0;
    const std::vector<std::string> entries{"Beginner", "Intermediate", "Expert", "Custom"};
    const ftxui::MenuOption menuOption{
        .on_enter = screen.ExitLoopClosure()
    };
    const ftxui::Component menu = Menu(&entries, &selected, menuOption);
    screen.Loop(ftxui::Renderer(menu, [&] {
        return ftxui::vbox({ftxui::text("Choose your difficulty"), ftxui::separator(), menu->Render()}) | ftxui::border
               | ftxui::center;
    }));
    std::shared_ptr<Board> board;
    switch (selected) {
        case beginner:
            board = std::make_shared<Board>(9, 9, 10);
            break;
        case intermediate:
            board = std::make_shared<Board>(16, 16, 40);
            break;
        case expert:
            board = std::make_shared<Board>(16, 30, 99);
            break;
        case custom: {
            std::string rowStr, columnStr, mineStr;
            std::uint8_t row, column;
            std::uint16_t mines;
            const ftxui::InputOption rowOption{
                .placeholder = "Enter number of rows",
                .multiline = false,
            };
            const ftxui::InputOption columnOption{
                .placeholder = "Enter number of columns",
                .multiline = false,
            };
            const ftxui::InputOption mineOption{
                .placeholder = "Enter number of mines",
                .multiline = false,
            };
            const auto inputs{ftxui::Container::Vertical({
                Input(&rowStr, rowOption) | ftxui::CatchEvent([&](const ftxui::Event& event) {
                    return event.is_character() && !std::isdigit(event.character()[0]);
                }) | ftxui::CatchEvent([&](const ftxui::Event& event) {
                    return event.is_character() && rowStr.size() > std::numeric_limits<std::uint8_t>::digits10;
                }),
                Input(&columnStr, columnOption) | ftxui::CatchEvent([&](const ftxui::Event& event) {
                    return event.is_character() && !std::isdigit(event.character()[0]);
                }) | ftxui::CatchEvent([&](const ftxui::Event& event) {
                    return event.is_character() && columnStr.size() > std::numeric_limits<std::uint8_t>::digits10;
                }),
                Input(&mineStr, mineOption) | ftxui::CatchEvent([&](const ftxui::Event& event) {
                    return event.is_character() && !std::isdigit(event.character()[0]);
                }) | ftxui::CatchEvent([&](const ftxui::Event& event) {
                    return event.is_character() && mineStr.size() > std::numeric_limits<std::uint16_t>::digits10;
                })
            })};
            const ftxui::ButtonOption buttonOption{
                .label = "Enter row, column, and mine amounts.",
                .on_click = [&] {
                    if (rowStr.empty() || columnStr.empty() || mineStr.empty()) {
                        return;
                    }
                    const unsigned long rowRaw{std::stoul(rowStr)}, columnRaw{std::stoul(columnStr)}, minesRaw{
                        std::stoul(mineStr)};
                    if (rowRaw == 0 || rowRaw > UINT8_MAX ||
                        columnRaw == 0 || columnRaw > UINT8_MAX ||
                        minesRaw == 0 || minesRaw > UINT16_MAX) {
                        if (rowRaw == 0 || rowRaw > UINT8_MAX) {
                            rowStr.clear();
                        }
                        if (columnRaw == 0 || columnRaw > UINT8_MAX) {
                            columnStr.clear();
                        }
                        if (minesRaw == 0 || minesRaw > UINT16_MAX) {
                            mineStr.clear();
                        }
                        return;
                    }
                    if (minesRaw >= rowRaw * columnRaw) {
                        mineStr.clear();
                        return;
                    }
                    row = static_cast<std::uint8_t>(rowRaw);
                    column = static_cast<std::uint8_t>(columnRaw);
                    mines = static_cast<std::uint16_t>(minesRaw);
                    screen.Exit();
                }};
            auto button = Button(buttonOption);
            auto customMenu = ftxui::Container::Vertical({inputs, button});
            screen.Loop(customMenu | ftxui::center);
            board = std::make_shared<Board>(row, column, mines);
        }
        break;
        default:
            UNREACHABLE();
    }
    const BoardComponent baseBoard{BoardComponentBase::Create(board, screen.ExitLoopClosure())};
    ftxui::Component boardComponent{Hoverable(baseBoard, baseBoard->hovered())};
    ftxui::Component gameplayRender{Renderer(boardComponent, [&] {
        return ftxui::hbox({boardComponent->Render(), ftxui::window(ftxui::text("Testing"), ftxui::text("Bazinga"))}) |
               ftxui::center;
    })};
    screen.Loop(gameplayRender);
    std::cin.peek();
    return 0;
}
