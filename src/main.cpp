#include <chrono>
#include <thread>
#include <ftxui/component/screen_interactive.hpp>

#include "Board.hpp"
#include "BoardComponentBase.hpp"

#ifndef UINT8_MAX
#define UINT8_MAX 0xff
#endif
#ifndef UINT16_MAX
#define UINT16_MAX 0xffff
#endif

int main() {
    using Minesweeper::Board, Minesweeper::BoardComponentBase, Minesweeper::BoardComponent;
    try {
        while (true) {
            namespace tui = ftxui;
            tui::ScreenInteractive screen{tui::ScreenInteractive::Fullscreen()};
            int difficultySelection{0};
            const std::vector<std::string> difficultyEntries{"Beginner", "Intermediate", "Expert", "Custom"};
            const tui::Component menu = tui::Menu(&difficultyEntries, std::bit_cast<int*>(&difficultySelection),
                                                  {.on_enter{screen.ExitLoopClosure()}});
            screen.Loop(Renderer(menu, [&] {
                return tui::vbox({
                           tui::text("Choose your difficulty"),
                           tui::separator(),
                           menu->Render()
                       }) | tui::border
                       | tui::center;
            }));
            std::shared_ptr<Board> board{nullptr};
            switch (difficultySelection) {
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
                    const tui::InputOption rowOption{
                        .placeholder{"Enter number of rows"},
                        .multiline{false},
                    };
                    const tui::InputOption columnOption{
                        .placeholder{"Enter number of columns"},
                        .multiline{false},
                    };
                    const tui::InputOption mineOption{
                        .placeholder{"Enter number of mines"},
                        .multiline{false},
                    };
                    const tui::Component inputs{
                        tui::Container::Vertical({
                            Input(&rowStr, rowOption) | tui::CatchEvent([&](const tui::Event& event) {
                                return event.is_character() && !std::isdigit(event.character()[0]);
                            }) | tui::CatchEvent([&](const tui::Event& event) {
                                return event.is_character() && rowStr.size() > std::numeric_limits<
                                           std::uint8_t>::digits10;
                            }),
                            Input(&columnStr, columnOption) | tui::CatchEvent([&](const tui::Event& event) {
                                return event.is_character() && !std::isdigit(event.character()[0]);
                            }) | tui::CatchEvent([&](const tui::Event& event) {
                                return event.is_character() && columnStr.size() > std::numeric_limits<
                                           std::uint8_t>::digits10;
                            }),
                            Input(&mineStr, mineOption) | tui::CatchEvent([&](const tui::Event& event) {
                                return event.is_character() && !std::isdigit(event.character()[0]);
                            }) | tui::CatchEvent([&](const tui::Event& event) {
                                return event.is_character() && mineStr.size() > std::numeric_limits<
                                           std::uint16_t>::digits10;
                            })
                        })
                    };
                    tui::Component button = tui::Button({
                        .label{"Enter row, column, and mine amounts."},
                        .on_click = [&] {
                            if (rowStr.empty() || columnStr.empty() || mineStr.empty()) {
                                return;
                            }
                            const unsigned long rowRaw{std::stoul(rowStr)}, columnRaw{std::stoul(columnStr)},
                                minesRaw{std::stoul(mineStr)};
                            if (rowRaw == 0 || rowRaw > UINT8_MAX
                                || columnRaw == 0 || columnRaw > UINT8_MAX
                                || minesRaw == 0 || minesRaw > UINT16_MAX) {
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
                        }
                    });
                    const tui::Component customMenu{tui::Container::Vertical({inputs, button})};
                    screen.Loop(customMenu | tui::border | tui::center);
                    board = std::make_shared<Board>(row, column, mines);
                }
                break;
            }
            const BoardComponent baseBoard{BoardComponentBase::Create(board, screen.ExitLoopClosure())};
            const tui::Component boardComponent{Hoverable(baseBoard, baseBoard->hovered())};
            const std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
            const tui::Component gameplayRender{
                Renderer(boardComponent, [&] {
                    const std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
                    const std::chrono::duration elapsedTime = currentTime - startTime;
                    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsedTime).count() % 60;
                    const auto minutes = std::chrono::duration_cast<std::chrono::minutes>(elapsedTime).count();
                    return tui::vbox({
                               tui::hbox({
                                   tui::text(std::format("Remaining mines: {}", board->getRemainingMines())),
                                   tui::filler(),
                                   tui::separator(),
                                   tui::filler(),
                                   tui::text(std::format("Time: {:02}:{:02}", minutes <= 99 ? minutes : 99,
                                                         minutes <= 99 ? seconds : 59))
                               }) | tui::flex,
                               tui::separator(),
                               boardComponent->Render() | tui::border | tui::hcenter
                           }) | tui::border | tui::center;
                })
            };
            std::chrono::steady_clock::time_point nextTime{startTime + std::chrono::seconds(1)};
            std::atomic_bool stop{false};
            std::thread timerRefreshThread([&] {
                while (!stop.load()) {
                    std::this_thread::sleep_until(nextTime);
                    if (stop.load()) {
                        return;
                    }
                    screen.PostEvent(tui::Event::Custom);
                    nextTime += std::chrono::seconds(1);
                }
            });
            screen.Loop(gameplayRender);
            stop.store(true);
            std::chrono::steady_clock::time_point endScreenTime = std::chrono::steady_clock::now();
            const int seconds = std::min(
                static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(endScreenTime - startTime).count() %
                                 60),
                59);
            const int minutes = std::min(
                static_cast<int>(std::chrono::duration_cast<std::chrono::minutes>(endScreenTime - startTime).count()),
                99);
            const std::vector<std::string> endEntries{"Retry", "Exit"};
            int endScreenSelection;
            const tui::Component endMenu{
                tui::Menu(&endEntries, &endScreenSelection, {.on_enter = screen.ExitLoopClosure()})
            };
            const tui::Element endMessage{
                board->hitMine()
                    ? tui::text("You hit a mine! You lose!")
                    : tui::text("You flagged all the mines! You win!")
            };
            const tui::Component endScreenRender{
                Renderer(endMenu, [&] {
                    return tui::vbox({
                               tui::vbox({
                                   tui::hbox({
                                       tui::text(std::format("Remaining mines: {}", board->getRemainingMines())),
                                       tui::filler(),
                                       tui::separator(),
                                       tui::filler(),
                                       tui::text(std::format("Time: {:02}:{:02}", minutes, seconds))
                                   }) | tui::flex,
                                   tui::separator(),
                                   boardComponent->Render() | tui::border | tui::hcenter
                               }) | tui::border | tui::center,
                               endMessage | tui::hcenter,
                               endMenu->Render() | tui::hcenter
                           }) | tui::center;
                })
            };
            screen.Loop(endScreenRender);
            timerRefreshThread.join();
            if (endScreenSelection == 1) {
                return EXIT_SUCCESS;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        return EXIT_FAILURE;
    }
}
