#include "movement-result.hpp"


MovementResult::MovementResult(const QPointF& finalDest,
                                QList<QPointF> m_collectedGems) :
                                m_finalDestination(finalDest),
                                m_collectedGems(std::move(m_collectedGems))
{

}

QPointF MovementResult::finalDestination() const
{
    return m_finalDestination;
}

QList<QPointF> MovementResult::collectedGems() const
{
    return m_collectedGems;
}
