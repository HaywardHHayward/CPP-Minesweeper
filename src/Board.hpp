#ifndef BOARD_HPP
#define BOARD_HPP

#include <cstdint>
#include <deque>
#include <mutex>
#include <unordered_set>
#include <vector>


namespace Minesweeper {
    class Tile;

    class Board final {
        std::deque<std::mutex> m_tileLocks;
        std::unordered_set<Tile*> m_minedTiles{};
        std::unordered_set<Tile*> m_uncheckedTiles{};
        std::vector<Tile> m_board{};
        std::uint16_t m_mineCount;
        const std::uint8_t m_rowAmount;
        const std::uint8_t m_columnAmount;
        bool m_firstCheck{true};

        void getSurroundingTiles(std::vector<Tile*>& vec,
                                 std::uint8_t row, std::uint8_t column);
        void generateMines(std::uint8_t row, std::uint8_t column);
        [[nodiscard]] std::size_t gridToLinear(std::uint8_t row, std::uint8_t column) const;

    public:
        explicit Board(std::uint8_t rowAmount, std::uint8_t columnAmount, std::uint16_t mineCount);
        [[nodiscard]] std::uint8_t getRowAmount() const noexcept;
        [[nodiscard]] std::uint8_t getColumnAmount() const noexcept;
        [[nodiscard]] bool foundAllMines() const;
        Tile& at(std::uint8_t row, std::uint8_t column);
        void checkTile(std::uint8_t row, std::uint8_t column);
        void toggleFlag(std::uint8_t row, std::uint8_t column) noexcept;
        void clearSafeTiles(std::uint8_t row, std::uint8_t column);
    };

    inline std::uint8_t Board::getRowAmount() const noexcept {
        return m_rowAmount;
    }

    inline std::uint8_t Board::getColumnAmount() const noexcept {
        return m_columnAmount;
    }

    inline bool Board::foundAllMines() const {
        return m_uncheckedTiles == m_minedTiles;
    }

    inline std::size_t Board::gridToLinear(const std::uint8_t row, const std::uint8_t column) const {
        return row * m_columnAmount + column;
    }
} // Minesweeper

#endif //BOARD_HPP
