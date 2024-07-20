#include "BoardComponent.hpp"

#include "Board.hpp"
#include "ftxui/dom/table.hpp"
#include "TileComponent.hpp"

namespace Minesweeper {
    BoardComponent::BoardComponent(Board& board): ComponentBase(), m_board(board) {
        for (int row = 0; row < board.getRowAmount(); ++row) {
            std::vector<ftxui::Element> rowOfElements;
            for (int col = 0; col < board.getColumnAmount(); ++col) {
                std::shared_ptr<TileComponent> child = std::make_shared<TileComponent>(board.at(row, col));
                Add(child);
                rowOfElements.push_back(child->Render());
            }
            m_boardElements.push_back(std::move(rowOfElements));
        }
    }


    ftxui::Element BoardComponent::Render() {
        const std::vector<std::vector<ftxui::Element> > renderElements = m_boardElements;
        ftxui::Table table(m_boardElements);
        table.SelectAll().SeparatorVertical(ftxui::EMPTY);
        table.SelectAll().Border(ftxui::LIGHT);
        return table.Render();
    }

    bool BoardComponent::OnEvent(ftxui::Event event) {
        return ComponentBase::OnEvent(event);
    }

    ftxui::Component BoardComponent::ActiveChild() {
        return ComponentBase::ActiveChild();
    }

    bool BoardComponent::Focusable() const {
        return ComponentBase::Focusable();
    }

    void BoardComponent::SetActiveChild(ComponentBase* child) {
        ComponentBase::SetActiveChild(child);
    }

    ftxui::Component& BoardComponent::childAtCoords(const size_t r, const size_t c) {
        return ChildAt(r * m_board.getColumnAmount() + c);
    }
} // Minesweeper
