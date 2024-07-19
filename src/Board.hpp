#ifndef BOARD_HPP
#define BOARD_HPP

#include <cstdint>
#include <unordered_set>
#include <vector>


namespace Minesweeper {
    class Tile;

    class Board final {
        std::unordered_set<Tile*> m_minedTiles;
        std::unordered_set<Tile*> m_flaggedTiles;
        std::vector<Tile> m_board;
        std::uint16_t m_mineCount;
        const std::uint8_t m_rowAmount;
        const std::uint8_t m_columnAmount;
        bool m_firstCheck{true};

        void getSurroundingTiles(std::vector<Tile*>& vec,
                                 std::uint8_t row, std::uint8_t column);
        void generateMines(std::uint8_t row, std::uint8_t column);

    public:
        Board(std::uint8_t rowAmount, std::uint8_t columnAmount, std::uint16_t mineCount);
        Tile& at(std::uint8_t row, std::uint8_t column);
        void checkTile(std::uint8_t row, std::uint8_t column);
        void toggleFlag(std::uint8_t row, std::uint8_t column);
    };
} // Minesweeper

#endif //BOARD_HPP
