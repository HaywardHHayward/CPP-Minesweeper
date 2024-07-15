#include "Board.hpp"

#include <algorithm>
#include <format>
#include <random>
#include <stdexcept>

namespace Minesweeper {
    void Board::getSurroundingTiles(const std::shared_ptr<std::vector<Tile*> >& vec,
                                    const std::uint8_t row, const std::uint8_t column) {
        for (int r = -1; r <= 1; r++) {
            if (r + row < 0 || r + row >= m_rowAmount) {
                continue;
            }
            for (int c = -1; c <= 1; c++) {
                if (c + column < 0 || c + column >= m_columnAmount) {
                    continue;
                }
                if (c == 0 && r == 0) {
                    continue;
                }
                vec->push_back(&at(row + r, column + c));
            }
        }
    }

    void Board::generateMines(const std::uint8_t row, const std::uint8_t column) {
        std::random_device rd{};
        std::seed_seq seedSeq{rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd()};
        std::mt19937_64 gen{seedSeq};
        std::uniform_int_distribution<std::uint8_t> rowRandom{0, static_cast<std::uint8_t>(m_rowAmount - 1)};
        std::uniform_int_distribution<std::uint8_t> columnRandom{0, static_cast<std::uint8_t>(m_columnAmount - 1)};
        const std::shared_ptr<std::vector<Tile*> > surroundingTiles = std::make_shared<std::vector<Tile*> >();
        getSurroundingTiles(surroundingTiles, row, column);
        const bool tooManyMines = m_mineCount >= m_rowAmount * m_columnAmount - surroundingTiles->size();
        const bool notEnoughSpace = m_rowAmount <= 3 && m_columnAmount <= 3;
        if (tooManyMines || notEnoughSpace) {
            while (m_minedTiles.size() < m_mineCount) {
                const uint8_t randRow{rowRandom(gen)};
                const uint8_t randColumn{columnRandom(gen)};
                if (randRow == row && randColumn == column) {
                    continue;
                }
                Tile& randTile{at(row, column)};
                if (m_minedTiles.insert(&randTile).second) {
                    // inserts tile into m_minedTiles, and checks if it was already in there
                    randTile.becomeMine();
                }
            }
            return;
        }
        while (m_minedTiles.size() < m_mineCount) {
            const uint8_t randRow{rowRandom(gen)};
            const uint8_t randColumn{columnRandom(gen)};
            if (randRow == row && randColumn == column) {
                continue;
            }
            Tile& randTile{at(randRow, randColumn)};
            if (std::ranges::find(*surroundingTiles, &randTile) != surroundingTiles->end()) {
                // true if at(randRow, randColumn) is next to at(row, column)
                continue;
            }
            if (m_minedTiles.insert(&randTile).second) {
                // inserts tile into m_minedTiles, and checks if it was already in there
                randTile.becomeMine();
            }
        }
    }

    Board::Board(const std::uint8_t rowAmount, const std::uint8_t columnAmount,
                 const std::uint16_t mineCount): m_minedTiles(mineCount), m_flaggedTiles(rowAmount * columnAmount),
                                                 m_mineCount(mineCount),
                                                 m_rowAmount(rowAmount),
                                                 m_columnAmount(columnAmount) {
        m_board.reserve(rowAmount * columnAmount);
        for (int row = 0; row < rowAmount; row++) {
            for (int col = 0; col < columnAmount; col++) {
                m_board.emplace_back(row, col);
            }
        }
    }

    Tile& Board::at(const std::uint8_t row, const std::uint8_t column) {
        if (column >= m_columnAmount) {
            throw std::out_of_range(std::format(
                "column index provided ({}) greater than Board's max column amount ({})", column, m_columnAmount));
        }
        if (row >= m_rowAmount) {
            throw std::out_of_range(std::format(
                "row index provided ({}) greater than Board's max row amount ({})", row, m_rowAmount));
        }
        return m_board.at(row * m_columnAmount + column);
    }

    void Board::checkTile(const std::uint8_t row, const std::uint8_t column) {
        if (m_firstCheck) {
            generateMines(row, column);
            m_firstCheck = false;
        }
        Tile& tile{at(row, column)};
        if (tile.isChecked() || tile.isFlagged()) {
            return;
        }
        tile.becomeChecked();
        if (tile.getSurroundingMines() == 0) {
            const std::shared_ptr<std::vector<Tile*> > surroundingTiles = std::make_shared<std::vector<Tile*> >();
            getSurroundingTiles(surroundingTiles, row, column);
            for (const Tile* s_tile: *surroundingTiles) {
                checkTile(s_tile->getRow(), s_tile->getColumn());
            }
        }
    }

    void Board::toggleFlag(const std::uint8_t row, const std::uint8_t column) {
        at(row, column).toggleFlag();
    }
} // Minesweeper
