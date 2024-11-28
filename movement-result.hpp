#pragma once

#include <QtQml/qqmlregistration.h>
#include <QPointF>
#include <QList>
#include <qobjectdefs.h>


class MovementResult
{
    Q_GADGET
    QML_VALUE_TYPE(movementResult)

    Q_PROPERTY(QPointF finalDestination READ finalDestination CONSTANT FINAL)
    Q_PROPERTY(QList<QPointF> collectedGems READ collectedGems FINAL)

public:

    MovementResult() = default;
    MovementResult(const QPointF& finalDest, QList<QPointF> m_collectedGems);
    MovementResult(const MovementResult&) = default;

    QPointF finalDestination() const;
    QList<QPointF> collectedGems() const;

private:

    QPointF m_finalDestination;
    QList<QPointF> m_collectedGems;
};
