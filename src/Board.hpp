#ifndef BOARD_HPP
#define BOARD_HPP
#include <BS_thread_pool.hpp> // https://github.com/bshoshany/thread-pool
#include <cstdint>
#include <deque>
#include <mutex>
#include <unordered_set>
#include <vector>

#include "Tile.hpp"


namespace Minesweeper {
    class Board final {
        BS::thread_pool m_threadPool{BS::thread_pool()};
        std::deque<std::mutex> m_tileLocks;
        std::unordered_set<Tile*> m_minedTiles;
        std::unordered_set<Tile*> m_uncheckedTiles;
        std::vector<Tile> m_board;
        std::mutex m_uncheckedMutex{};
        std::uint16_t m_mineCount;
        std::uint16_t m_flagCount{0};
        const std::uint8_t m_rowAmount;
        const std::uint8_t m_columnAmount;
        bool m_firstCheck{true};
        bool m_hitMine{false};

        void getSurroundingTiles(std::vector<Tile*>& vec, std::uint8_t row, std::uint8_t column);
        void generateMines(std::uint8_t row, std::uint8_t column);
        [[nodiscard]] std::size_t gridToLinear(std::uint8_t row, std::uint8_t column) const noexcept;

    public:
        explicit Board(std::uint8_t rowAmount, std::uint8_t columnAmount, std::uint16_t mineCount);
        [[nodiscard]] std::uint8_t getRowAmount() const noexcept;
        [[nodiscard]] std::uint8_t getColumnAmount() const noexcept;
        [[nodiscard]] std::uint16_t getMineCount() const noexcept;
        [[nodiscard]] std::int32_t getRemainingMines() const noexcept;
        [[nodiscard]] bool foundAllMines() const noexcept;
        [[nodiscard]] bool hitMine() const noexcept;
        Tile& at(std::uint8_t row, std::uint8_t column);
        void checkTile(std::uint8_t row, std::uint8_t column, bool topCheck = true);
        void toggleFlag(std::uint8_t row, std::uint8_t column) noexcept;
        void clearSafeTiles(std::uint8_t row, std::uint8_t column);
    };

    inline std::uint8_t Board::getRowAmount() const noexcept {
        return m_rowAmount;
    }

    inline std::uint8_t Board::getColumnAmount() const noexcept {
        return m_columnAmount;
    }

    inline std::uint16_t Board::getMineCount() const noexcept {
        return m_mineCount;
    }

    inline std::int32_t Board::getRemainingMines() const noexcept {
        return m_mineCount - m_flagCount;
    }

    inline bool Board::foundAllMines() const noexcept {
        return m_uncheckedTiles == m_minedTiles;
    }

    inline bool Board::hitMine() const noexcept {
        return m_hitMine;
    }

    inline std::size_t Board::gridToLinear(const std::uint8_t row, const std::uint8_t column) const noexcept {
        return row * m_columnAmount + column;
    }
} // Minesweeper

#endif //BOARD_HPP
