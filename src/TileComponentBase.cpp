#include "TileComponentBase.hpp"

#include "Tile.hpp"

#if defined(_MSVC_VER) && !defined(__clang__)
#define UNREACHABLE() __assume(false)
#elif defined(__GNUC__) || defined(__clang__)
#define UNREACHABLE() __builtin_unreachable()
#else
[[noreturn]] inline void unreachable() { }
#define UNREACHABLE() unreachable()
#endif

namespace Minesweeper {
    TileComponentBase::TileComponentBase(Tile& tile) noexcept: ComponentBase(),
                                                               m_tile{tile},
                                                               m_coords(std::make_pair(
                                                                   tile.getRow(), tile.getColumn())),
                                                               m_hovered{false} { }

    ftxui::Element TileComponentBase::Render() {
        ftxui::Element tileRepr;
        if (!m_tile.isChecked()) {
            tileRepr = m_tile.isFlagged()
                           ? ftxui::text("P") | color(ftxui::Color::Red)
                           : ftxui::text(" ") | color(ftxui::Color());
            tileRepr |= bgcolor(ftxui::Color::GrayLight);
            return tileRepr;
        }
        if (m_tile.isMine()) {
            tileRepr = ftxui::text("*") | color(ftxui::Color::Black);
        } else {
            tileRepr = ftxui::text(m_tile.getSurroundingMines() == 0
                                       ? " "
                                       : std::to_string(m_tile.getSurroundingMines()));
            switch (m_tile.getSurroundingMines()) {
                [[likely]] case 0:
                    tileRepr |= color(ftxui::Color());
                    break;
                [[likely]] case 1:
                    tileRepr |= color(ftxui::Color::Blue);
                    break;
                [[likely]] case 2:
                    tileRepr |= color(ftxui::Color::Green);
                    break;
                [[likely]] case 3:
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
                [[unlikely]] case 7:
                    tileRepr |= color(ftxui::Color::Black);
                    break;
                [[unlikely]] case 8:
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
