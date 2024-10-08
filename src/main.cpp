#include <chrono>
#include <cstdint>
#include <thread>
#include <argparse/argparse.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include "Board.hpp"
#include "BoardComponentBase.hpp"

#if defined(_MSVC_VER) && !defined(__clang__)
#define UNREACHABLE() __assume(false)
#elif defined(__GNUC__) || defined(__clang__)
#define UNREACHABLE() __builtin_unreachable()
#else
[[noreturn]] inline void unreachable() { }
#define UNREACHABLE() unreachable()
#endif

void customInitialization(ftxui::ScreenInteractive& screen, std::shared_ptr<Minesweeper::Board>& board);

void parseArguments(int argc, const char* const argv[], std::shared_ptr<Minesweeper::Board>& board);

int main(const int argc, const char* const argv[]) {
    using Minesweeper::Board, Minesweeper::BoardComponentBase, Minesweeper::BoardComponent;
    namespace tui = ftxui;
    std::shared_ptr<Board> board{nullptr};
    try {
        parseArguments(argc, argv, board);
    } catch (...) {
        return EXIT_FAILURE;
    }
    try {
        tui::ScreenInteractive screen{tui::ScreenInteractive::Fullscreen()};
        screen.SetCursor({0, 0, tui::Screen::Cursor::Shape::Hidden});
        while (true) {
            if (board == nullptr) {
                int difficultySelection{-1};
                const std::vector<std::string> difficultyEntries{"Beginner", "Intermediate", "Expert", "Custom"};
                const tui::Component difficultyMenu = tui::Menu(&difficultyEntries, &difficultySelection,
                                                                {.on_enter{screen.ExitLoopClosure()}});

                const tui::Component difficultyRender = Renderer(difficultyMenu, [&] {
                    return tui::vbox({
                               tui::text("Choose your difficulty"),
                               tui::separator(),
                               difficultyMenu->Render()
                           }) | tui::border
                           | tui::center;
                });

                screen.Loop(difficultyRender);
                enum class Difficulty: int {
                    beginner = 0,
                    intermediate = 1,
                    expert = 2,
                    custom = 3
                };
                switch (static_cast<Difficulty>(difficultySelection)) {
                    case Difficulty::beginner:
                        board = std::make_shared<Board>(9, 9, 10);
                        break;
                    case Difficulty::intermediate:
                        board = std::make_shared<Board>(16, 16, 40);
                        break;
                    case Difficulty::expert:
                        board = std::make_shared<Board>(16, 30, 99);
                        break;
                    case Difficulty::custom:
                        customInitialization(screen, board);
                        break;
                    [[unlikely]] default:
                        throw std::out_of_range("Difficulty selection out of range");
                }
            }

            const BoardComponent baseBoard{BoardComponentBase::Create(board, screen.ExitLoopClosure())};
            const tui::Component boardComponent{Hoverable(baseBoard, &baseBoard->hovered)};
            const tui::Component boardRenderer = Renderer(boardComponent, [&] {
                return boardComponent->Render() | tui::border;
            });

            tui::Component countRenderer = tui::Renderer([&] {
                return tui::text(std::format("Remaining mines: {:{}}", board->getRemainingMines(),
                                             std::to_string(board->getMineCount()).length()));
            });

            using steadyClock = std::chrono::steady_clock;
            const steadyClock::time_point startTime{steadyClock::now()};
            auto timeRenderer = [&](
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

            tui::Component infoRenderer = tui::Renderer([&] {
                return tui::hbox({
                    countRenderer->Render(),
                    tui::filler(),
                    tui::separator(),
                    tui::filler(),
                    timeRenderer()->Render()
                });
            });

            const tui::Component gameplayRender = Renderer(boardRenderer, [&] {
                return tui::vbox({
                           infoRenderer->Render() | tui::flex,
                           tui::separator(),
                           boardRenderer->Render() | tui::hcenter
                       }) | tui::border | tui::center;
            });

            std::atomic_bool stopToken{false};
            steadyClock::time_point nextTime{startTime + std::chrono::seconds(1)};
            std::thread timerRefreshThread([&] {
                while (!stopToken.load()) {
                    std::this_thread::sleep_until(nextTime);
                    nextTime += std::chrono::seconds(1);
                    screen.PostEvent(tui::Event::Custom);
                }
            });

            screen.Loop(gameplayRender);

            stopToken.store(true);
            const std::vector<std::string> endEntries{"Retry", "Exit"};
            int endScreenSelection{-1};
            const tui::Component endMenu = tui::Menu(&endEntries, &endScreenSelection,
                                                     {.on_enter = screen.ExitLoopClosure()});
            const tui::Element endMessage = board->hitMine()
                                                ? tui::text("You hit a mine! You lose!")
                                                : tui::text("You flagged all the mines! You win!");

            const steadyClock::time_point endScreenTime{steadyClock::now()};
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
            enum class EndSelection : int {
                retry = 0,
                exit = 1
            };
            switch (static_cast<EndSelection>(endScreenSelection)) {
                case EndSelection::retry:
                    board.reset();
                    continue;
                case EndSelection::exit:
                    return EXIT_SUCCESS;
                [[unlikely]] default:
                    throw std::out_of_range("EndSelection out of range");
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

void customInitialization(ftxui::ScreenInteractive& screen, std::shared_ptr<Minesweeper::Board>& board) {
    std::string rowStr, columnStr, mineStr;
    std::uint8_t row, column;
    std::uint16_t mines;

    const ftxui::InputOption rowOption{
        .placeholder{"Enter number of rows"},
        .multiline{false},
    };
    const ftxui::InputOption columnOption{
        .placeholder{"Enter number of columns"},
        .multiline{false},
    };
    const ftxui::InputOption mineOption{
        .placeholder{"Enter number of mines"},
        .multiline{false},
    };
    const ftxui::Component customInputs{
        ftxui::Container::Vertical({
            Input(&rowStr, rowOption) | ftxui::CatchEvent([](const ftxui::Event& event) {
                return event.is_character() && !std::isdigit(event.character()[0]);
            }) | ftxui::CatchEvent([&](const ftxui::Event& event) {
                return event.is_character() && rowStr.size() > std::numeric_limits<
                           std::uint8_t>::digits10;
            }),
            Input(&columnStr, columnOption) | ftxui::CatchEvent([](const ftxui::Event& event) {
                return event.is_character() && !std::isdigit(event.character()[0]);
            }) | ftxui::CatchEvent([&](const ftxui::Event& event) {
                return event.is_character() && columnStr.size() > std::numeric_limits<
                           std::uint8_t>::digits10;
            }),
            Input(&mineStr, mineOption) | ftxui::CatchEvent([](const ftxui::Event& event) {
                return event.is_character() && !std::isdigit(event.character()[0]);
            }) | ftxui::CatchEvent([&](const ftxui::Event& event) {
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
            || minesRaw == 0 || minesRaw >= rowRaw * columnRaw) {
            if (rowRaw == 0 || rowRaw > UINT8_MAX) {
                rowStr.clear();
            }
            if (columnRaw == 0 || columnRaw > UINT8_MAX) {
                columnStr.clear();
            }
            if (minesRaw == 0 || minesRaw >= rowRaw * columnRaw) {
                mineStr.clear();
            }
            return;
        }
        row = static_cast<std::uint8_t>(rowRaw);
        column = static_cast<std::uint8_t>(columnRaw);
        mines = static_cast<std::uint16_t>(minesRaw);
        screen.Exit();
    };
    ftxui::Component customButton = ftxui::Button({
        .label{"Enter row, column, and mine amounts."},
        .on_click{clickFunction}
    });
    const ftxui::Component customMenu{ftxui::Container::Vertical({customInputs, customButton})};
    screen.Loop(customMenu | ftxui::border | ftxui::center);
    screen.SetCursor({0, 0, ftxui::Screen::Cursor::Shape::Hidden});
    board = std::make_shared<Minesweeper::Board>(row, column, mines);
}

void parseArguments(const int argc, const char* const argv[], std::shared_ptr<Minesweeper::Board>& board) {
    argparse::ArgumentParser parser("minesweeper", "", argparse::default_arguments::help);
    parser.set_usage_max_line_width(80);
    parser.set_usage_break_on_mutex();

    auto& difficultyArguments{parser.add_mutually_exclusive_group()};
    difficultyArguments.add_argument("--b", "--beginner").help("Creates a 9 x 9 board with 10 mines.").flag();
    difficultyArguments.add_argument("--i", "--intermediate").help("Creates a 16 x 16 board with 40 mines.").flag();
    difficultyArguments.add_argument("--e", "--expert").help("Creates a 16 x 30 board with 99 mines.").flag();
    difficultyArguments.add_argument("--c", "--custom").help(
                           "Creates a ROW x COLUMN with MINES mines. ROW and COLUMN must be less than 256, and MINES cannot be equal to or greater than ROW * COLUMN.")
                       .metavar("ROW COLUMN MINES").nargs(3);

    parser.add_description("A Minesweeper instance you can play in your terminal.");
    parser.add_epilog("Providing no arguments will allow you to select the difficulty from the application itself.");

    try {
        parser.parse_args(argc, argv);
        if (parser.is_used("--b")) {
            board = std::make_shared<Minesweeper::Board>(9, 9, 10);
            return;
        }
        if (parser.is_used("--i")) {
            board = std::make_shared<Minesweeper::Board>(16, 16, 40);
            return;
        }
        if (parser.is_used("--e")) {
            board = std::make_shared<Minesweeper::Board>(16, 30, 99);
            return;
        }
        if (parser.is_used("--c")) {
            auto stringArguments{parser.get<std::vector<std::string> >("--c")};
            auto isNotPositiveNumber = [](const std::string_view& string) {
                return std::ranges::any_of(string, [](const unsigned char& ch) {
                    return !std::isdigit(ch);
                });
            };
            if (std::ranges::any_of(stringArguments, isNotPositiveNumber)) {
                throw std::logic_error("Invalid usage. ROW, COLUMN, and MINES must all be a positive number");
            }
            auto isAllZero = [](const std::string_view& string) {
                return string.find_first_not_of('0') == std::string_view::npos;
            };
            if (std::ranges::any_of(stringArguments, isAllZero)) {
                throw std::logic_error("Invalid usage. ROW, COLUMN, and MINES cannot be zero");
            }
            auto removeLeadingZeros = [](std::string& string) {
                string = string.substr(string.find_first_not_of('0'));
            };
            std::ranges::for_each(stringArguments, removeLeadingZeros);
            auto validate = [](const bool row, const bool column, const bool mines) {
                if (row || column || mines) {
                    std::string errorMessage{"Invalid usage. "};
                    if (row) {
                        errorMessage += "ROW";
                    }
                    if (column) {
                        if (row) {
                            errorMessage += " and ";
                        }
                        errorMessage += "COLUMN";
                    }
                    if (row || column) {
                        errorMessage += " must be less than 256. ";
                    }
                    if (mines) {
                        errorMessage += "MINES must be less than ROW multiplied by COLUMN.";
                    }
                    throw std::invalid_argument(errorMessage);
                }
            };
            const bool rowTooLong{stringArguments[0].length() >= std::numeric_limits<unsigned long>::digits10};
            const bool columnTooLong{stringArguments[1].length() >= std::numeric_limits<unsigned long>::digits10};
            const bool minesTooLong{stringArguments[2].length() >= std::numeric_limits<unsigned long>::digits10};
            validate(rowTooLong, columnTooLong, minesTooLong);
            std::array<unsigned long, 3> arguments{};
            std::ranges::transform(stringArguments, std::begin(arguments), [](const std::string& string) {
                return std::stoul(string);
            });
            const bool rowBigger{arguments[0] > UINT8_MAX};
            const bool columnBigger{arguments[1] > UINT8_MAX};
            const bool minesBigger{arguments[2] >= arguments[0] * arguments[1]};
            validate(rowBigger, columnBigger, minesBigger);
            board = std::make_shared<Minesweeper::Board>(static_cast<std::uint8_t>(arguments[0]),
                                                         static_cast<std::uint8_t>(arguments[1]),
                                                         static_cast<std::uint16_t>(arguments[2]));
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        std::cerr << parser;
        throw; // throws the exception back into main where all destructors can be called safely
    }
}
