cmake_minimum_required(VERSION 3.29)
project(CPP_Minesweeper)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
        GIT_TAG v5.0.0
)

FetchContent_GetProperties(ftxui)
if(NOT ftxui_POPULATED)
    FetchContent_Populate(ftxui)
    add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

add_executable(CPP_Minesweeper src/main.cpp
        src/Tile.cpp
        src/Tile.hpp
        src/Board.cpp
        src/Board.hpp
        src/TileComponent.cpp
        src/TileComponent.hpp)

target_link_libraries(CPP_Minesweeper
        PRIVATE ftxui::screen
        PRIVATE ftxui::dom
        PRIVATE ftxui::component # Not needed for this example.
)