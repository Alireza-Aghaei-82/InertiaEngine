#pragma once

#include "common-definitions.hpp"

#include <vector>


namespace Constants
{
    inline constexpr float kCellsObstaclesRatio = 0.2;
    inline constexpr float kCellsGemsRatio = 0.2;
    inline constexpr float kCellsStopsRoughRatio = 0.2;

    inline const std::vector<Definitions::MovementDirection> kAllDirections
        {
            Definitions::MovementDirection::Up,
            Definitions::MovementDirection::Down,
            Definitions::MovementDirection::Right,
            Definitions::MovementDirection::Left,
            Definitions::MovementDirection::UpRight,
            Definitions::MovementDirection::DownRight,
            Definitions::MovementDirection::DownLeft,
            Definitions::MovementDirection::UpLeft
        };
}
