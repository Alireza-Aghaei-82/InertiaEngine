#pragma once

#include "movement-result.hpp"
#include "common-definitions.hpp"
#include "hint-handler.hpp"


class MoveHandler
{
public:

    MoveHandler();

    MovementResult moveBall(Definitions::MovementDirection direction);
    void undo(QPointF preMovePos, QList<QPointF> pickedGems);
    void hint();

private:

    void onGemPicked(quint32 rowIndex, quint32 columnIndex);

    std::unique_ptr<HintHandler> m_hintHandler{};
};
