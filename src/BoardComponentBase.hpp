#ifndef BOARDCOMPONENT_HPP
#define BOARDCOMPONENT_HPP

#include <ftxui/component/component.hpp>
#include <ftxui/component/task.hpp>

#include "Board.hpp"


namespace Minesweeper {
    class BoardComponentBase final : public ftxui::ComponentBase {
        ftxui::Closure m_exit;
        Board& m_board;
        bool m_hovered;

    public:
        static std::shared_ptr<BoardComponentBase> Create(Board& board, ftxui::Closure exit);
        explicit BoardComponentBase(Board& board, ftxui::Closure exit);
        [[nodiscard]] bool* hovered();
        ftxui::Element Render() override;
        bool OnEvent(ftxui::Event) override;
        ftxui::Component ActiveChild() override;
        [[nodiscard]] constexpr bool Focusable() const override;
        void SetActiveChild(ComponentBase* child) override;
        ftxui::Component& childAtCoords(size_t r, size_t c);
    };

    inline std::shared_ptr<BoardComponentBase> BoardComponentBase::Create(Board& board, ftxui::Closure exit) {
        return std::make_shared<BoardComponentBase>(board, std::move(exit));
    }

    inline ftxui::Component& BoardComponentBase::childAtCoords(const size_t r, const size_t c) {
        return ChildAt(r * m_board.getColumnAmount() + c);
    }

    inline bool* BoardComponentBase::hovered() { return &m_hovered; }

    using BoardComponent = std::shared_ptr<BoardComponentBase>;
} // Minesweeper

#endif //BOARDCOMPONENT_HPP
