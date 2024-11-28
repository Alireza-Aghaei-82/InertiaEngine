#pragma once

#include <QObject>


namespace Definitions
{

Q_NAMESPACE

    struct Position
    {
        quint32 rowIndex{};
        quint32 columnIndex{};

        auto operator<=>(const Position& other) const = default;
    };

    enum CellType
    {
        Clear,
        Waiting,
        Wall,
        Stop,
        Gem,
        Mine,
        Exploded
    };

    Q_ENUM_NS(CellType)

    enum MovementDirection
    {
        Right = 0,
        UpRight = 45,
        Up = 90,
        UpLeft = 135,
        Left = 180,
        DownLeft = 225,
        Down = 270,
        DownRight = 315,
        InvalidDirection = -1
    };

    Q_ENUM_NS(MovementDirection)


    enum Hint
    {
        UpHint,
        DownHint,
        RightHint,
        LeftHint,
        UpRightHint,
        DownRightHint,
        DownLeftHint,
        UpLeftHint,
        NoHint,
    };

    Q_ENUM_NS(Hint)
}

    namespace std
    {
        template<>
        class hash<Definitions::Position>
        {
        public:
            std::size_t operator()(const Definitions::Position& pos) const
            {
                return std::hash<quint32>{}(pos.rowIndex) + std::hash<quint32>{}(pos.columnIndex);
            }
        };
    }

#include <QtQml/qqmlregistration.h>

    class InertiaDefinitions : public QObject
    {
        Q_OBJECT
        QML_ELEMENT
        QML_EXTENDED_NAMESPACE(Definitions)
    };

    Q_DECLARE_METATYPE(Definitions::CellType)
    Q_DECLARE_METATYPE(Definitions::MovementDirection)
    Q_DECLARE_METATYPE(Definitions::Hint)
