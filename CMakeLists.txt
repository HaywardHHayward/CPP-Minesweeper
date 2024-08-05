cmake_minimum_required(VERSION 3.28)
project(MinesweeperC++)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
        GIT_TAG v5.0.0
)

FetchContent_GetProperties(ftxui)
if (NOT ftxui_POPULATED)
    FetchContent_Populate(ftxui)
    add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()

include_directories(lib)

add_executable(Minesweeper src/main.cpp
        src/Tile.cpp
        src/Board.cpp
        src/TileComponentBase.cpp
        src/BoardComponentBase.cpp)

target_link_libraries(Minesweeper
        PRIVATE ftxui::screen
        PRIVATE ftxui::dom
        PRIVATE ftxui::component
)