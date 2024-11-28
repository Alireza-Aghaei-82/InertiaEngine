#include "service/move-handler.hpp"
#include "state-wrapper.hpp"


using namespace Definitions;


MoveHandler::MoveHandler() : m_hintHandler(std::make_unique<HintHandler>())
{

}

MovementResult MoveHandler::moveBall(MovementDirection direction)
{
    m_hintHandler->checkMove(direction);

    auto ballPos {StateWrapper::instance().state()->ballPos()};
    std::optional<Position> currentPos {ballPos};
    auto previousPos {currentPos.value()};
    CellType currentCellType;
    QPointF finalPos;
    bool emitDataChanged {false};
    bool checkGameCompletion {false};
    bool updateBallPos {false};
    bool exitLoop {false};
    QList<QPointF> collectedGems;

    while(true)
    {
        previousPos = currentPos.value();
        currentPos = StateWrapper::instance().state()->nextCellPos(currentPos.value(), direction);

        if(currentPos)
            currentCellType = StateWrapper::instance().state()->cellAt(currentPos->rowIndex, currentPos->columnIndex);

        if(!currentPos || currentCellType == CellType::Wall)
        {
            finalPos = QPointF(previousPos.columnIndex, previousPos.rowIndex);
            updateBallPos = true;
            exitLoop = true;
        }

        else if(currentCellType == CellType::Mine)
        {
            updateBallPos = true;
            emitDataChanged = true;
            exitLoop = true;
            StateWrapper::instance().state()->cellAt(currentPos->rowIndex, currentPos->columnIndex) = CellType::Exploded;
            finalPos = QPointF(currentPos->columnIndex, currentPos->rowIndex);
        }

        else if(currentCellType == CellType::Gem)
        {
            StateWrapper::instance().state()->cellAt(currentPos->rowIndex, currentPos->columnIndex) = CellType::Waiting;
            collectedGems.emplace_back(currentPos->columnIndex, currentPos->rowIndex);
            checkGameCompletion = true;
            onGemPicked(currentPos->rowIndex, currentPos->columnIndex);
        }

        else if(currentCellType == CellType::Stop)
        {
            updateBallPos = true;
            exitLoop = true;
            finalPos = QPointF(currentPos->columnIndex, currentPos->rowIndex);
        }


        if(updateBallPos)
        {
            updateBallPos = false;
            auto& ballPos {StateWrapper::instance().state()->ballPos()};
            ballPos.rowIndex = finalPos.y();
            ballPos.columnIndex = finalPos.x();
            StateWrapper::instance().state()-> notifyBallPosChange(ballPos);
        }

        if(emitDataChanged)
        {
            emitDataChanged = false;
            auto indx {StateWrapper::instance().state()->index(currentPos->rowIndex, currentPos->columnIndex)};
            StateWrapper::instance().state()->notifyDataChange(indx, indx);
        }

        if(checkGameCompletion)
        {
            checkGameCompletion = false;

            if(!StateWrapper::instance().state()->remainingGemsCount())
                StateWrapper::instance().state()->notifyGameCompletion();
        }

        if(exitLoop)
            return {finalPos, std::move(collectedGems)};
    }
}

void MoveHandler::undo(QPointF preMovePos, QList<QPointF> pickedGems)
{
    for(const auto& point : pickedGems)
    {
        const auto px {point.x()};
        const auto py {point.y()};
        StateWrapper::instance().state()->gemsPositions().emplace_back(py, px);
        StateWrapper::instance().state()->cellAt(py, px) = CellType::Gem;
    }

    StateWrapper::instance().state()->remainingGemsCount() += pickedGems.size();

    auto& ballPos {StateWrapper::instance().state()->ballPos()};
    ballPos.rowIndex = preMovePos.y();
    ballPos.columnIndex = preMovePos.x();

    StateWrapper::instance().state()->notifyBallPosChange(ballPos);

    for(const auto& point : pickedGems)
    {
        auto modelIndex {StateWrapper::instance().state()->index(point.y(), point.x())};
        StateWrapper::instance().state()->notifyDataChange(modelIndex, modelIndex);
    }

    m_hintHandler->checkUndo(pickedGems);
}

void MoveHandler::hint()
{
    m_hintHandler->hint();
}

void MoveHandler::onGemPicked(quint32 rowIndex, quint32 columnIndex)
{
    --StateWrapper::instance().state()->remainingGemsCount();
    std::erase(StateWrapper::instance().state()->gemsPositions(), Definitions::Position{rowIndex, columnIndex});

    m_hintHandler->onGemPicked(rowIndex, columnIndex);
}
