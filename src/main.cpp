#include <chrono>
#include <cstdint>
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
    namespace tui = ftxui;
    try {
        while (true) {
            tui::ScreenInteractive screen{tui::ScreenInteractive::Fullscreen()};
            screen.SetCursor({0, 0, tui::Screen::Cursor::Shape::Hidden});
            enum Difficulty: int {
                beginner = 0,
                intermediate = 1,
                expert = 2,
                custom = 3
            };
            Difficulty difficultySelection{custom};
            const std::vector<std::string> difficultyEntries{"Beginner", "Intermediate", "Expert", "Custom"};
            const tui::Component menu = tui::Menu(&difficultyEntries, std::bit_cast<int*>(&difficultySelection),
                                                  {.on_enter{screen.ExitLoopClosure()}});

            tui::Component difficultyRender = Renderer(menu, [&menu] {
                return tui::vbox({
                           tui::text("Choose your difficulty"),
                           tui::separator(),
                           menu->Render()
                       }) | tui::border
                       | tui::center;
            });

            screen.Loop(difficultyRender);
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

                    const tui::Component customInputs{
                        tui::Container::Vertical({
                            Input(&rowStr, rowOption) | tui::CatchEvent([](const tui::Event& event) {
                                return event.is_character() && !std::isdigit(event.character()[0]);
                            }) | tui::CatchEvent([&rowStr](const tui::Event& event) {
                                return event.is_character() && rowStr.size() > std::numeric_limits<
                                           std::uint8_t>::digits10;
                            }),
                            Input(&columnStr, columnOption) | tui::CatchEvent([](const tui::Event& event) {
                                return event.is_character() && !std::isdigit(event.character()[0]);
                            }) | tui::CatchEvent([&columnStr](const tui::Event& event) {
                                return event.is_character() && columnStr.size() > std::numeric_limits<
                                           std::uint8_t>::digits10;
                            }),
                            Input(&mineStr, mineOption) | tui::CatchEvent([](const tui::Event& event) {
                                return event.is_character() && !std::isdigit(event.character()[0]);
                            }) | tui::CatchEvent([&mineStr](const tui::Event& event) {
                                return event.is_character() && mineStr.size() > std::numeric_limits<
                                           std::uint16_t>::digits10;
                            })
                        })
                    };

                    auto clickFunction = [&] {
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
                    };
                    tui::Component customButton = tui::Button({
                        .label{"Enter row, column, and mine amounts."},
                        .on_click{clickFunction}
                    });
                    const tui::Component customMenu{tui::Container::Vertical({customInputs, customButton})};
                    screen.Loop(customMenu | tui::border | tui::center);
                    board = std::make_shared<Board>(row, column, mines);
                }
                break;
            }

            const BoardComponent baseBoard{BoardComponentBase::Create(board, screen.ExitLoopClosure())};
            const tui::Component boardComponent{Hoverable(baseBoard, baseBoard->hovered())};
            const tui::Component boardRenderer = Renderer(boardComponent, [&boardComponent] {
                return boardComponent->Render() | tui::border;
            });

            tui::Component countRenderer = tui::Renderer([&board] {
                return tui::text(std::format("Remaining mines: {:{}}", board->getRemainingMines(),
                                             std::to_string(board->getMineCount()).length()));
            });

            using steadyClock = std::chrono::steady_clock;
            const steadyClock::time_point startTime{steadyClock::now()};
            auto timeRenderer = [&startTime](
                const steadyClock::time_point endTime = steadyClock::now()) {
                return tui::Renderer([&] {
                    namespace time = std::chrono;
                    const time::duration elapsedTime{endTime - startTime};
                    const bool isMaxedOut{elapsedTime > time::minutes(99) + time::seconds(59)};
                    const std::uint8_t seconds = static_cast<std::uint8_t>(
                        time::duration_cast<time::seconds>(elapsedTime).count() % 60);
                    const auto minutes{time::duration_cast<time::minutes>(elapsedTime).count()};
                    return tui::text(isMaxedOut ? "Time: MAXED" : std::format("Time: {:02}:{:02}", minutes, seconds));
                });
            };

            tui::Component infoRenderer = tui::Renderer([&countRenderer, &timeRenderer] {
                return tui::hbox({
                    countRenderer->Render(),
                    tui::filler(),
                    tui::separator(),
                    tui::filler(),
                    timeRenderer()->Render()
                });
            });

            const tui::Component gameplayRender = Renderer(boardRenderer, [&infoRenderer, &boardRenderer] {
                return tui::vbox({
                           infoRenderer->Render() | tui::flex,
                           tui::separator(),
                           boardRenderer->Render() | tui::hcenter
                       }) | tui::border | tui::center;
            });

            steadyClock::time_point nextTime{startTime + std::chrono::seconds(1)};

            #ifdef __cpp_lib_jthread
            using timerType = std::jthread;
            #define TIMER_ARGUMENT (const std::stop_token& stopToken)
            auto timerPredicate = [] (const std::stop_token& stop) {
                return stop.stop_requested();
            };
            auto stopThread = [] (std::jthread& thread) {
                thread.request_stop();
            };
            #else
            using timerType = std::thread;
            #define TIMER_ARGUMENT ()
            std::atomic_bool stopToken{false};
            auto timerPredicate = [](const std::atomic_bool& stop) {
                return stop.load();
            };
            auto stopThread = [&stopToken]([[maybe_unused]] timerType& thread) {
                stopToken.store(true);
            };
            #endif

            timerType timerRefreshThread([&] TIMER_ARGUMENT {
                while (!timerPredicate(stopToken)) {
                    std::this_thread::sleep_until(nextTime);
                    if (timerPredicate(stopToken)) {
                        return;
                    }
                    screen.PostEvent(tui::Event::Custom);
                    nextTime += std::chrono::seconds(1);
                }
            });
            screen.Loop(gameplayRender);
            stopThread(timerRefreshThread);
            const std::vector<std::string> endEntries{"Retry", "Exit"};
            enum EndSelection : int {
                retry = 0,
                exit = 1
            };
            EndSelection endScreenSelection{exit};
            const tui::Component endMenu = tui::Menu(&endEntries, std::bit_cast<int*>(&endScreenSelection),
                                                     {.on_enter = screen.ExitLoopClosure()});
            const tui::Element endMessage = board->hitMine()
                                                ? tui::text("You hit a mine! You lose!")
                                                : tui::text("You flagged all the mines! You win!");

            steadyClock::time_point endScreenTime{steadyClock::now()};
            const tui::Element endInfo = tui::hbox({
                countRenderer->Render(),
                tui::filler(),
                tui::separator(),
                tui::filler(),
                timeRenderer(endScreenTime)->Render()
            });

            const tui::Component endScreenRender = Renderer(endMenu, [&] {
                return tui::vbox({
                           tui::vbox({
                               endInfo | tui::flex,
                               tui::separator(),
                               boardRenderer->Render() | tui::border | tui::hcenter
                           }) | tui::border | tui::center,
                           endMessage | tui::hcenter,
                           endMenu->Render() | tui::hcenter
                       }) | tui::center;
            });

            screen.Loop(endScreenRender);
            timerRefreshThread.join();
            if (endScreenSelection == exit) {
                return EXIT_SUCCESS;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        std::cout << "An unhandled exception occurred, exiting program..." << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Non-standard exception (not inheriting from std::exception)" << std::endl;
        std::cout << "An unhandled exception occurred, exiting program..." << std::endl;
        return EXIT_FAILURE;
    }
}
