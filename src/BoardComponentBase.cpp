#include "BoardComponentBase.hpp"

#include <algorithm>
#include <ftxui/component/component.hpp>

#include "Tile.hpp"
#include "TileComponentBase.hpp"
#include "ftxui/dom/table.hpp"

namespace Minesweeper {
    BoardComponentBase::BoardComponentBase(Board& board): ComponentBase(), m_board{board}, m_hovered{false} {
        for (int row = 0; row < board.getRowAmount(); ++row) {
            for (int col = 0; col < board.getColumnAmount(); ++col) {
                TileComponent child = TileComponentBase::Create(board.at(row, col));
                Add(Hoverable(child, &child->m_hovered));
            }
        }
    }


    ftxui::Element BoardComponentBase::Render() {
        std::vector<std::vector<ftxui::Element> > renderElements;
        renderElements.reserve(m_board.getRowAmount());
        for (int row = 0; row < m_board.getRowAmount(); ++row) {
            std::vector<ftxui::Element> rowOfElements;
            rowOfElements.reserve(m_board.getColumnAmount());
            for (int col = 0; col < m_board.getColumnAmount(); ++col) {
                rowOfElements.push_back(childAtCoords(row, col)->Render());
            }
            renderElements.push_back(std::move(rowOfElements));
        }
        ftxui::Table table(renderElements);
        table.SelectAll().SeparatorVertical(ftxui::EMPTY);
        table.SelectAll().Border(ftxui::LIGHT);
        return table.Render();
    }

    bool BoardComponentBase::OnEvent(ftxui::Event event) {
        if (event.is_mouse()) {
            auto [button, motion, shift, meta, control, x, y] = event.mouse();
            if (!m_hovered) {
                return true;
            }
            std::ranges::for_each(std::as_const(children_),
                                  [&event](const ftxui::Component& child) { child->OnEvent(event); });
            std::optional<std::pair<std::uint8_t, std::uint8_t> > possibleCoordinates;
            if (const auto it = std::ranges::find_if(std::as_const(children_),
                                                     [](const ftxui::Component& child) {
                                                         return std::static_pointer_cast<
                                                                     TileComponentBase>(child->ActiveChild())->
                                                                 m_hovered;
                                                     }); it != children_.end()) {
                possibleCoordinates = std::static_pointer_cast<TileComponentBase>((*it)->ChildAt(0))->getCoordinates();
            }
            if (!possibleCoordinates.has_value()) {
                return true;
            }
            auto [row, column] = possibleCoordinates.value();
            if (motion == ftxui::Mouse::Motion::Released) {
                switch (button) {
                    case ftxui::Mouse::Left:
                        m_board.checkTile(row, column);
                        break;
                    case ftxui::Mouse::Middle:
                        m_board.clearSafeTiles(row, column);
                        break;
                    case ftxui::Mouse::Right:
                        m_board.toggleFlag(row, column);
                        break;
                    default:
                        break;
                }
            }
        }
        return true;
    }

    ftxui::Component BoardComponentBase::ActiveChild() {
        return ComponentBase::ActiveChild();
    }

    constexpr bool BoardComponentBase::Focusable() const {
        return true;
    }

    void BoardComponentBase::SetActiveChild(ComponentBase* child) {
        ComponentBase::SetActiveChild(child);
    }
} // Minesweeper
