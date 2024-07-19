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
        Tile(std::uint8_t row, std::uint8_t column);
        [[nodiscard]] std::uint8_t getRow() const;
        [[nodiscard]] std::uint8_t getColumn() const;
        [[nodiscard]] std::uint8_t getSurroundingMines() const;
        [[nodiscard]] bool isChecked() const;
        [[nodiscard]] bool isFlagged() const;
        [[nodiscard]] bool isMine() const;
        void incrementSurroundingMines();
        void becomeChecked();
        void becomeMine();
        void toggleFlag();
    };
} // Minesweeper

#endif //TILE_HPP
