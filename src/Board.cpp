#include "Board.hpp"

#include <algorithm>
#include <format>
#include <random>
#include <stdexcept>
#include <thread>
#include <pcg/pcg_extras.hpp>
#include <pcg/pcg_random.hpp>

#include "Tile.hpp"

namespace Minesweeper {
    Board::Board(const std::uint8_t rowAmount, const std::uint8_t columnAmount,
                 const std::uint16_t mineCount): m_tileLocks(rowAmount * columnAmount), m_minedTiles(mineCount),
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
                Tile* newTile{&m_board.emplace_back(row, col)};
                m_uncheckedTiles.insert(newTile);
            }
        }
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
        return m_board.at(gridToLinear(row, column));
    }

    void Board::checkTile(const std::uint8_t row, const std::uint8_t column) {
        const std::unique_lock tileLock{m_tileLocks.at(gridToLinear(row, column)), std::try_to_lock};
        if (!tileLock.owns_lock()) {
            return;
        }
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
            std::vector<std::thread> threads;
            threads.reserve(surroundingTiles.size());
            for (const Tile* sTile: surroundingTiles) {
                threads.emplace_back(&Board::checkTile, this, sTile->getRow(), sTile->getColumn());
            }
            for (std::thread& thread: threads) {
                thread.join();
            }
        }
    }

    void Board::toggleFlag(const std::uint8_t row, const std::uint8_t column) noexcept {
        Tile& tile = at(row, column);
        if (tile.isChecked()) {
            return;
        }
        tile.toggleFlag();
    }

    void Board::clearSafeTiles(const std::uint8_t row, const std::uint8_t column) {
        const Tile& safeTile{at(row, column)};
        if (!safeTile.isChecked() || safeTile.getSurroundingMines() == 0) {
            return;
        }
        std::vector<Tile*> uncheckedTiles, trueUncheckedTiles, flaggedSurroundingTiles;
        getSurroundingTiles(uncheckedTiles, row, column);
        std::erase_if(uncheckedTiles, [this](const Tile* tile) { return tile->isChecked(); });
        std::ranges::partition_copy(uncheckedTiles, std::back_inserter(flaggedSurroundingTiles),
                                    std::back_inserter(trueUncheckedTiles), [this](const Tile* tile) {
                                        return tile->isFlagged();
                                    });
        if (flaggedSurroundingTiles.size() == safeTile.getSurroundingMines()) {
            std::ranges::for_each(trueUncheckedTiles, [this](const Tile* tile) {
                checkTile(tile->getRow(), tile->getColumn());
            });
        }
    }

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
        // TODO replace rejection sampling of 2d coordinates with randomly choosing an index from a vector that gets
        // updated to remove all mines, consistent mine generation time
        pcg32_k2_fast rng{pcg_extras::seed_seq_from<std::random_device>()};
        std::vector<Tile*> surroundingTiles;
        getSurroundingTiles(surroundingTiles, row, column);
        const bool tooManyMines{m_mineCount >= m_rowAmount * m_columnAmount - surroundingTiles.size()};
        const bool notEnoughSpace{m_rowAmount <= 3 && m_columnAmount <= 3};
        if (tooManyMines || notEnoughSpace) [[unlikely]] {
            while (m_minedTiles.size() < m_mineCount) {
                const uint8_t randRow{static_cast<uint8_t>(rng(m_rowAmount))};
                const uint8_t randColumn{static_cast<std::uint8_t>(rng(m_columnAmount))};
                if (randRow == row && randColumn == column) {
                    continue;
                }
                Tile& randTile{at(randRow, randColumn)};
                if (m_minedTiles.contains(&randTile)) [[likely]] {
                    // lots of mines, so more likely to select a spot more than once
                    continue;
                }
                m_minedTiles.insert(&randTile);
                randTile.becomeMine();
            }
        } else {
            while (m_minedTiles.size() < m_mineCount) {
                const uint8_t randRow{static_cast<uint8_t>(rng(m_rowAmount))};
                const uint8_t randColumn{static_cast<std::uint8_t>(rng(m_columnAmount))};
                if (randRow == row && randColumn == column) {
                    continue;
                }
                Tile& randTile{at(randRow, randColumn)};
                if (std::ranges::find(surroundingTiles, &randTile) != surroundingTiles.end()) {
                    // true if at(randRow, randColumn) is next to at(row, column)
                    continue;
                }
                if (m_minedTiles.contains(&randTile)) {
                    continue;
                }
                m_minedTiles.insert(&randTile);
                randTile.becomeMine();
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
} // Minesweeper
