#ifndef BOARDCOMPONENT_HPP
#define BOARDCOMPONENT_HPP

#include <ftxui/component/component.hpp>
#include <ftxui/component/task.hpp>

#include "Board.hpp"


namespace Minesweeper {
    class BoardComponentBase;
    using BoardComponent = std::shared_ptr<BoardComponentBase>;

    class BoardComponentBase final : public ftxui::ComponentBase {
        ftxui::Closure m_exit;
        std::shared_ptr<Board> m_board;

    public:
        bool hovered;
        static BoardComponent Create(const std::shared_ptr<Board>& board, ftxui::Closure exit);
        explicit BoardComponentBase(const std::shared_ptr<Board>& board, ftxui::Closure exit);
        ftxui::Element Render() override;
        bool OnEvent(ftxui::Event) override;
        ftxui::Component ActiveChild() override;
        [[nodiscard]] constexpr bool Focusable() const override;
        void SetActiveChild(ComponentBase* child) override;
        ftxui::Component& childAtCoords(size_t r, size_t c);
    };

    constexpr bool BoardComponentBase::Focusable() const {
        return true;
    }

    inline BoardComponent BoardComponentBase::Create(
        const std::shared_ptr<Board>& board, ftxui::Closure exit) {
        return std::make_shared<BoardComponentBase>(board, std::move(exit));
    }

    inline ftxui::Component& BoardComponentBase::childAtCoords(const size_t r, const size_t c) {
        return ChildAt(r * m_board->getColumnAmount() + c);
    }
} // Minesweeper

#endif //BOARDCOMPONENT_HPP
