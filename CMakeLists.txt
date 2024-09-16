cmake_minimum_required(VERSION 3.28)
project(MinesweeperC++)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
        GIT_TAG v5.0.0
)
FetchContent_MakeAvailable(ftxui)

FetchContent_Declare(
        argparse
        GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)

include_directories(lib)

add_executable(Minesweeper src/main.cpp
        src/Tile.hpp
        src/Board.cpp
        src/Board.hpp
        src/TileComponentBase.cpp
        src/TileComponentBase.hpp
        src/BoardComponentBase.cpp
        src/BoardComponentBase.hpp
)


target_link_libraries(Minesweeper
        PRIVATE ftxui::component
        PRIVATE ftxui::dom
        PRIVATE ftxui::screen
        PRIVATE argparse
)

