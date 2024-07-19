#include "TileComponent.hpp"

#include "Tile.hpp"

#if defined(_MSC_VER)
#define UNREACHABLE() __assume(0)
#elif defined(__GNUC__)
#define UNREACHABLE() __builtin_unreachable()
#else
#define UNREACHABLE() 
#endif

namespace Minesweeper {
    TileComponent::TileComponent(Tile& tile): m_tile(tile) { }

    ftxui::Element TileComponent::Render() {
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
            switch (m_tile.getSurroundingMines()) {
                case 0:
                    tileRepr = ftxui::text(" ") | color(ftxui::Color());
                    break;
                case 1:
                    tileRepr = ftxui::text("1") | color(ftxui::Color::Blue);
                    break;
                case 2:
                    tileRepr = ftxui::text("2") | color(ftxui::Color::Green);
                    break;
                case 3:
                    tileRepr = ftxui::text("3") | color(ftxui::Color::Red);
                    break;
                case 4:
                    tileRepr = ftxui::text("4") | color(ftxui::Color::DarkBlue);
                    break;
                case 5:
                    tileRepr = ftxui::text("5") | color(ftxui::Color::DarkRed);
                    break;
                case 6:
                    tileRepr = ftxui::text("6") | color(ftxui::Color::Cyan);
                    break;
                case 7:
                    tileRepr = ftxui::text("7") | color(ftxui::Color::Black);
                    break;
                case 8:
                    tileRepr = ftxui::text("8") | color(ftxui::Color::GrayLight);
                    break;
                default: // this should never be reached since it'll be a violation of Tile's surroundingMines invariant
                    UNREACHABLE();
            }
        }
        tileRepr |= bgcolor(ftxui::Color::GrayDark);
        return tileRepr;
    }

    bool TileComponent::OnEvent(ftxui::Event event) {
        return ComponentBase::OnEvent(event);
    }

    bool TileComponent::Focusable() const {
        return true;
    }
} // Minesweeper
