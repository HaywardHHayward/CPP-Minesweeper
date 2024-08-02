#include "Tile.hpp"


namespace Minesweeper {
    Tile::Tile(const std::uint8_t row, const std::uint8_t column) noexcept: m_row(row), m_column(column) { }

    void Tile::incrementSurroundingMines() noexcept {
        if (m_surroundingMines + 1 >= 9) {
            return;
        }
        m_surroundingMines++;
    }

    void Tile::becomeChecked() noexcept {
        if (m_isFlagged) {
            return;
        }
        m_isChecked = true;
    }

    void Tile::becomeMine() noexcept {
        if (m_isChecked) {
            return;
        }
        m_isMine = true;
    }

    void Tile::toggleFlag() noexcept {
        if (m_isChecked) {
            return;
        }
        m_isFlagged = !m_isFlagged;
    }
} // Minesweeper
