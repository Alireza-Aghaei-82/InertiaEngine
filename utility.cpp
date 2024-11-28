#include "utility.hpp"
#include "constants.hpp"

#include <random>


namespace InertiaUtility
{
    Definitions::MovementDirection oppositeDirection(Definitions::MovementDirection direction)
    {
        switch(direction)
        {
        case Definitions::MovementDirection::Up:
            return Definitions::MovementDirection::Down;

        case Definitions::MovementDirection::Down:
            return Definitions::MovementDirection::Up;

        case Definitions::MovementDirection::Right:
            return Definitions::MovementDirection::Left;

        case Definitions::MovementDirection::Left:
            return Definitions::MovementDirection::Right;

        case Definitions::MovementDirection::UpRight:
            return Definitions::MovementDirection::DownLeft;

        case Definitions::MovementDirection::DownRight:
            return Definitions::MovementDirection::UpLeft;

        case Definitions::MovementDirection::DownLeft:
            return Definitions::MovementDirection::UpRight;

        case Definitions::MovementDirection::UpLeft:
            return Definitions::MovementDirection::DownRight;
        }
    }

    Definitions::MovementDirection movementDirection(const Definitions::Position& ballPos,
                                                      quint32 targetRowIndex,
                                                      quint32 targetColumnIndex)
    {
        if(ballPos.columnIndex == targetColumnIndex)
        {
            if(ballPos.rowIndex > targetRowIndex)
                return Definitions::MovementDirection::Up;

            else
                return Definitions::MovementDirection::Down;
        }

        else if(ballPos.rowIndex == targetRowIndex)
        {
            if(ballPos.columnIndex < targetColumnIndex)
                return Definitions::MovementDirection::Right;

            else
                return Definitions::MovementDirection::Left;
        }

        else if(targetRowIndex - ballPos.rowIndex == targetColumnIndex - ballPos.columnIndex)
        {
            if(targetRowIndex > ballPos.rowIndex)
                return Definitions::DownRight;

            else
                return Definitions::UpLeft;
        }

        else
        {
            if(targetColumnIndex > ballPos.columnIndex)
                return Definitions::MovementDirection::UpRight;

            else
                return Definitions::DownLeft;
        }
    }

    double distance(const Definitions::Position& pos1, const Definitions::Position& pos2)
    {
        return std::sqrt(std::pow(static_cast<qint32>(pos2.rowIndex) - static_cast<qint32>(pos1.rowIndex), 2) +
                         std::pow(static_cast<qint32>(pos2.columnIndex) - static_cast<qint32>(pos1.columnIndex), 2));
    }

    quint32 randomRowIndex(quint32 rowsCounts)
    {
        std::random_device rd;
        std::mt19937 generator(rd());
        return generator() % rowsCounts;
    }

    quint32 randomColumnIndex(quint32 columnsCounts)
    {
        std::random_device rd;
        std::mt19937 generator(rd());
        return generator() % columnsCounts;
    }

    std::vector<Definitions::MovementDirection> shuffledDirections()
    {
        std::random_device rd;
        std::mt19937 generator(rd());
        auto shuffledDirs {Constants::kAllDirections};

        std::shuffle(shuffledDirs.begin(), shuffledDirs.end(), generator);
        return shuffledDirs;
    }

    double angle(double slope, double p1x, double p1y, double p2x, double p2y)
    {
        if(p1x == p2x && p1y == p2y)
            return 0;

        auto angle {180 * std::atan(slope) / std::numbers::pi};

        if(!slope)
        {
            if(p2x >= p1x)
                return 0;

            else
                return 180;
        }

        if(slope > 0 && std::isfinite(slope))
        {
            if(p2x >= p1x)
                return angle;

            else
                return 180 + angle;
        }

        if(std::isinf(slope))
        {
            if(p1y >= p2y)
                return 90;

            else
                return 270;
        }

        if(slope < 0)
        {
            if(p2x <= p1x)
                return 180 + angle;

            else
                return 360 + angle;
        }
    }

    std::vector<Definitions::MovementDirection> orderedDirections(const Definitions::Position& sourcePos,
                                                                   const Definitions::Position& destPos)
    {
        double p1x = sourcePos.columnIndex;
        double p1y = sourcePos.rowIndex;
        double p2x = destPos.columnIndex;
        double p2y = destPos.rowIndex;

        double slope {(p1y - p2y) / (p2x - p1x)};
        const double moveAngle {angle(slope, p1x, p1y, p2x, p2y)};

        auto ordering {[moveAngle](Definitions::MovementDirection dir1, Definitions::MovementDirection dir2)
            {
                return std::abs(moveAngle - static_cast<double>(dir1)) <= std::abs(moveAngle - static_cast<double>(dir2));
            }};

        auto sortedDirs {Constants::kAllDirections};
        std::sort(sortedDirs.begin(), sortedDirs.end(), ordering);

        return sortedDirs;
    }

    Definitions::Hint directionToHint(Definitions::MovementDirection direction)
    {
        switch (direction)
        {
        case Definitions::MovementDirection::Up:
            return Definitions::Hint::UpHint;

        case Definitions::MovementDirection::Down:
            return Definitions::Hint::DownHint;

        case Definitions::MovementDirection::Right:
            return Definitions::Hint::RightHint;

        case Definitions::MovementDirection::Left:
            return Definitions::Hint::LeftHint;

        case Definitions::MovementDirection::UpRight:
            return Definitions::Hint::UpRightHint;

        case Definitions::MovementDirection::DownRight:
            return Definitions::Hint::DownRightHint;

        case Definitions::MovementDirection::DownLeft:
            return Definitions::Hint::DownLeftHint;

        case Definitions::MovementDirection::UpLeft:
            return Definitions::Hint::UpLeftHint;
        }
    }

    bool isValidDirection(Definitions::MovementDirection direction)
    {
        auto intValue {static_cast<int64_t>(direction)};

        return (!intValue ||
                intValue == 45 ||
                intValue == 90 ||
                intValue == 135 ||
                intValue == 180 ||
                intValue == 225 ||
                intValue == 270 ||
                intValue == 315);
    }
}
