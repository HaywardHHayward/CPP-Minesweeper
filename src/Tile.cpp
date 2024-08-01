#include "Tile.hpp"


namespace Minesweeper {
    Tile::Tile(const std::uint8_t row, const std::uint8_t column): m_row(row), m_column(column) { }

    void Tile::incrementSurroundingMines() {
        if (m_surroundingMines + 1 >= 9) {
            return;
        }
        m_surroundingMines++;
    }

    void Tile::becomeChecked() {
        if (m_isFlagged) {
            return;
        }
        m_isChecked = true;
    }

    void Tile::becomeMine() {
        if (m_isChecked) {
            return;
        }
        m_isMine = true;
    }

    void Tile::toggleFlag() {
        if (m_isChecked) {
            return;
        }
        m_isFlagged = !m_isFlagged;
    }
} // Minesweeper
