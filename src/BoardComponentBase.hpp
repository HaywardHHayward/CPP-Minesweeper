#ifndef BOARDCOMPONENT_HPP
#define BOARDCOMPONENT_HPP

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include "TileComponentBase.hpp"


namespace Minesweeper {
    class Board;
    class TileComponentBase;

    class BoardComponentBase final : public ftxui::ComponentBase {
        Board& m_board;
        std::vector<std::vector<ftxui::Element> > m_boardElements;
        std::vector<TileComponent> m_tileComponents;

    public:
        static std::shared_ptr<BoardComponentBase> Create(Board& board);
        explicit BoardComponentBase(Board& board);
        bool hovered;
        ftxui::Element Render() override;
        bool OnEvent(ftxui::Event) override;
        ftxui::Component ActiveChild() override;
        bool Focusable() const override;
        void SetActiveChild(ComponentBase* child) override;
        ftxui::Component& childAtCoords(size_t r, size_t c);
    };

    using BoardComponent = std::shared_ptr<BoardComponentBase>;
} // Minesweeper

#endif //BOARDCOMPONENT_HPP
