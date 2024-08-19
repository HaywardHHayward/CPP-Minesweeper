#include "Board.hpp"

#include <algorithm>
#include <cassert>
#ifdef __cpp_lib_parallel_algorithm
#define PARALLEL_ALGORITHM
#include <execution>
#endif
#include <format>
#include <random>
#include <thread>
#ifndef _MSC_VER
#include <pcg/pcg_extras.hpp>
#include <pcg/pcg_random.hpp>
#endif

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
                Tile* newTile{&m_board.emplace_back(row, col)};
                m_uncheckedTiles.insert(newTile);
            }
        }
    }

    Tile& Board::at(const std::uint8_t row, const std::uint8_t column) {
        assert(row < m_rowAmount && column < m_columnAmount);
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
        if (tile.isMine()) {
            m_hitMine = true;
        }
        m_uncheckedTiles.erase(&tile);
        if (tile.getSurroundingMines() == 0 && !tile.isMine()) {
            std::vector<Tile*> surroundingTiles;
            getSurroundingTiles(surroundingTiles, row, column);
            #ifdef __cpp_lib_jthread
            using threadType = std::jthread;
            #else
            using threadType = std::thread;
            #endif
            std::vector<threadType> threads;
            threads.reserve(surroundingTiles.size());
            for (const Tile* sTile: surroundingTiles) {
                threads.emplace_back(&Board::checkTile, this, sTile->getRow(), sTile->getColumn());
            }
            #ifndef __cpp_lib_jthread
            for (threadType& thread: threads) {
                thread.join();
            }
            #endif
        }
    }

    void Board::toggleFlag(const std::uint8_t row, const std::uint8_t column) noexcept {
        Tile& tile{at(row, column)};
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
        const Tile& safeTile{at(row, column)};
        if (!safeTile.isChecked() || safeTile.getSurroundingMines() == 0) {
            return;
        }
        std::vector<Tile*> uncheckedTiles;
        uncheckedTiles.reserve(8);
        getSurroundingTiles(uncheckedTiles, row, column);
        std::erase_if(uncheckedTiles, [](const Tile* tile) { return tile->isChecked(); });
        std::vector<Tile*> trueUncheckedTiles, flaggedSurroundingTiles;
        flaggedSurroundingTiles.reserve(uncheckedTiles.size());
        trueUncheckedTiles.reserve(uncheckedTiles.size());
        std::ranges::partition_copy(uncheckedTiles, std::back_inserter(flaggedSurroundingTiles),
                                    std::back_inserter(trueUncheckedTiles), [](const Tile* tile) {
                                        return tile->isFlagged();
                                    });
        if (flaggedSurroundingTiles.size() == safeTile.getSurroundingMines()) {
            #ifdef __cpp_lib_jthread
            using threadType = std::jthread;
            #else
            using threadType = std::thread;
            #endif
            std::vector<threadType> threads;
            threads.reserve(trueUncheckedTiles.size());
            for (const Tile* tile: trueUncheckedTiles) {
                threads.emplace_back(&Board::checkTile, this, tile->getRow(), tile->getColumn());
            }
            #ifndef __cpp_lib_jthread
            for (threadType& thread: threads) {
                thread.join();
            }
            #endif
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
                vec.push_back(&at(row + r, column + c));
            }
        }
    }

    void Board::generateMines(const std::uint8_t row, const std::uint8_t column) {
        std::vector<Tile*> surroundingTiles;
        surroundingTiles.reserve(8);
        getSurroundingTiles(surroundingTiles, row, column);
        std::vector<Tile*> possibleTiles;
        std::ranges::transform(m_board, std::back_inserter(possibleTiles), [](Tile& tile) { return &tile; });
        std::erase(possibleTiles, &at(row, column));
        const bool tooManyMines{m_mineCount >= m_rowAmount * m_columnAmount - surroundingTiles.size()};
        const bool notEnoughSpace{m_rowAmount <= 3 && m_columnAmount <= 3};
        if (!tooManyMines && !notEnoughSpace) [[likely]] {
            auto contains = [](auto& range, auto value) {
                return std::ranges::find(range, value) != std::ranges::end(range);
            };
            std::erase_if(possibleTiles, [&surroundingTiles, contains](Tile* tile) {
                return contains(surroundingTiles, tile);
            });
        }
        #ifndef _MSC_VER
        pcg32_fast rng{pcg_extras::seed_seq_from<std::random_device>()};
        #else
        std::random_device rand;
        std::seed_seq seedSeq{rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand()};
        std::minstd_rand rng{seedSeq};
        #endif
        for (std::uint_fast16_t _{0}; _ < m_mineCount; _++) {
            #ifndef _MSC_VER
            const std::size_t randIndex{rng(possibleTiles.size())};
            #else
            const std::size_t randIndex{std::uniform_int_distribution<std::size_t>(0, possibleTiles.size() - 1)(rng)};
            #endif
            Tile* randTile{possibleTiles.at(randIndex)};
            m_minedTiles.insert(randTile);
            randTile->becomeMine();
            std::erase(possibleTiles, randTile);
        }
        surroundingTiles.clear();
        surroundingTiles.reserve(8 * m_mineCount); // maximum amount of tiles that could surround all the mines
        for (const Tile* tile: m_minedTiles) {
            getSurroundingTiles(surroundingTiles, tile->getRow(), tile->getColumn());
        }
        auto isMine = [](const Tile* tile) {
            return tile->isMine();
        };
        #ifdef PARALLEL_ALGORITHM
        surroundingTiles.erase(
            std::remove_if(std::execution::par_unseq, surroundingTiles.begin(), surroundingTiles.end(), isMine),
            surroundingTiles.end());
        std::for_each(std::execution::par_unseq, surroundingTiles.begin(), surroundingTiles.end(),
                      [](Tile* tile) { tile->incrementSurroundingMines(); });
        #else
        std::erase_if(surroundingTiles, isMine);
        for (Tile* tile: surroundingTiles) {
            tile->incrementSurroundingMines();
        }
        #endif
    }
} // Minesweeper
