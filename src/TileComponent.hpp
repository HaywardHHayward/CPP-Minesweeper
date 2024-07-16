#ifndef TILECOMPONENT_HPP
#define TILECOMPONENT_HPP

#include <ftxui/component/component.hpp>

#include "Tile.hpp"

namespace Minesweeper {
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
