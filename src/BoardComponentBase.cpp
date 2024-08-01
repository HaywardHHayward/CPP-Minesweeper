#include "BoardComponentBase.hpp"

#include <algorithm>
#include <ftxui/component/component.hpp>

#include "Board.hpp"
#include "Tile.hpp"
#include "TileComponentBase.hpp"
#include "ftxui/dom/table.hpp"

namespace Minesweeper {
    std::shared_ptr<BoardComponentBase> BoardComponentBase::Create(Board& board) {
        return std::make_shared<BoardComponentBase>(board);
    }

    BoardComponentBase::BoardComponentBase(Board& board): ComponentBase(), m_board{board}, hovered{false} {
        for (int row = 0; row < board.getRowAmount(); ++row) {
            std::vector<ftxui::Element> rowOfElements;
            for (int col = 0; col < board.getColumnAmount(); ++col) {
                TileComponent child = TileComponentBase::Create(board.at(row, col));
                Add(Hoverable(child, &child->hovered));
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
            if (!hovered) {
                return true;
            }
            std::optional<std::pair<std::uint8_t, std::uint8_t> > possibleCoordinates;
            std::ranges::for_each(std::as_const(children_),
                                  [&event](const ftxui::Component& child) { child->OnEvent(event); });
            if (const auto it = std::ranges::find_if(children_,
                                                     [](const ftxui::Component& child) {
                                                         return std::static_pointer_cast<
                                                                     TileComponentBase>(child->ChildAt(0))->
                                                                 hovered;
                                                     }); it != children_.end()) {
                possibleCoordinates = std::static_pointer_cast<TileComponentBase>((*it)->ChildAt(0))->getCoordinates();
            }
            if (!possibleCoordinates.has_value()) {
                return true;
            }
            auto [fst, snd] = possibleCoordinates.value();
            if (button == ftxui::Mouse::Button::Left && motion == ftxui::Mouse::Motion::Released) {
                m_board.checkTile(fst, snd);
            }
            if (button == ftxui::Mouse::Button::Right && motion == ftxui::Mouse::Motion::Released) {
                m_board.toggleFlag(fst, snd);
            }
        }
        return true;
    }

    ftxui::Component BoardComponentBase::ActiveChild() {
        return ComponentBase::ActiveChild();
    }

    bool BoardComponentBase::Focusable() const {
        return ComponentBase::Focusable();
    }

    void BoardComponentBase::SetActiveChild(ComponentBase* child) {
        ComponentBase::SetActiveChild(child);
    }

    ftxui::Component& BoardComponentBase::childAtCoords(const size_t r, const size_t c) {
        return ChildAt(r * m_board.getColumnAmount() + c);
    }
} // Minesweeper
