#include "BoardComponentBase.hpp"

#include <algorithm>

#include "TileComponentBase.hpp"
#include "ftxui/dom/table.hpp"

namespace Minesweeper {
    BoardComponentBase::BoardComponentBase(const std::shared_ptr<Board>& board, ftxui::Closure exit): ComponentBase(),
        m_exit{std::move(exit)},
        m_board{board},
        m_hovered{false} {
        for (std::uint_fast8_t row{0}; row < board->getRowAmount(); row++) {
            for (std::uint_fast8_t col{0}; col < board->getColumnAmount(); col++) {
                TileComponent child{TileComponentBase::Create(board->at(row, col))};
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
        ftxui::Table table(renderElements);
        table.SelectAll().SeparatorVertical(ftxui::EMPTY);
        return table.Render();
    }

    bool BoardComponentBase::OnEvent(ftxui::Event event) {
        for (const ftxui::Component& child: children_) {
            child->OnEvent(event);
        }
        if (!m_hovered || !event.is_mouse()) {
            return false;
        }
        auto [button, motion, shift, meta, control, x, y]{event.mouse()};
        auto isHovered = [](const ftxui::Component& child) {
            if (child->ActiveChild() == nullptr) {
                return false;
            }
            const std::shared_ptr<TileComponentBase> possibleCast = std::dynamic_pointer_cast<TileComponentBase>(
                child->ActiveChild());
            return possibleCast != nullptr && possibleCast->m_hovered;
        };
        const auto it{std::ranges::find_if(std::as_const(children_), isHovered)};
        if (it == children_.end()) {
            return false;
        }
        auto [row, column] = std::static_pointer_cast<TileComponentBase>((*it)->ActiveChild())->getCoordinates();
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
        if (m_board->foundAllMines() || m_board->hitMine()) {
            m_exit();
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
