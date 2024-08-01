#ifndef BOARDCOMPONENT_HPP
#define BOARDCOMPONENT_HPP

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include "Board.hpp"
#include "TileComponentBase.hpp"


namespace Minesweeper {

    class BoardComponentBase final : public ftxui::ComponentBase {
        Board& m_board;

    public:
        static std::shared_ptr<BoardComponentBase> Create(Board& board);
        explicit BoardComponentBase(Board& board);
        bool hovered;
        ftxui::Element Render() override;
        bool OnEvent(ftxui::Event) override;
        ftxui::Component ActiveChild() override;
        [[nodiscard]] bool Focusable() const override;
        void SetActiveChild(ComponentBase* child) override;
        ftxui::Component& childAtCoords(size_t r, size_t c);
    };

    inline std::shared_ptr<BoardComponentBase> BoardComponentBase::Create(Board& board) {
        return std::make_shared<BoardComponentBase>(board);
    }

    inline ftxui::Component& BoardComponentBase::childAtCoords(const size_t r, const size_t c) {
        return ChildAt(r * m_board.getColumnAmount() + c);
    }

    using BoardComponent = std::shared_ptr<BoardComponentBase>;
} // Minesweeper

#endif //BOARDCOMPONENT_HPP
