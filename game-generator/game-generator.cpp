#include "game-generator/game-generator.hpp"
#include "state-wrapper.hpp"
#include "constants.hpp"
#include "utility.hpp"

#include <QTimer>
#include <QCoreApplication>

#include <random>
#include <thread>


void GameGenerator::generateAllGames(quint32 rowsCount,
                                      quint32 columnsCount,
                                      quint64 gamesCount,
                                      const QString& filePath)
{
    StateWrapper::instance().state()->setRowsCount(rowsCount);
    StateWrapper::instance().state()->setColumnsCount(columnsCount);
    initializeModel(true, filePath.mid(8), gamesCount);
}

void GameGenerator::initializeModel(bool storeInFile,
                                     const QString& filePath,
                                     std::optional<quint64> toBeGeneratedGamesCount)
{
    auto stopsSelectortimer {createStopsSelectorTimer(storeInFile)};

    if(!storeInFile)
        StateWrapper::instance().state()->beginResetModel();

    QTextStream* stream {};

    if(storeInFile)
    {
        m_file.setFileName(filePath);

        if(!m_file.open(QIODevice::WriteOnly))
            throw std::runtime_error{std::format("Could not open the file {} for writing game data into", filePath.toStdString())};

        stream = new QTextStream(&m_file);
    }

    std::optional<quint64> gamesCountOnlyStopsVar{};

    if(storeInFile)
        gamesCountOnlyStopsVar = std::max(1ULL, toBeGeneratedGamesCount.value() / 100);

    else
        toBeGeneratedGamesCount = 1;

    quint64 totalGamesGenerated {};

    while(toBeGeneratedGamesCount > 0)
    {
        StateWrapper::instance().state()->resetCells();

        placeBall();
        buildUpWalls();
        plantMines();
        placeGems();

        const auto gamesGenerated {placeStops(stopsSelectortimer, stream, gamesCountOnlyStopsVar)};

        if(!(storeInFile || gamesGenerated))
            return;

        if(storeInFile)
        {
            toBeGeneratedGamesCount =
                (toBeGeneratedGamesCount.value() < gamesGenerated) ?
                    0 : toBeGeneratedGamesCount.value() - gamesGenerated;

            totalGamesGenerated += gamesGenerated;
        }

        else
            toBeGeneratedGamesCount = 0;
    }

    if(!storeInFile)
        StateWrapper::instance().state()->endResetModel();

    else
    {
        StateWrapper::instance().state()->notifyGameGenerationCompletion(totalGamesGenerated);
        delete stream;
        stream = nullptr;
        m_file.close();

        StateWrapper::instance().state()->updatePaths(StateWrapper::instance().state()->rowsCount(),
                                                       StateWrapper::instance().state()->columnsCount(),
                                                       filePath);
    }

    delete stopsSelectortimer;
    stopsSelectortimer = nullptr;
}

void GameGenerator::newGameFromFile(quint32 rowsCount, quint32 columnsCount)
{
    auto filePath {StateWrapper::instance().state()->pathForDimensions(rowsCount, columnsCount)};

    if(!filePath.size())
        return;

    QFile file{filePath};

    if(!file.open(QIODevice::ReadOnly))
        throw std::runtime_error{std::format("Could not open file {} for reading.", filePath.toStdString())};

    StateWrapper::instance().state()->setRowsCount(rowsCount);
    StateWrapper::instance().state()->setColumnsCount(columnsCount);

    auto& gemsCount {StateWrapper::instance().state()->gemsCount()};
    gemsCount = StateWrapper::instance().state()->rowsCount() *
                                                   StateWrapper::instance().state()->columnsCount() *
                                                   Constants::kCellsGemsRatio;

    StateWrapper::instance().state()->remainingGemsCount() = gemsCount;

    StateWrapper::instance().state()->beginResetModel();

    StateWrapper::instance().state()->resetCells();

    QString fileContent;

    {
        QTextStream stream{&file};
        fileContent = stream.readAll();
    }

    if(!fileContent.size())
    {
        StateWrapper::instance().state()->endResetModel();
        return;
    }

    const auto gamesData {fileContent.split('#', Qt::SkipEmptyParts)};
    const auto gamesCount {gamesData.size() - 1};

    std::random_device rd;
    std::mt19937 generator(rd());
    auto gameData {gamesData.at(generator() % gamesCount)};

    loadGameFromData(gameData);

    StateWrapper::instance().state()->onGameStart() = true;
    StateWrapper::instance().state()->endResetModel();
}

void GameGenerator::loadSavedGame(quint32 rowsCount,
                                   quint32 columnsCount,
                                   QPointF ballPosPoint,
                                   QString stuckAreaData,
                                   QString stuckAreaGemsData,
                                   QString initialCellTypesData,
                                   QString cellTypesData)
{
    StateWrapper::instance().state()->beginResetModel();

    resetGameData(rowsCount, columnsCount);
    auto& ballPos {StateWrapper::instance().state()->ballPos()};
    ballPos = Definitions::Position(ballPosPoint.y(), ballPosPoint.x());
    StateWrapper::instance().state()->notifyBallPosChange(ballPos);

    {
        QTextStream stream{&stuckAreaData};
        quint64 stuckAreaSize{};
        stream >> stuckAreaSize;

        auto& stuckArea {StateWrapper::instance().state()->stuckArea()};
        quint32 rowIndex, columnIndex;

        for(quint64 i{}; i < stuckAreaSize; ++i)
        {
            stream >> rowIndex >> columnIndex;
            stuckArea.emplace(rowIndex, columnIndex);
        }
    }

    {
        QTextStream stream{&stuckAreaGemsData};
        quint64 stuckAreaGemsSize{};
        stream >> stuckAreaGemsSize;

        auto& stuckAreaGems {StateWrapper::instance().state()->stuckAreaGems()};
        quint32 rowIndex, columnIndex;

        for(quint64 i{}; i < stuckAreaGemsSize; ++i)
        {
            stream >> rowIndex >> columnIndex;
            stuckAreaGems.emplace_back(rowIndex, columnIndex);
        }
    }

    {
        QTextStream cellsStream{&initialCellTypesData};
        qint16 cellTypeValue{};

        for(quint32 rowIndex{}; rowIndex < rowsCount; ++rowIndex)
            for(quint32 columnIndex{}; columnIndex < columnsCount; ++columnIndex)
            {
                cellsStream >> cellTypeValue;
                StateWrapper::instance().state()->initialCells()[rowIndex][columnIndex] =
                    static_cast<Definitions::CellType>(cellTypeValue);
            }
    }

    {
        QTextStream cellsStream{&cellTypesData};
        qint16 cellTypeValue{};
        auto& gems {StateWrapper::instance().state()->gemsPositions()};

        for(quint32 rowIndex{}; rowIndex < rowsCount; ++rowIndex)
            for(quint32 columnIndex{}; columnIndex < columnsCount; ++columnIndex)
            {
                cellsStream >> cellTypeValue;
                auto& cellType {StateWrapper::instance().state()->cellAt(rowIndex, columnIndex)};
                cellType = static_cast<Definitions::CellType>(cellTypeValue);

                if(cellType == Definitions::CellType::Gem)
                {
                    gems.emplace_back(rowIndex, columnIndex);
                    ++StateWrapper::instance().state()->remainingGemsCount();
                }
            }
    }

    StateWrapper::instance().state()->findHintCandidateGems();
    StateWrapper::instance().state()->onGameStart() = true;
    StateWrapper::instance().state()->endResetModel();
}

void GameGenerator::storeInFile(QTextStream* file)
{
    const auto& ballPosition {StateWrapper::instance().state()->ballPos()};

    (*file) << QString("%1 %2 %3 %4 ").
               arg(StateWrapper::instance().state()->rowsCount()).
               arg(StateWrapper::instance().state()->columnsCount()).
               arg(ballPosition.rowIndex).
               arg(ballPosition.columnIndex);

    const auto& stuckArea {StateWrapper::instance().state()->stuckArea()};
    (*file) << QString("%1 ").arg(stuckArea.size());

    for(const auto& pos : stuckArea)
        (*file) << QString("%1 %2 ").arg(pos.rowIndex).arg(pos.columnIndex);

    const auto& stuckAreaGems {StateWrapper::instance().state()->stuckAreaGems()};
    (*file) << QString("%1 ").arg(stuckAreaGems.size());

    for(const auto& pos : stuckAreaGems)
        (*file) << QString("%1 %2 ").arg(pos.rowIndex).arg(pos.columnIndex);

    const auto rowsCount {StateWrapper::instance().state()->rowsCount()};

    for(quint32 rowIndex{}; rowIndex < rowsCount; ++rowIndex)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents);
        auto& row {StateWrapper::instance().state()->initialCells().at(rowIndex)};

        for(auto it {row.cbegin()}; it != row.cend(); ++it)
            (*file) << QString("%1 ").arg(static_cast<int8_t>(*it));
    }

    (*file) << "# ";
}

void GameGenerator::resetStopParam()
{
    m_stopNewGameGeneration = false;
}

void GameGenerator::resetGameData(quint32 rowsCount, quint32 columnsCount)
{
    StateWrapper::instance().state()->resetGameData(rowsCount, columnsCount);
    m_stopNewGameGeneration = false;
}

// Temporary
#include <iostream>
//

void GameGenerator::loadGameFromData(QString& gameData)
{
    QTextStream stream{&gameData};
    quint32 rowsCount{}, columnsCountx{};

    auto& ballPosition {StateWrapper::instance().state()->ballPos()};
    stream >> rowsCount >> columnsCountx >> ballPosition.rowIndex >> ballPosition.columnIndex;

    StateWrapper::instance().state()->initialBallPos() = ballPosition;
    StateWrapper::instance().state()->notifyBallPosChange(ballPosition);

    {
        auto& stuckArea {StateWrapper::instance().state()->stuckArea()};
        quint64 stuckAreaSize{};
        stream >> stuckAreaSize;

        quint32 rowIndx{}, columnIndx{};

        for(quint64 i{}; i < stuckAreaSize; ++i)
        {
            stream >> rowIndx >> columnIndx;
            stuckArea.emplace(rowIndx, columnIndx);
        }
    }

    {
        auto& stuckAreaGems {StateWrapper::instance().state()->stuckAreaGems()};
        quint64 stuckAreaGemsSize{};
        stream >> stuckAreaGemsSize;

        quint32 rowIndx{}, columnIndx{};

        for(quint64 i{}; i < stuckAreaGemsSize; ++i)
        {
            stream >> rowIndx >> columnIndx;
            stuckAreaGems.emplace_back(rowIndx, columnIndx);
        }
    }

    quint16 cellTypeValue{};
    std::vector<Definitions::Position>{}.swap(StateWrapper::instance().state()->gemsPositions());


    for(quint32 rowIndex{}; rowIndex < rowsCount; ++rowIndex)
    {
        auto& row {StateWrapper::instance().state()->cells().at(rowIndex)};

        for(auto it{row.begin()}; it != row.end(); ++it)
        {
            stream >> cellTypeValue;

            switch(cellTypeValue)
            {
            case 0:
                *it = Definitions::CellType::Clear;
                break;

            case 2:
                *it = Definitions::CellType::Wall;
                break;

            case 3:
                *it = Definitions::CellType::Stop;
                break;

            case 4:
            {
                *it = Definitions::CellType::Gem;
                StateWrapper::instance().state()->gemsPositions().emplace_back(rowIndex, std::distance(row.begin(), it));
            }
            break;

            case 5:
                *it = Definitions::CellType::Mine;
                break;
            }
        }
    }

    StateWrapper::instance().state()->initialCells() = StateWrapper::instance().state()->cells();
    StateWrapper::instance().state()->findHintCandidateGems();
}

void GameGenerator::placeBall()
{
    const auto rowsCount {StateWrapper::instance().state()->rowsCount()};
    const auto columnsCount {StateWrapper::instance().state()->columnsCount()};
    auto& ballPosition {StateWrapper::instance().state()->ballPos()};
    ballPosition.rowIndex = InertiaUtility::randomRowIndex(rowsCount);
    ballPosition.columnIndex = InertiaUtility::randomColumnIndex(columnsCount);

    StateWrapper::instance().state()->notifyBallPosChange(ballPosition);
    StateWrapper::instance().state()->initialBallPos() = ballPosition;
    StateWrapper::instance().state()->cellAt(ballPosition.rowIndex, ballPosition.columnIndex) = Definitions::CellType::Stop;
}

void GameGenerator::placeObstacles(bool plantMines)
{
    static std::vector<Definitions::Position> walls;

    auto properCells {obstaclesInitialCandidates(walls, plantMines)};
    placeObstaclesHelper(properCells, walls, plantMines);

    if(plantMines)
        std::vector<Definitions::Position>{}.swap(walls);
}

void GameGenerator::buildUpWalls()
{
    placeObstacles(false);
}

void GameGenerator::plantMines()
{
    placeObstacles(true);
}

void GameGenerator::placeGems()
{
    std::vector<Definitions::Position> clearCells;

    const auto rowsCount {StateWrapper::instance().state()->rowsCount()};
    const auto columnsCount {StateWrapper::instance().state()->columnsCount()};

    for(quint32 rowIndex{}; rowIndex < rowsCount; ++rowIndex)
        for(quint32 columnIndex{}; columnIndex < columnsCount; ++columnIndex)
        {
            if(StateWrapper::instance().state()->cellAt(rowIndex, columnIndex) == Definitions::CellType::Clear)
                clearCells.emplace_back(rowIndex, columnIndex);
        }

    std::random_device rd;
    std::mt19937 generator(rd());

    std::shuffle(clearCells.begin(), clearCells.end(), generator);

    StateWrapper::instance().state()->remainingGemsCount() = rowsCount * columnsCount * Constants::kCellsGemsRatio;
    std::vector<Definitions::Position>{}.swap(StateWrapper::instance().state()->gemsPositions());

    const auto remainingGemsCnt {StateWrapper::instance().state()->remainingGemsCount()};

    for(std::size_t i{}; i < remainingGemsCnt; ++i)
    {
        if(!clearCells.size())
            break;

        const auto& pos {clearCells.back()};
        const auto rowIndex {pos.rowIndex};
        const auto columnIndex {pos.columnIndex};
        clearCells.pop_back();
        StateWrapper::instance().state()->cellAt(rowIndex, columnIndex) = Definitions::CellType::Gem;
        StateWrapper::instance().state()->gemsPositions().emplace_back(rowIndex, columnIndex);
    }
}

quint64 GameGenerator::placeStops(QTimer* stopsSelectortimer,
                                 QTextStream* fileStream,
                                 std::optional<quint64> targetedGamesCount)
{
    return generateTryStopPatterns(stopCandidateCells(),
                                   stopsSelectortimer,
                                   fileStream,
                                   targetedGamesCount);
}

std::vector<std::vector<bool>> GameGenerator::obstaclesInitialCandidates(const std::vector<Definitions::Position>& walls,
                                                                        bool plantMines) const
{
    std::vector<std::vector<bool>> properCells(StateWrapper::instance().state()->rowsCount());

    for(auto& row : properCells)
        row.resize(StateWrapper::instance().state()->columnsCount(), true);

    const auto& ballPosition {StateWrapper::instance().state()->ballPos()};
    properCells[ballPosition.rowIndex][ballPosition.columnIndex] = false;

    if(plantMines)
        for(const auto& wallPos : walls)
            properCells[wallPos.rowIndex][wallPos.columnIndex] = false;

    return properCells;
}

void GameGenerator::placeObstaclesHelper(std::vector<std::vector<bool>>& properCells,
                                        std::vector<Definitions::Position>& walls,
                                        bool plantMines)
{
    std::size_t obstaclesCount = StateWrapper::instance().state()->rowsCount() *
                                  StateWrapper::instance().state()->columnsCount() *
                                  Constants::kCellsObstaclesRatio;

    quint32 rowIndex{}, columnIndex{};
    bool terminateOuterLoop {false};
    bool terminateInnerLoops{false};
    auto candidatePositions {obstacleCandidatePositionsGenerator(properCells)};

    for(quint32 i{}; i < obstaclesCount; ++i)
    {
        if(!candidatePositions.size())
            break;

        terminateOuterLoop = false;

        while(!terminateOuterLoop && candidatePositions.size())
        {
            const auto& pos {candidatePositions.back()};
            rowIndex = pos.rowIndex;
            columnIndex = pos.columnIndex;
            candidatePositions.pop_back();

            auto& candidateCell {StateWrapper::instance().state()->cellAt(rowIndex,columnIndex)};
            candidateCell = plantMines ? Definitions::CellType::Mine : Definitions::CellType::Wall;

            if(!plantMines)
                walls.emplace_back(rowIndex, columnIndex);

            bool isOk{true};
            terminateInnerLoops = false;

            const auto rowsCount {StateWrapper::instance().state()->rowsCount()};
            const auto columnsCount {StateWrapper::instance().state()->columnsCount()};

            for(quint32 row{}; row < rowsCount; ++row)
            {
                for(quint32 column{}; column < columnsCount; ++column)
                {
                    Definitions::Position pos{row, column};
                    
                    if(StateWrapper::instance().state()->isClear(pos) &&
                       !isConnectedTo(StateWrapper::instance().state()->ballPos(), pos))
                    {
                        candidateCell = Definitions::CellType::Clear;

                        if(!plantMines)
                            walls.pop_back();

                        terminateInnerLoops = true;
                        break;
                    }
                }

                if(terminateInnerLoops)
                    break;
            }

            terminateOuterLoop = !terminateInnerLoops;
        }
    }
}

std::vector<Definitions::Position> GameGenerator::obstacleCandidatePositionsGenerator(const std::vector<std::vector<bool>>& properCells)
{
    std::vector<Definitions::Position> result;

    const auto rowsCount {StateWrapper::instance().state()->rowsCount()};
    const auto columnsCount {StateWrapper::instance().state()->columnsCount()};

    for(quint32 rowIndex{}; rowIndex < rowsCount; ++rowIndex)
        for(quint32 columnIndex{}; columnIndex < columnsCount; ++columnIndex)
        {
            if(properCells[rowIndex][columnIndex])
                result.emplace_back(rowIndex, columnIndex);
        }

    std::random_device rd;
    std::mt19937 generator(rd());

    std::shuffle(result.begin(), result.end(), generator);

    return result;
}

std::vector<Definitions::Position> GameGenerator::stopCandidateCells() const
{
    std::vector<Definitions::Position> availableCells;

    const auto rowsCount {StateWrapper::instance().state()->rowsCount()};
    const auto columnsCount {StateWrapper::instance().state()->columnsCount()};

    for(quint32 row{}; row < rowsCount; ++row)
        for(quint32 column{}; column < columnsCount; ++column)
            if(StateWrapper::instance().state()->cellAt(row, column) == Definitions::CellType::Clear)
                availableCells.emplace_back(row, column);

    return availableCells;
}

std::pair<std::optional<std::unordered_set<Definitions::Position>>, std::unordered_set<Definitions::Position>>
GameGenerator::visitableCells(const Definitions::Position& currentPosition, bool onlyStoppedByCells) const
{
    std::optional<std::unordered_set<Definitions::Position>> visitedCells{};

    if(!onlyStoppedByCells)
        visitedCells.emplace({currentPosition});

    std::unordered_set<Definitions::Position> stoppedByCells {currentPosition};
    std::vector<Definitions::Position> traces{currentPosition};

    while(traces.size())
    {
        const auto tracesCount {traces.size()};

        for(quint64 i{}; i < tracesCount; ++i)
        {
            for(const auto dir : Constants::kAllDirections)
            {
                const auto currentPos {traces.front()};

                auto [finalPos, safeFinalPos, trace]
                    {StateWrapper::instance().state()->finalDestinationPlusTrace(currentPos, dir, true, true)};

                if(!safeFinalPos || finalPos == currentPos || stoppedByCells.contains(finalPos))
                    continue;

                if(!onlyStoppedByCells)
                {
                    visitedCells->merge(trace.value());
                    visitedCells->insert(finalPos);
                }

                stoppedByCells.insert(finalPos);
                traces.push_back(std::move(finalPos));
            }

            traces.erase(traces.begin());
        }
    }

    return {visitedCells, stoppedByCells};
}

void GameGenerator::visitableCellsHelper(const Definitions::Position& startPos,
                                           std::unordered_set<Definitions::Position>& processedSourcePositions,
                                           std::unordered_set<Definitions::Position>& currentResult,
                                           std::optional<Definitions::Position>& stuckInPos,
                                           std::optional<Definitions::MovementDirection> excludedDir) const
{
    for(auto direction : Constants::kAllDirections)
    {
        if(excludedDir && direction == excludedDir.value())
            continue;

        auto [finalPos, safeFinalDest, vCellsInDir] =
            StateWrapper::instance().state()->finalDestinationPlusTrace(startPos, direction, true, true);

        if(startPos == finalPos)
            continue;

        if(safeFinalDest)
        {
            const auto processedAsSourcePos {processedSourcePositions.contains(finalPos)};

            if(!processedAsSourcePos)
            {
                const auto startPos {finalPos};
                bool escapePossible {false};

                for(auto direction : Constants::kAllDirections)
                {
                    auto [finalPos, safeFinalDest, vCellsInDir] =
                        StateWrapper::instance().state()->finalDestinationPlusTrace(startPos, direction, false);

                    if(safeFinalDest && (!stuckInPos || finalPos != stuckInPos.value()))
                    {
                        escapePossible = true;
                        break;
                    }
                }

                bool addTrace {true};

                if(!escapePossible)
                {
                    if(!stuckInPos && !StateWrapper::instance().state()->explodesAfterPassingFrom(startPos))
                        stuckInPos = startPos;

                    else
                        addTrace = false;
                }

                if(addTrace)
                {
                    currentResult.merge(vCellsInDir.value());
                    processedSourcePositions.insert(finalPos);

                    visitableCellsHelper(finalPos,
                                         processedSourcePositions,
                                         currentResult,
                                         stuckInPos,
                                         InertiaUtility::oppositeDirection(direction));
                }
            }
        }
    }
}

void GameGenerator::applyStopPattern(const std::vector<Definitions::Position>& stopPattern)
{
    auto& initCells {StateWrapper::instance().state()->initialCells()};
    initCells = StateWrapper::instance().state()->cells();

    for(const auto [stopPosRow, stopPosColumn] : stopPattern)
        initCells[stopPosRow][stopPosColumn] = Definitions::CellType::Stop;
}

// Temporary
// #include <iostream>
//

quint64 GameGenerator::generateTryStopPatterns(const std::vector<Definitions::Position>& availableCells,
                                                 QTimer* stopsSelectortimer,
                                                 QTextStream* fileStream,
                                                 std::optional<quint64> targetedGamesCount)
{
    const auto availableCellsCount {availableCells.size()};
    bool generateAllGames= fileStream;
    auto selectionsModel {generateStopsSelectionsModel(availableCellsCount, generateAllGames)};
    quint64 gamesGenerated {};
    std::vector<Definitions::Position> selection;
    const auto ballPos {StateWrapper::instance().state()->ballPos()};

    // Temporary
    //std::cout << "@@@@@@@@@ Trying a new game Ball/Wall/Mine pattern ... @@@@@@@@@" << std::endl;
    //

    stopsSelectortimer->stop();
    resetStopperVar();
    stopsSelectortimer->start();

    do
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents);

        std::vector<Definitions::Position>{}.swap(selection);

        for(quint32 i{}; i < availableCellsCount; ++i)
            if(selectionsModel[i])
                selection.push_back(availableCells[i]);

        applyStopPattern(selection);
        const auto [vCells, stoppedByCells] {visitableCells(ballPos)};

        bool skipCurrentPattern {false};

        const auto& gemPoses {StateWrapper::instance().state()->gemsPositions()};

        for(const auto& gemPos : gemPoses)
        {
            QCoreApplication::processEvents(QEventLoop::AllEvents);

            if(!vCells->contains(gemPos))
            {
                skipCurrentPattern = true;
                break;
            }
        }

        std::unordered_set<Definitions::Position> stuckArea;

        if(!skipCurrentPattern)
        {
            std::tie(skipCurrentPattern, stuckArea) = checkSolvability(stoppedByCells);
            skipCurrentPattern = !skipCurrentPattern;
        }

        if(!skipCurrentPattern)
        {
            // Temporary
             std::cout << "**** A new game generated! ****" << std::endl;
            //

            ++gamesGenerated;

            StateWrapper::instance().state()->stuckArea() = std::move(stuckArea);
            StateWrapper::instance().state()->stuckAreaGems() = findStuckAreaGems();

            if(generateAllGames)
            {
                storeInFile(fileStream);
                std::vector<std::vector<Definitions::CellType>>{}.swap(StateWrapper::instance().state()->initialCells());
            }

            else
            {
                StateWrapper::instance().state()->cells() = StateWrapper::instance().state()->initialCells();
                break;
            }

        }

        if(targetedGamesCount && targetedGamesCount.value() == gamesGenerated)
            break;
    }

    while (!m_stopNewGameGeneration.load() && std::prev_permutation(selectionsModel.begin(), selectionsModel.end()));

    if(!generateAllGames && m_stopNewGameGeneration.load() && !gamesGenerated)
        std::thread(&GameGenerator::newGameFromFile,
                    this,
                    StateWrapper::instance().state()->rowsCount(),
                    StateWrapper::instance().state()->columnsCount()).detach();

    return gamesGenerated;
}

std::vector<bool> GameGenerator::generateStopsSelectionsModel(std::size_t availableCellsCount,
                                                             bool generateAllGames) const
{
    std::vector<bool> selectionsModel;
    std::size_t stopsCount = StateWrapper::instance().state()->rowsCount() *
                              StateWrapper::instance().state()->columnsCount() *
                              Constants::kCellsStopsRoughRatio;

    if(generateAllGames)
    {
        selectionsModel.resize(stopsCount, true);
        selectionsModel.resize(availableCellsCount, false);

        std::random_device rd;
        std::mt19937 generator(rd());

        std::shuffle(selectionsModel.begin(), selectionsModel.end(), generator);
    }

    else
    {
        auto modulo {availableCellsCount / stopsCount};

        std::size_t selectedsCount{};

        for(std::size_t i{}; i < availableCellsCount; ++i)
        {
            if(i % modulo == (modulo / 2 ) && selectedsCount < stopsCount)
            {
                ++selectedsCount;
                selectionsModel.push_back(true);
            }

            else
                selectionsModel.push_back(false);
        }
    }

    return selectionsModel;
}

QTimer* GameGenerator::createStopsSelectorTimer(bool generateMultipleGames)
{
    auto stopsSelectortimer {new QTimer(this)};

    stopsSelectortimer->setSingleShot(true);
    stopsSelectortimer->setTimerType(Qt::PreciseTimer);

    if(generateMultipleGames)
        stopsSelectortimer->setInterval(1.8 * std::pow(1.7, StateWrapper::instance().state()->rowsCount()));
    else
        stopsSelectortimer->setInterval(300);

    connect(stopsSelectortimer, &QTimer::timeout, this, &GameGenerator::setStopperVar);

    return stopsSelectortimer;
}

bool GameGenerator::isConnectedTo(const Definitions::Position& sourcePos,
                                 const Definitions::Position& targetPos) const
{
    std::unordered_set<Definitions::Position> alreadyChecked{};
    return isConnectedToHelper(sourcePos, targetPos, alreadyChecked);
}

bool GameGenerator::isConnectedToHelper(const Definitions::Position& sourcePos,
                                       const Definitions::Position& targetPos,
                                       std::unordered_set<Definitions::Position>& alreadyChecked) const
{
    const auto& nonObsNeighbours {nonObstacleNeighbours(sourcePos, alreadyChecked)};

    for(const auto& pos : nonObsNeighbours)
        if(pos == targetPos)
            return true;

    return std::any_of(nonObsNeighbours.begin(),
                       nonObsNeighbours.end(),
                       [this, &alreadyChecked, targetPos](const auto& pos)
                       { return isConnectedToHelper(pos, targetPos, alreadyChecked);});
}

std::vector<Definitions::Position> GameGenerator::nonObstacleNeighbours(const Definitions::Position& pos,
                                                                        std::unordered_set<Definitions::Position>& alreadyChecked) const
{
    std::vector<Definitions::Position> result;

    for(const auto direction : Constants::kAllDirections)
    {
        const auto& nextPos {StateWrapper::instance().state()->nextCellPos(pos, direction)};

        if(!nextPos || alreadyChecked.contains(nextPos.value()))
            continue;

        if(const auto& cell {StateWrapper::instance().state()->cellAt(nextPos->rowIndex, nextPos->columnIndex)};
            cell != Definitions::CellType::Wall && cell != Definitions::CellType::Mine)
        {
            result.push_back(nextPos.value());
            alreadyChecked.insert(nextPos.value());
        }
    }

    return result;
}

void GameGenerator::resetStopperVar()
{
    m_stopNewGameGeneration.store(false);
}

void GameGenerator::setStopperVar()
{
    // Temporary
    // std::cout << "$$$$$$$ m_stopNewGameGeneration set to true! $$$$$$$" << std::endl;
    //

    m_stopNewGameGeneration.store(true);
}

std::pair<bool, std::unordered_set<Definitions::Position>>
GameGenerator::checkSolvability(const std::unordered_set<Definitions::Position>& originalStoppedByCells)
{
    std::optional<Definitions::Position> stuckAreaRepresentative{};
    std::unordered_set<Definitions::Position> stuckArea;

    for(const auto& startPos : originalStoppedByCells)
    {
        std::unordered_set<Definitions::Position> stoppedByCells;
        std::tie(std::ignore, stoppedByCells) = visitableCells(startPos, true);

        for(const auto& pos : originalStoppedByCells)
        {
            if(!stoppedByCells.contains(pos))
            {
                if(!stuckAreaRepresentative)
                {
                    stuckAreaRepresentative = startPos;
                    stuckArea.insert(startPos);
                }

                else if(!stoppedByCells.contains(stuckAreaRepresentative.value()))
                    return {false, {}};

                else
                    stuckArea.insert(startPos);
            }

        }
    }

    return {true, stuckArea};
}

std::vector<Definitions::Position> GameGenerator::findStuckAreaGems() const
{
    if(!StateWrapper::instance().state()->stuckArea().size())
        return {};

    std::vector<Definitions::Position> result;
    const auto& stuckAreaRep {*StateWrapper::instance().state()->stuckArea().begin()};

    auto [vCells, stoppedByCells] {visitableCells(stuckAreaRep)};

    const auto& gemPoses {StateWrapper::instance().state()->gemsPositions()};

    for(const auto& gemPos : gemPoses)
        if(vCells->contains(gemPos))
            result.push_back(gemPos);

    std::sort(result.begin(), result.end());

    return result;
}









