#include "hint-handler.hpp"
#include "state-wrapper.hpp"
#include "utility.hpp"

#include <execution>


void HintHandler::hint()
{
    if(m_activeHint)
    {
        if(auto& gameStart {StateWrapper::instance().state()->onGameStart()})
        {
            gameStart = false;
            invalidateHint();
        }

        else
        {
            if(InertiaUtility::isValidDirection(*m_hintNextExpectedDir))
            {
                StateWrapper::instance().state()->showHint(*m_hintNextExpectedDir);

                if(m_hintNextExpectedDir == m_hintTrace.end())
                    m_activeHint = false;

                return;
            }

            else
                invalidateHint();
        }
    }

    const auto nearestGemPos {nearestGem(StateWrapper::instance().state()->ballPos())};

    if(nearestGemPos)
    {
        m_hintTargetGem = nearestGemPos.value();
        shortestWayToGem(StateWrapper::instance().state()->ballPos(), m_hintTargetGem);

        if(m_activeHint)
        {
            m_hintNextExpectedDir = m_hintTrace.cbegin();
            StateWrapper::instance().state()->showHint(*m_hintNextExpectedDir);
        }
    }
}

std::optional<Definitions::Position> HintHandler::nearestGem(const Definitions::Position& currentPos)
{
    auto& gemsPositions {StateWrapper::instance().state()->gemsPositions()};

    if(!gemsPositions.size())
        return {};

    if(StateWrapper::instance().state()->canEnterStuckArea())
        return nearestGemCore(currentPos, gemsPositions);

    else
        return nearestGemCore(currentPos, StateWrapper::instance().state()->hintCandidateGems());
}

bool HintHandler::shortestWayToGem(const Definitions::Position& currentPos,
                                    const Definitions::Position& gemPos)
{
    static uint8_t triesCout {};

    if(triesCout++ == kMaxHintRetryCounts)
    {
        triesCout = 0;
        return false;
    }

    if(currentPos == gemPos)
    {
        triesCout = 0;
        return false;
    }

    std::vector<HintBranch> branches {{currentPos, {}}};
    auto stuckArea {StateWrapper::instance().state()->stuckArea()};
    std::atomic_bool pathFound{false};
    std::atomic_bool returnEarly{false};

    ShortestWayToGemLoopTask loopTask{this,
                                      pathFound,
                                      returnEarly,
                                      gemPos,
                                      stuckArea};

    while(const auto branchesCount {branches.size()})
    {
        if(branchesCount > kMaxBranchesVecSize)
            return shortestWayToGem(currentPos, gemPos);

        std::vector<std::optional<std::vector<HintBranch>>> newBranchesVec(branches.size());

        std::transform(std::execution::par_unseq,
                       branches.begin(),
                       branches.end(),
                       newBranchesVec.begin(),
                       loopTask);

        if(pathFound)
        {
            triesCout = 0;
            return true;
        }

        branches.erase(branches.begin(), branches.begin() + branchesCount);

        std::for_each(newBranchesVec.begin(),
                      newBranchesVec.end(),
                      [&branches](auto& newBranches)
                      { if(newBranches) branches.insert(branches.end(),
                                        std::make_move_iterator(newBranches->begin()),
                                        std::make_move_iterator(newBranches->end()));});
    }

    triesCout = 0;
    return false;
}

void HintHandler::checkMove(Definitions::MovementDirection moveDir)
{
    if(m_activeHint)
    {
        if(*m_hintNextExpectedDir == moveDir)
            ++m_hintNextExpectedDir;

        else
            invalidateHint();
    }
}

void HintHandler::checkUndo(const QList<QPointF>& pickedGems)
{
    if(m_activeHint)
    {
        if(m_hintNextExpectedDir == m_hintTrace.begin())
            invalidateHint();

        else
            --m_hintNextExpectedDir;
    }

    for(const auto& point : pickedGems)
    {
        Definitions::Position gemPos(point.y(), point.x());

        if(!StateWrapper::instance().state()->stuckArea().contains(gemPos))
            StateWrapper::instance().state()->hintCandidateGems().insert(gemPos);
    }
}

void HintHandler::invalidateHint()
{
    m_activeHint = false;
    m_hintNextExpectedDir = m_hintTrace.cend();
    std::vector<Definitions::MovementDirection>{}.swap(m_hintTrace);
}

void HintHandler::onGemPicked(quint32 rowIndex, quint32 columnIndex)
{
    if(m_hintTargetGem == Definitions::Position{rowIndex, columnIndex})
        invalidateHint();

    StateWrapper::instance().state()->hintCandidateGems().erase(Definitions::Position(rowIndex, columnIndex));
}

HintHandler::ShortestWayToGemLoopTask::ShortestWayToGemLoopTask(HintHandler* hintHandler,
                                                                  std::atomic_bool& pathFound,
                                                                  std::atomic_bool& returnEarly,
                                                                  const Definitions::Position& gemPos,
                                                                  std::unordered_set<Definitions::Position>& stuckArea) :
    m_hintHandler(hintHandler),
    m_pathFound(pathFound),
    m_returnEarly(returnEarly),
    m_gemPos(gemPos),
    m_stuckArea(stuckArea)
{

}

std::optional<std::vector<HintBranch>> HintHandler::ShortestWayToGemLoopTask::operator()(HintBranch& branch) const
{
    std::vector<HintBranch> newBranches;
    const auto& dirs {InertiaUtility::orderedDirections(branch.currentPos, m_gemPos)};

    HintBranch newBranch;
    Definitions::Position finalPos;
    bool safeFinalPos;
    std::optional<std::unordered_set<Definitions::Position>> trace{};

    for(const auto dir : dirs)
    {
        if(m_returnEarly)
            return {};

        std::tie (finalPos, safeFinalPos, trace) =
            StateWrapper::instance().state()->finalDestinationPlusTrace(branch.currentPos, dir);

        if(!safeFinalPos || finalPos == branch.currentPos || branch.stoppedByCells.contains(finalPos))
            continue;


        if(trace->contains(m_gemPos))
        {
            if(m_returnEarly)
                return {};

            if(m_stuckArea.contains(finalPos) &&
                !StateWrapper::instance().state()->canEnterStuckArea())
                continue;

            branch.track.push_back(dir);
            m_hintHandler->m_hintTrace = branch.track;
            m_hintHandler->m_activeHint = true;
            m_returnEarly.store(true);
            m_pathFound.store(true);
            return {};
        }

        if(m_returnEarly)
            return {};

        newBranch.currentPos = finalPos;
        newBranch.stoppedByCells = branch.stoppedByCells;
        newBranch.stoppedByCells.insert(finalPos);
        newBranch.track = branch.track;
        newBranch.track.push_back(dir);

        newBranches.push_back(std::move(newBranch));
    }

    return newBranches;
}
