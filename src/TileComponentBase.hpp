#ifndef TILECOMPONENT_HPP
#define TILECOMPONENT_HPP

#include <ftxui/component/component.hpp>

namespace Minesweeper {
    class Tile;

    class TileComponentBase final : public ftxui::ComponentBase {
        Tile& m_tile;
        const std::pair<std::uint8_t, std::uint8_t> m_coords;

    public:
        static std::shared_ptr<TileComponentBase> Create(Tile& tile);
        bool hovered;
        [[nodiscard]] std::pair<std::uint8_t, std::uint8_t> getCoordinates() const;
        explicit TileComponentBase(Tile& tile);
        ftxui::Element Render() override;
        [[nodiscard]] bool Focusable() const override;
    };

    inline std::pair<uint8_t, uint8_t> TileComponentBase::getCoordinates() const {
        return m_coords;
    }

    inline std::shared_ptr<TileComponentBase> TileComponentBase::Create(Tile& tile) {
        return std::make_shared<TileComponentBase>(tile);
    }

    using TileComponent = std::shared_ptr<TileComponentBase>;
} // Minesweeper

#endif //TILECOMPONENT_HPP
