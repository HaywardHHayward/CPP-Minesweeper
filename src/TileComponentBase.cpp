#include "TileComponentBase.hpp"

#include "Tile.hpp"

#if defined(_MSC_VER)
#define UNREACHABLE() __assume(0)
#elif defined(__GNUC__)
#define UNREACHABLE() __builtin_unreachable()
#else
#define UNREACHABLE() 
#endif

namespace Minesweeper {
    TileComponentBase::TileComponentBase(Tile& tile) noexcept: ComponentBase(), m_tile(tile),
                                                      m_coords{std::make_pair(tile.getRow(), tile.getColumn())},
                                                      m_hovered{false} { }

    ftxui::Element TileComponentBase::Render() {
        ftxui::Element tileRepr;
        if (!m_tile.isChecked()) {
            if (m_tile.isFlagged()) {
                tileRepr = ftxui::text("P") | color(ftxui::Color::Red);
            } else {
                tileRepr = ftxui::text(" ") | color(ftxui::Color());
            }
            tileRepr |= bgcolor(ftxui::Color::GrayLight);
            return tileRepr;
        }
        if (m_tile.isMine()) {
            tileRepr = ftxui::text("X") | color(ftxui::Color::Black);
        } else {
            if (m_tile.getSurroundingMines() == 0) {
                tileRepr = ftxui::text(" ");
            } else {
                tileRepr = ftxui::text(std::to_string(m_tile.getSurroundingMines()));
            }
            switch (m_tile.getSurroundingMines()) {
                case 0:
                    tileRepr |= color(ftxui::Color());
                    break;
                case 1:
                    tileRepr |= color(ftxui::Color::Blue);
                    break;
                case 2:
                    tileRepr |= color(ftxui::Color::Green);
                    break;
                case 3:
                    tileRepr |= color(ftxui::Color::Red);
                    break;
                case 4:
                    tileRepr |= color(ftxui::Color::DarkBlue);
                    break;
                case 5:
                    tileRepr |= color(ftxui::Color::DarkRed);
                    break;
                case 6:
                    tileRepr |= color(ftxui::Color::Cyan);
                    break;
                case 7:
                    tileRepr |= color(ftxui::Color::Black);
                    break;
                case 8:
                    tileRepr |= color(ftxui::Color::GrayLight);
                    break;
                default: // this should never be reached since it'll be a violation of Tile's surroundingMines invariant
                    UNREACHABLE();
            }
        }
        tileRepr |= bgcolor(ftxui::Color::GrayDark);
        return tileRepr;
    }

    constexpr bool TileComponentBase::Focusable() const {
        return true;
    }
} // Minesweeper
