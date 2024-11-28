#pragma once

#include "common-definitions.hpp"


namespace InertiaUtility
{
    Definitions::MovementDirection oppositeDirection(Definitions::MovementDirection direction);
    Definitions::MovementDirection movementDirection(const Definitions::Position& ballPos,
                                                      quint32 targetRowIndex,
                                                      quint32 targetColumnIndex);

    double distance(const Definitions::Position& pos1, const Definitions::Position& pos2);
    quint32 randomRowIndex(quint32 rowsCounts);
    quint32 randomColumnIndex(quint32 columnsCounts);

    std::vector<Definitions::MovementDirection> shuffledDirections();

    std::vector<Definitions::MovementDirection> orderedDirections(const Definitions::Position& sourcePos,
                                                                   const Definitions::Position& destPos);

    double angle(double slope, double p1x, double p1y, double p2x, double p2y);

    Definitions::Hint directionToHint(Definitions::MovementDirection direction);
    bool isValidDirection(Definitions::MovementDirection direction);
};
