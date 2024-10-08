#include "BoardComponentBase.hpp"

#include <algorithm>

#include "TileComponentBase.hpp"
#include "ftxui/dom/table.hpp"

namespace Minesweeper {
    BoardComponentBase::BoardComponentBase(const std::shared_ptr<Board>& board, ftxui::Closure exit): ComponentBase(),
        m_exit{std::move(exit)},
        m_board{board},
        hovered{false} {
        for (std::uint_fast8_t row{0}; row < board->getRowAmount(); row++) {
            for (std::uint_fast8_t col{0}; col < board->getColumnAmount(); col++) {
                TileComponent child{TileComponentBase::Create(board->atCoordinate(row, col))};
                Add(Hoverable(child, &child->m_hovered));
            }
        }
    }

    ftxui::Element BoardComponentBase::Render() {
        std::vector<std::vector<ftxui::Element> > renderElements;
        renderElements.reserve(m_board->getRowAmount());
        for (std::uint_fast8_t row{0}; row < m_board->getRowAmount(); row++) {
            std::vector<ftxui::Element> rowOfElements;
            rowOfElements.reserve(m_board->getColumnAmount());
            for (std::uint_fast8_t col{0}; col < m_board->getColumnAmount(); col++) {
                rowOfElements.push_back(childAtCoords(row, col)->Render());
            }
            renderElements.push_back(std::move(rowOfElements));
        }
        ftxui::Table table(std::move(renderElements));
        return table.Render();
    }

    bool BoardComponentBase::OnEvent(ftxui::Event event) {
        for (const ftxui::Component& child: children_) {
            child->OnEvent(event);
        }
        if (m_board->foundAllMines() || m_board->hitMine()) {
            m_exit();
        }
        if (!hovered || !event.is_mouse()) {
            return false;
        }
        auto [button, motion, shift, meta, control, x, y]{event.mouse()};
        auto isHovered = [](const ftxui::Component& child) {
            if (child->ActiveChild() == nullptr) {
                return false;
            }
            const TileComponent possibleCast = std::dynamic_pointer_cast<TileComponentBase>(
                child->ActiveChild());
            return possibleCast != nullptr && possibleCast->m_hovered;
        };
        const auto hoveredTile{std::ranges::find_if(children_, isHovered)};
        if (hoveredTile == children_.end()) {
            return false;
        }
        auto [row, column] = std::static_pointer_cast<TileComponentBase>((*hoveredTile)->ActiveChild())->
            getCoordinates();
        if (motion != ftxui::Mouse::Motion::Released) {
            return true;
        }
        switch (button) {
            case ftxui::Mouse::Left:
                m_board->checkTile(row, column);
                break;
            case ftxui::Mouse::Middle:
                m_board->clearSafeTiles(row, column);
                break;
            case ftxui::Mouse::Right:
                m_board->toggleFlag(row, column);
                break;
            default:
                break;
        }
        return true;
    }

    ftxui::Component BoardComponentBase::ActiveChild() {
        return ComponentBase::ActiveChild();
    }

    void BoardComponentBase::SetActiveChild(ComponentBase* child) {
        ComponentBase::SetActiveChild(child);
    }
} // Minesweeper
