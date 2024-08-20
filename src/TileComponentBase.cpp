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
        using namespace ftxui;
        if (!m_tile.isChecked()) {
            if (m_tile.isFlagged()) {
                return text("P") | color(Color::Red) | bgcolor(Color::GrayLight);
            }
            return text(" ") | bgcolor(Color::GrayLight);
        }
        if (m_tile.isMine()) {
            return text("*") | color(Color::Black) | bgcolor(Color::GrayDark);
        }
        const uint8_t surroundingMines = m_tile.getSurroundingMines();
        if(surroundingMines == 0) [[likely]] {
            return text(" ") | bgcolor(Color::GrayDark);
        }
        Element tileRepr{text(std::to_string(surroundingMines)) | bgcolor(Color::GrayDark)};
        switch (surroundingMines) {
            [[likely]] case 1:
                tileRepr |= color(Color::Blue);
                break;
            [[likely]] case 2:
                tileRepr |= color(Color::Green);
                break;
            [[likely]] case 3:
                tileRepr |= color(Color::Red);
                break;
            case 4:
                tileRepr |= color(Color::DarkBlue);
                break;
            case 5:
                tileRepr |= color(Color::DarkRed);
                break;
            case 6:
                tileRepr |= color(Color::Cyan);
                break;
            [[unlikely]] case 7:
                tileRepr |= color(Color::Black);
                break;
            [[unlikely]] case 8:
                tileRepr |= color(Color::GrayLight);
                break;
            default: // this should never be reached since it'll be a violation of Tile's surroundingMines invariant
                UNREACHABLE();
        }
        return tileRepr;
    }

    constexpr bool TileComponentBase::Focusable() const {
        return true;
    }
} // Minesweeper
