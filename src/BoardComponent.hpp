#ifndef BOARDCOMPONENT_HPP
#define BOARDCOMPONENT_HPP

#include <ftxui/component/component.hpp>

#include <ftxui/dom/elements.hpp>

namespace Minesweeper {
    class Board;

    class BoardComponent final : public ftxui::ComponentBase {
        Board& m_board;
        std::vector<std::vector<ftxui::Element>> m_boardElements;

    public:
        explicit BoardComponent(Board& board);

        ftxui::Element Render() override;
        bool OnEvent(ftxui::Event) override;
        ftxui::Component ActiveChild() override;
        bool Focusable() const override;
        void SetActiveChild(ComponentBase* child) override;
        ftxui::Component& childAtCoords(size_t r, size_t c);
    };
} // Minesweeper

#endif //BOARDCOMPONENT_HPP
