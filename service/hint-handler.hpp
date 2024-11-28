#pragma once

#include "common-definitions.hpp"
#include "utility.hpp"

#include <unordered_set>


class GameStateMaintainer;


struct HintBranch
{
    Definitions::Position currentPos;
    std::unordered_set<Definitions::Position> stoppedByCells;
    std::vector<Definitions::MovementDirection> track;
};


class HintHandler
{

    struct ShortestWayToGemLoopTask
    {
        ShortestWayToGemLoopTask(HintHandler* hintHandler,
                                   std::atomic_bool& pathFound,
                                   std::atomic_bool& returnEarly,
                                   const Definitions::Position& gemPos,
                                   std::unordered_set<Definitions::Position>& stuckArea);

        std::optional<std::vector<HintBranch>> operator()(HintBranch& branch) const;

    private:

        HintHandler* m_hintHandler {};
        std::atomic_bool& m_pathFound;
        std::atomic_bool& m_returnEarly;
        const Definitions::Position& m_gemPos;
        std::unordered_set<Definitions::Position>& m_stuckArea;
    };

public:

    void hint();
    void checkMove(Definitions::MovementDirection moveDir);
    void checkUndo(const QList<QPointF>& pickedGems);
    void onGemPicked(quint32 rowIndex, quint32 columnIndex);

private:

    std::optional<Definitions::Position> nearestGem(const Definitions::Position& currentPos);

    template<typename Container>
    Definitions::Position nearestGemCore(const Definitions::Position& currentPos,
                                                         const Container& candidateGems);

    bool shortestWayToGem(const Definitions::Position& currentPos, const Definitions::Position& gemPos);
    void invalidateHint();


    static inline constexpr uint32_t kMaxBranchesVecSize = 500000;
    static inline constexpr uint8_t kMaxHintRetryCounts = 20;

    bool m_activeHint{false};
    std::vector<Definitions::MovementDirection> m_hintTrace{};
    Definitions::Position m_hintTargetGem;
    std::vector<Definitions::MovementDirection>::const_iterator m_hintNextExpectedDir;
};


template<typename ContainerType>
Definitions::Position
HintHandler::nearestGemCore(const Definitions::Position& currentPos,
                             const ContainerType& candidateGems)
{
    double minDistance{std::numeric_limits<double>::infinity()};

    typename  ContainerType::const_iterator resultIt;

    for(auto it {candidateGems.cbegin()}; it != candidateGems.cend(); ++it)
        if(auto dist {InertiaUtility::distance(currentPos, *it)}; dist < minDistance)
        {
            minDistance = dist;
            resultIt = it;
        }


    return *resultIt;
}
