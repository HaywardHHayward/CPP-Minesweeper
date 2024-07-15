cmake_minimum_required(VERSION 3.29)
project(CPP_Minesweeper)

set(CMAKE_CXX_STANDARD 20)

add_executable(CPP_Minesweeper src/main.cpp
        src/Tile.cpp
        src/Tile.hpp
        src/Board.cpp
        src/Board.hpp)
