#ifndef TILECOMPONENT_HPP
#define TILECOMPONENT_HPP

#include <ftxui/component/component.hpp>

namespace Minesweeper {
    class Tile;
    class TileComponentBase;
    using TileComponent = std::shared_ptr<TileComponentBase>;

    class TileComponentBase final : public ftxui::ComponentBase {
        friend class BoardComponentBase;
        Tile& m_tile;
        const std::pair<std::uint8_t, std::uint8_t> m_coords;
        bool m_hovered;

    public:
        explicit TileComponentBase(Tile& tile) noexcept;
        static TileComponent Create(Tile& tile);
        [[nodiscard]] std::pair<std::uint8_t, std::uint8_t> getCoordinates() const noexcept;
        ftxui::Element Render() override;
        [[nodiscard]] constexpr bool Focusable() const override;
    };

    inline std::pair<uint8_t, uint8_t> TileComponentBase::getCoordinates() const noexcept {
        return m_coords;
    }

    inline TileComponent TileComponentBase::Create(Tile& tile) {
        return std::make_shared<TileComponentBase>(tile);
    }
} // Minesweeper

#endif //TILECOMPONENT_HPP
