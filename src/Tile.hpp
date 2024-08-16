#ifndef TILE_HPP
#define TILE_HPP

#include <cstdint>

namespace Minesweeper {
    class Tile final {
        const std::uint8_t m_row;
        const std::uint8_t m_column;
        std::uint8_t m_surroundingMines : 4 {0};
        bool m_isChecked : 1 {false};
        bool m_isFlagged : 1 {false};
        bool m_isMine : 1 {false};

    public:
        explicit Tile(std::uint8_t row, std::uint8_t column) noexcept;

        [[nodiscard]] std::uint8_t getRow() const noexcept;
        [[nodiscard]] std::uint8_t getColumn() const noexcept;
        [[nodiscard]] std::uint8_t getSurroundingMines() const noexcept;
        [[nodiscard]] bool isChecked() const noexcept;
        [[nodiscard]] bool isFlagged() const noexcept;
        [[nodiscard]] bool isMine() const noexcept;
        void incrementSurroundingMines() noexcept;
        void becomeChecked() noexcept;
        void becomeMine() noexcept;
        void toggleFlag() noexcept;
    };

    inline Tile::Tile(const std::uint8_t row, const std::uint8_t column) noexcept: m_row(row), m_column(column) { }

    inline std::uint8_t Tile::getRow() const noexcept {
        return m_row;
    }

    inline std::uint8_t Tile::getColumn() const noexcept {
        return m_column;
    }

    inline std::uint8_t Tile::getSurroundingMines() const noexcept {
        return m_surroundingMines;
    }

    inline bool Tile::isChecked() const noexcept {
        return m_isChecked;
    }

    inline bool Tile::isFlagged() const noexcept {
        return m_isFlagged;
    }

    inline bool Tile::isMine() const noexcept {
        return m_isMine;
    }

    inline void Tile::incrementSurroundingMines() noexcept {
        if (m_surroundingMines + 1 >= 9) {
            return;
        }
        m_surroundingMines++;
    }

    inline void Tile::becomeChecked() noexcept {
        if (m_isFlagged) {
            return;
        }
        m_isChecked = true;
    }

    inline void Tile::becomeMine() noexcept {
        if (m_isChecked) {
            return;
        }
        m_isMine = true;
    }

    inline void Tile::toggleFlag() noexcept {
        if (m_isChecked) {
            return;
        }
        m_isFlagged = !m_isFlagged;
    }
} // Minesweeper

#endif //TILE_HPP
