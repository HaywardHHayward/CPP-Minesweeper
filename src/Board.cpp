#include "Board.hpp"

#include <format>
#include <random>
#include <stdexcept>

#include "Tile.hpp"

namespace Minesweeper {
    void Board::getSurroundingTiles(std::vector<Tile*>& vec,
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
                vec.push_back(&at(row + r, column + c));
            }
        }
    }

    void Board::generateMines(const std::uint8_t row, const std::uint8_t column) {
        std::random_device rd{};
        std::seed_seq seedSeq{rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd()};
        std::mt19937 gen{seedSeq};
        std::uniform_int_distribution<std::uint8_t> rowRandom(0, m_rowAmount - 1);
        std::uniform_int_distribution<std::uint8_t> columnRandom(0, m_columnAmount - 1);
        std::vector<Tile*> surroundingTiles{8};
        getSurroundingTiles(surroundingTiles, row, column);
        const bool tooManyMines = m_mineCount >= m_rowAmount * m_columnAmount - surroundingTiles.size();
        const bool notEnoughSpace = m_rowAmount <= 3 && m_columnAmount <= 3;
        if (tooManyMines || notEnoughSpace) {
            while (m_minedTiles.size() < m_mineCount) {
                const uint8_t randRow{(rowRandom(gen))};
                const uint8_t randColumn{(columnRandom(gen))};
                if (randRow == row && randColumn == column) {
                    continue;
                }
                Tile& randTile{at(row, column)};
                if (m_minedTiles.insert(&randTile).second) {
                    // inserts tile into m_minedTiles, and checks if it was already in there
                    randTile.becomeMine();
                }
            }
        } else {
            while (m_minedTiles.size() < m_mineCount) {
                const uint8_t randRow{(rowRandom(gen))};
                const uint8_t randColumn{(columnRandom(gen))};
                if (randRow == row && randColumn == column) {
                    continue;
                }
                Tile& randTile{at(randRow, randColumn)};
                if (std::ranges::find(surroundingTiles, &randTile) != surroundingTiles.end()) {
                    // true if at(randRow, randColumn) is next to at(row, column)
                    continue;
                }
                if (m_minedTiles.insert(&randTile).second) {
                    // inserts tile into m_minedTiles, and checks if it was already in there
                    randTile.becomeMine();
                }
            }
        }
        surroundingTiles.clear();
        surroundingTiles.reserve(3 * m_mineCount); // minimum amount of tiles that could surround all the mines
        for (const Tile* tile: m_minedTiles) {
            getSurroundingTiles(surroundingTiles, tile->getRow(), tile->getColumn());
        }
        for (Tile* tile: surroundingTiles) {
            if (tile->isMine()) {
                continue;
            }
            tile->incrementSurroundingMines();
        }
    }

    Board::Board(const std::uint8_t rowAmount, const std::uint8_t columnAmount,
                 const std::uint16_t mineCount): m_minedTiles(mineCount),
                                                 m_uncheckedTiles(rowAmount * columnAmount),
                                                 m_mineCount(mineCount),
                                                 m_rowAmount(rowAmount),
                                                 m_columnAmount(columnAmount) {
        if (mineCount >= m_rowAmount * m_columnAmount) [[unlikely]] {
            throw(std::invalid_argument(std::format(
                "mineCount ({}) cannot be greater than or equal to the board size ({})", mineCount,
                m_rowAmount * m_columnAmount)));
        }
        m_board.reserve(rowAmount * columnAmount);
        for (int row = 0; row < rowAmount; row++) {
            for (int col = 0; col < columnAmount; col++) {
                Tile* newTile = &m_board.emplace_back(row, col);
                m_uncheckedTiles.insert(newTile);
            }
        }
    }

    std::uint8_t Board::getRowAmount() const {
        return m_rowAmount;
    }

    std::uint8_t Board::getColumnAmount() const {
        return m_columnAmount;
    }

    Tile& Board::at(const std::uint8_t row, const std::uint8_t column) {
        if (column >= m_columnAmount) [[unlikely]] {
            throw std::out_of_range(std::format(
                "column index provided ({}) greater than Board's max column amount ({})", column, m_columnAmount));
        }
        if (row >= m_rowAmount) [[unlikely]] {
            throw std::out_of_range(std::format(
                "row index provided ({}) greater than Board's max row amount ({})", row, m_rowAmount));
        }
        return m_board.at(row * m_columnAmount + column);
    }

    void Board::checkTile(const std::uint8_t row, const std::uint8_t column) {
        Tile& tile{at(row, column)};
        if (tile.isFlagged()) {
            return;
        }
        if (m_firstCheck) {
            generateMines(row, column);
            m_firstCheck = false;
        }
        if (tile.isChecked() || tile.isFlagged()) {
            return;
        }
        tile.becomeChecked();
        m_uncheckedTiles.erase(&tile);
        if (tile.getSurroundingMines() == 0 && !tile.isMine()) {
            std::vector<Tile*> surroundingTiles;
            getSurroundingTiles(surroundingTiles, row, column);
            for (const Tile* sTile: surroundingTiles) {
                checkTile(sTile->getRow(), sTile->getColumn());
            }
        }
    }

    void Board::toggleFlag(const std::uint8_t row, const std::uint8_t column) {
        Tile& tile = at(row, column);
        if (tile.isChecked()) {
            return;
        }
        tile.toggleFlag();
    }

    bool Board::foundAllMines() const {
        return m_uncheckedTiles == m_minedTiles;
    }
} // Minesweeper
