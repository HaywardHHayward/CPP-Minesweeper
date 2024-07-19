#ifndef TILECOMPONENT_HPP
#define TILECOMPONENT_HPP

#include <ftxui/component/component.hpp>


namespace Minesweeper {
    class Tile;

    class TileComponent final : public ftxui::ComponentBase {
        Tile& m_tile;

    public:
        explicit TileComponent(Tile& tile);
        ftxui::Element Render() override;
        bool OnEvent(ftxui::Event) override;
        [[nodiscard]] bool Focusable() const override;
    };
} // Minesweeper

#endif //TILECOMPONENT_HPP
