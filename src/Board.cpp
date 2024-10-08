#include "Board.hpp"

#include <algorithm>
#include <BS_thread_pool.hpp> // https://github.com/bshoshany/thread-pool
#include <cassert>
#ifndef _MSC_VER
#define PCG_RAND
#include <pcg/pcg_extras.hpp>
#include <pcg/pcg_random.hpp>
#endif
#include <random>
#include <ranges>

namespace Minesweeper {
    Board::Board(const std::uint8_t rowAmount,
                 const std::uint8_t columnAmount,
                 const std::uint16_t mineCount): m_tileLocks(rowAmount * columnAmount),
                                                 m_minedTiles(mineCount),
                                                 m_uncheckedTiles(rowAmount * columnAmount),
                                                 m_mineCount{mineCount},
                                                 m_rowAmount{rowAmount},
                                                 m_columnAmount{columnAmount} {
        assert(mineCount < m_rowAmount * m_columnAmount);
        m_board.reserve(rowAmount * columnAmount);
        for (std::uint_fast8_t row{0}; row < rowAmount; row++) {
            for (std::uint_fast8_t col{0}; col < columnAmount; col++) {
                m_board.emplace_back(row, col);
            }
        }
        for (Tile& tile: m_board) {
            m_uncheckedTiles.insert(&tile);
        }
    }

    Tile& Board::atCoordinate(const std::uint8_t row, const std::uint8_t column) {
        #ifdef NDEBUG
        return m_board[gridToLinear(row, column)];
        #else
        assert(row < m_rowAmount && column < m_columnAmount);
        return m_board.at(row * m_columnAmount + column);
        #endif
    }

    void Board::checkTile(const std::uint8_t row, const std::uint8_t column) {
        Tile& tile{atCoordinate(row, column)};
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
        if (tile.isMine()) {
            m_hitMine = true;
            return;
        }
        if (tile.getSurroundingMines() == 0) {
            #ifdef NDEBUG
            const std::lock_guard tileLock{m_tileLocks[gridToLinear(row, column)]};
            #else
            const std::lock_guard tileLock{m_tileLocks.at(gridToLinear(row, column))};
            #endif
            std::vector<Tile*> surroundingTiles;
            getSurroundingTiles(surroundingTiles, row, column);
            for (const Tile* sTile: surroundingTiles) {
                m_threadPool.detach_task([this, sTile] {
                    threadedCheckTile(sTile->getRow(), sTile->getColumn());
                });
            }
            m_threadPool.wait();
        }
    }

    void Board::threadedCheckTile(const std::uint8_t row, const std::uint8_t column) {
        #ifdef NDEBUG
        const std::unique_lock tileLock{m_tileLocks[gridToLinear(row, column)], std::try_to_lock};
        #else
        const std::unique_lock tileLock{m_tileLocks.at(gridToLinear(row, column)), std::try_to_lock};
        #endif
        if (!tileLock.owns_lock()) {
            // if another thread is running checkTile on this tile, just return
            return;
        }
        Tile& tile{atCoordinate(row, column)};
        if (tile.isChecked() || tile.isFlagged()) {
            return;
        }
        tile.becomeChecked(); {
            std::lock_guard uncheckedLock{m_uncheckedMutex};
            m_uncheckedTiles.erase(&tile);
        }
        if (tile.isMine()) {
            m_hitMine = true;
            return;
        }
        if (tile.getSurroundingMines() == 0) {
            std::vector<Tile*> surroundingTiles;
            getSurroundingTiles(surroundingTiles, row, column);
            for (const Tile* sTile: surroundingTiles) {
                m_threadPool.detach_task([this, sTile] {
                    threadedCheckTile(sTile->getRow(), sTile->getColumn());
                });
            }
        }
    }

    void Board::toggleFlag(const std::uint8_t row, const std::uint8_t column) noexcept {
        Tile& tile{atCoordinate(row, column)};
        if (tile.isChecked()) {
            return;
        }
        if (!tile.isFlagged()) {
            m_flagCount++;
        } else {
            m_flagCount--;
        }
        tile.toggleFlag();
    }

    void Board::clearSafeTiles(const std::uint8_t row, const std::uint8_t column) {
        const Tile& safeTile{atCoordinate(row, column)};
        if (!safeTile.isChecked() || safeTile.getSurroundingMines() == 0) {
            return;
        }
        std::vector<Tile*> uncheckedTiles;
        uncheckedTiles.reserve(8);
        getSurroundingTiles(uncheckedTiles, row, column);
        std::erase_if(uncheckedTiles, [](const Tile* tile) { return tile->isChecked(); });
        const auto trueUncheckedTiles = std::ranges::subrange(uncheckedTiles.begin(),
                                                              std::ranges::remove_if(
                                                                  uncheckedTiles, [](const Tile* tile) {
                                                                      return tile->isFlagged();
                                                                  }).begin());
        if (uncheckedTiles.size() - trueUncheckedTiles.size() == safeTile.getSurroundingMines()) {
            for (const Tile* tile: trueUncheckedTiles) {
                m_threadPool.detach_task([this, tile] {
                    threadedCheckTile(tile->getRow(), tile->getColumn());
                });
            }
            m_threadPool.wait();
        }
    }

    void Board::getSurroundingTiles(std::vector<Tile*>& vec, const std::uint8_t row, const std::uint8_t column) {
        for (std::int_fast8_t r{-1}; r <= 1; r++) {
            if (r + row < 0 || r + row >= m_rowAmount) {
                continue;
            }
            for (std::int_fast8_t c{-1}; c <= 1; c++) {
                if (c + column < 0 || c + column >= m_columnAmount) {
                    continue;
                }
                if (c == 0 && r == 0) {
                    continue;
                }
                vec.push_back(&atCoordinate(row + r, column + c));
            }
        }
    }

    void Board::generateMines(const std::uint8_t row, const std::uint8_t column) {
        std::vector<Tile*> possibleTiles;
        std::ranges::transform(m_board, std::back_inserter(possibleTiles), [](Tile& tile) { return &tile; });
        std::erase(possibleTiles, &atCoordinate(row, column));
        std::vector<Tile*> surroundingTiles;
        surroundingTiles.reserve(8);
        getSurroundingTiles(surroundingTiles, row, column);
        if (m_mineCount < m_rowAmount * m_columnAmount - surroundingTiles.size()) [[likely]] {
            auto contains = [](auto& range, auto value) {
                return std::ranges::find(range, value) != std::ranges::end(range);
            };
            std::erase_if(possibleTiles, [&](Tile* tile) {
                return contains(surroundingTiles, tile);
            });
        }
        #ifdef PCG_RAND
        pcg32_fast rng{pcg_extras::seed_seq_from<std::random_device>()};
        #else
        std::random_device rand;
        std::seed_seq seedSeq{rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand()};
        std::minstd_rand rng{seedSeq};
        #endif
        const std::size_t cachedSize{possibleTiles.size()};
        for (std::uint_fast16_t i{0}; i < m_mineCount; i++) {
            #ifdef PCG_RAND
            const std::size_t randIndex{rng(cachedSize - i)};
            #else
            const std::size_t randIndex{std::uniform_int_distribution<std::size_t>(0, cachedSize - (i + 1))(rng)};
            #endif
            #ifdef NDEBUG
            Tile* randTile{possibleTiles[randIndex]};
            #else
            Tile* randTile{possibleTiles.at(randIndex)};
            #endif
            m_minedTiles.insert(randTile);
            randTile->becomeMine();
            std::erase(possibleTiles, randTile);
        }
        surroundingTiles.clear();
        surroundingTiles.reserve(8 * m_mineCount); // maximum amount of tiles that could surround all the mines
        for (const Tile* tile: m_minedTiles) {
            getSurroundingTiles(surroundingTiles, tile->getRow(), tile->getColumn());
        }
        auto nonMinedTiles = surroundingTiles | std::views::filter([](const Tile* tile) {
            return !tile->isMine();
        });
        for (Tile* tile: nonMinedTiles) {
            tile->incrementSurroundingMines();
        }
    }
} // Minesweeper
