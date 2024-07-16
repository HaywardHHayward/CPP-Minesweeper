#include "Tile.hpp"

#include <stdexcept>

namespace Minesweeper {
    Tile::Tile(const std::uint8_t row, const std::uint8_t column): m_row(row), m_column(column) { }
    std::uint8_t Tile::getRow() const { return m_row; }
    std::uint8_t Tile::getColumn() const { return m_column; }
    std::uint8_t Tile::getSurroundingMines() const { return m_surroundingMines; }
    bool Tile::isChecked() const { return m_isChecked; }
    bool Tile::isFlagged() const { return m_isFlagged; }
    bool Tile::isMine() const { return m_isMine; }

    void Tile::incrementSurroundingMines() {
        if (m_surroundingMines + 1 >= 9) [[unlikely]] {
            throw(std::invalid_argument("m_surrouindingMines cannot be incremented to more than 8."));
        }
        m_surroundingMines++;
    }

    void Tile::becomeChecked() {
        m_isChecked = true;
    }

    void Tile::becomeMine() {
        m_isMine = true;
    }

    void Tile::toggleFlag() {
        m_isFlagged = !m_isFlagged;
    }
} // Minesweeper
