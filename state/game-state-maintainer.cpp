#include "constants.hpp"
#include "utility.hpp"
#include "state-wrapper.hpp"

#include <QFile>

#include <execution>


GameStateMaintainer::GameStateMaintainer(QObject* parent) : QObject(parent)
{
    StateWrapper::instance().setState(this);
}

InertiaModel* GameStateMaintainer::model() const
{
    return m_gameModel;
}

void GameStateMaintainer::setModel(InertiaModel* model)
{
    m_gameModel = model;
}

std::vector<std::vector<Definitions::CellType>>& GameStateMaintainer::cells()
{
    return m_cells;
}

const std::vector<std::vector<Definitions::CellType>>& GameStateMaintainer::cells() const
{
    return m_cells;
}

Definitions::CellType& GameStateMaintainer::cellAt(quint32 rowIndex, quint32 columnIndex)
{
    return const_cast<Definitions::CellType&>(std::as_const(*this).cellAt(rowIndex, columnIndex));
}

const Definitions::CellType& GameStateMaintainer::cellAt(quint32 rowIndex, quint32 columnIndex) const
{
    if(rowIndex >= m_rowsCount || columnIndex >= m_columnsCount)
        throw std::out_of_range{"[GameStateMaintainer][cellAt]: Invalid index/indices given!"};

    return m_cells[rowIndex][columnIndex];
}

std::vector<std::vector<Definitions::CellType>>& GameStateMaintainer::initialCells()
{
    return m_initialCells;
}

const std::vector<std::vector<Definitions::CellType>>& GameStateMaintainer::initialCells() const
{
    return m_initialCells;
}

quint32 GameStateMaintainer::rowsCount() const
{
    return m_rowsCount;
}

void GameStateMaintainer::setRowsCount(quint32 newRowsCount, bool emitSignal)
{
    if (m_rowsCount != newRowsCount)
    {
        m_rowsCount = newRowsCount;

        if(emitSignal)
            static_cast<InertiaModel*>(m_gameModel)->notifyRowsCountChange(newRowsCount);
    }
}

quint32 GameStateMaintainer::columnsCount() const
{
    return m_columnsCount;
}

void GameStateMaintainer::setColumnsCount(quint32 columnsCount, bool emitSignal)
{
    if(m_columnsCount != columnsCount)
    {
        m_columnsCount = columnsCount;

        if(emitSignal)
            static_cast<InertiaModel*>(m_gameModel)->notifyColumnsCountChange(columnsCount);
    }
}

QModelIndex GameStateMaintainer::index(quint32 rowIndex, quint32 columnIndex)
{
    return m_gameModel->index(rowIndex, columnIndex);
}

QVariant GameStateMaintainer::data(const QModelIndex& index, int role) const
{
    int rowIndex = index.row();
    int columnIndex = index.column();

    if(rowIndex < 0 || rowIndex >= m_rowsCount || columnIndex < 0 || columnIndex >= m_columnsCount)
        return QVariant();

    if(role == Qt::DisplayRole)
        return m_cells[rowIndex][columnIndex];

    return {};
}

void GameStateMaintainer::resetGameData(quint32 rowsCount, quint32 columnsCount)
{
    setRowsCount(rowsCount);
    setColumnsCount(columnsCount);
    resetCells(false);
    m_remainingGemsCount = 0;
}

void GameStateMaintainer::notifyDataChange(const QModelIndex& topLeft, const QModelIndex& downRight) const
{
    static_cast<InertiaModel*>(m_gameModel)->notifyDataChange(topLeft, downRight);
}

void GameStateMaintainer::beginResetModel()
{
    static_cast<InertiaModel*>(m_gameModel)->notifyDataModificationStart();
}

void GameStateMaintainer::endResetModel()
{
    static_cast<InertiaModel*>(m_gameModel)->notifyDataModificationEnd();
}

void GameStateMaintainer::notifyGameGenerationCompletion(quint64 gamesGenerated)
{
    static_cast<InertiaModel*>(m_gameModel)->notifyGameGenerationCompletion(gamesGenerated);
}

quint32& GameStateMaintainer::gemsCount()
{
    return m_gemsCount;
}

const quint32& GameStateMaintainer::gemsCount() const
{
    return m_gemsCount;
}

Definitions::Position& GameStateMaintainer::ballPos()
{
    return m_currentBallPos;
}

const Definitions::Position& GameStateMaintainer::ballPos() const
{
    return m_currentBallPos;
}

Definitions::Position& GameStateMaintainer::initialBallPos()
{
    return m_initialBallPos;
}

const Definitions::Position& GameStateMaintainer::initialBallPos() const
{
    return m_initialBallPos;
}

QPointF GameStateMaintainer::ballPositionPoint() const
{
    return QPointF(m_currentBallPos.columnIndex, m_currentBallPos.rowIndex);
}

void GameStateMaintainer::setBallPosition(QPointF pos)
{
    if(m_currentBallPos.rowIndex != pos.y() || m_currentBallPos.columnIndex != pos.x())
    {
        m_currentBallPos.rowIndex = pos.y();
        m_currentBallPos.columnIndex = pos.x();

        notifyBallPosChange(pos);
    }
}

void GameStateMaintainer::notifyBallPosChange(const Definitions::Position& ballPos)
{
    static_cast<InertiaModel*>(m_gameModel)->notifyBallPosChange(ballPos);
}

void GameStateMaintainer::notifyBallPosChange(const QPointF& ballPos)
{
    static_cast<InertiaModel*>(m_gameModel)->notifyBallPosChange(ballPos);
}

std::vector<Definitions::Position>& GameStateMaintainer::gemsPositions()
{
    return m_gemsPositions;
}

const std::vector<Definitions::Position>& GameStateMaintainer::gemsPositions() const
{
    return m_gemsPositions;
}

quint32& GameStateMaintainer::remainingGemsCount()
{
    return m_remainingGemsCount;
}

const quint32& GameStateMaintainer::remainingGemsCount() const
{
    return m_remainingGemsCount;
}

void GameStateMaintainer::notifyGameCompletion()
{
    static_cast<InertiaModel*>(m_gameModel)->notifyGameCompletion();
}

void GameStateMaintainer::showHint(Definitions::MovementDirection moveDir)
{
    static_cast<InertiaModel*>(m_gameModel)->notifyHint(moveDir);
}

std::unordered_set<Definitions::Position>& GameStateMaintainer::stuckArea()
{
    return m_stuckArea;
}

const std::unordered_set<Definitions::Position>& GameStateMaintainer::stuckArea() const
{
    return m_stuckArea;
}

std::vector<Definitions::Position>& GameStateMaintainer::stuckAreaGems()
{
    return m_stuckAreaGems;
}

const std::vector<Definitions::Position>& GameStateMaintainer::stuckAreaGems() const
{
    return m_stuckAreaGems;
}

std::optional<Definitions::CellType> GameStateMaintainer::nextCell(const Definitions::Position& currentPos,
                                                                    Definitions::MovementDirection direction) const
{
    switch(direction)
    {
    case Definitions::MovementDirection::Up:
    {
        const auto row {currentPos.rowIndex};

        if(row > 0)
            return m_cells[row - 1][currentPos.columnIndex];

        return {};
    }


    case Definitions::MovementDirection::Down:
    {
        const auto row {currentPos.rowIndex};

        if(row < m_rowsCount - 1)
            return m_cells[row + 1][currentPos.columnIndex];

        return {};
    }

    case Definitions::MovementDirection::Right:
    {
        const auto column {currentPos.columnIndex};

        if(column < m_columnsCount - 1)
            return m_cells[currentPos.rowIndex][column + 1];

        return {};
    }

    case Definitions::MovementDirection::Left:
    {
        const auto column {currentPos.columnIndex};

        if(column > 0)
            return m_cells[currentPos.rowIndex][column - 1];

        return {};
    }

    case Definitions::MovementDirection::UpRight:
    {
        const auto [row, column] {currentPos};

        if(!row || column == m_columnsCount - 1)
            return {};

        return m_cells[row - 1][column + 1];
    }

    case Definitions::MovementDirection::DownRight:
    {
        const auto [row, column] {currentPos};

        if(row == m_rowsCount - 1 || column == m_columnsCount - 1)
            return {};

        return m_cells[row + 1][column + 1];
    }

    case Definitions::MovementDirection::DownLeft:
    {
        const auto [row, column] {currentPos};

        if(row == m_rowsCount - 1 || !column)
            return {};

        return m_cells[row + 1][column - 1];
    }

    case Definitions::MovementDirection::UpLeft:
    {
        const auto [row, column] {currentPos};

        if(!row || !column)
            return {};

        return m_cells[row - 1][column - 1];
    }

    default:
        return {};
    }
}

std::optional<Definitions::Position> GameStateMaintainer::nextCellPos(const Definitions::Position& pos, Definitions::MovementDirection direction) const
{
    switch(direction)
    {
    case Definitions::MovementDirection::Up:
    {
        const auto row {pos.rowIndex};

        if(row > 0)
            return Definitions::Position{row - 1, pos.columnIndex};

        return {};
    }


    case Definitions::MovementDirection::Down:
    {
        const auto row {pos.rowIndex};

        if(row < m_rowsCount - 1)
            return Definitions::Position{row + 1, pos.columnIndex};

        return {};
    }

    case Definitions::MovementDirection::Right:
    {
        const auto column {pos.columnIndex};

        if(column < m_columnsCount - 1)
            return Definitions::Position{pos.rowIndex, column + 1};

        return {};
    }

    case Definitions::MovementDirection::Left:
    {
        const auto column {pos.columnIndex};

        if(column > 0)
            return Definitions::Position{pos.rowIndex, column - 1};

        return {};
    }

    case Definitions::MovementDirection::UpRight:
    {
        const auto [row, column] {pos};

        if(!row || column == m_columnsCount - 1)
            return {};

        return Definitions::Position{row - 1, column + 1};
    }

    case Definitions::MovementDirection::DownRight:
    {
        const auto [row, column] {pos};

        if(row == m_rowsCount - 1 || column == m_columnsCount - 1)
            return {};

        return Definitions::Position{row + 1, column + 1};
    }

    case Definitions::MovementDirection::DownLeft:
    {
        const auto [row, column] {pos};

        if(row == m_rowsCount - 1 || !column)
            return {};

        return Definitions::Position{row + 1, column - 1};
    }

    case Definitions::MovementDirection::UpLeft:
    {
        const auto [row, column] {pos};

        if(!row || !column)
            return {};

        return Definitions::Position{row - 1, column - 1};
    }

    default:
        return {};
    }
}

std::tuple<Definitions::Position, bool, std::optional<std::unordered_set<Definitions::Position>>>
GameStateMaintainer::finalDestinationPlusTrace(const Definitions::Position& sourcePos,
                                                 Definitions::MovementDirection directon,
                                                 bool needTrace,
                                                 bool duringGameGeneration) const
{
    std::unordered_set<Definitions::Position> trace;

    return finalDestinationPlusTraceHelper(sourcePos, directon, trace, needTrace, duringGameGeneration);
}

std::tuple<Definitions::Position, bool, std::unordered_set<Definitions::Position>>
GameStateMaintainer::finalDestinationPlusTraceHelper(const Definitions::Position& sourcePos,
                                                        Definitions::MovementDirection directon,
                                                        std::unordered_set<Definitions::Position>& trace,
                                                        bool needTrace,
                                                        bool duringGameGeneration) const
{
    auto startPos {sourcePos};
    std::optional<Definitions::Position> nextPos;

    while(true)
    {
        nextPos = nextCellPos(startPos, directon);

        if(!nextPos)
            return {startPos, true, std::move(trace)};

        Definitions::CellType nextCell;

        if(duringGameGeneration)
            nextCell = m_initialCells[nextPos->rowIndex][nextPos->columnIndex];

        else
            nextCell = m_cells[nextPos->rowIndex][nextPos->columnIndex];

        if(nextCell == Definitions::CellType::Stop)
        {
            if(needTrace)
                trace.insert(nextPos.value());

            return {nextPos.value(), true, std::move(trace)};
        }

        if(nextCell == Definitions::CellType::Wall)
            return {startPos, true, std::move(trace)};

        if(nextCell == Definitions::CellType::Mine)
            return {nextPos.value(), false, {}};

        if(needTrace)
            trace.insert(nextPos.value());

        startPos = nextPos.value();
    }
}

bool GameStateMaintainer::explodesWithAnyMove(const Definitions::Position& currentPos) const
{
    Definitions::Position finalPos;
    bool safeFinalDestination {false};

    for(const auto direction : Constants::kAllDirections)
    {
        std::tie(finalPos, safeFinalDestination, std::ignore) = finalDestinationPlusTrace(currentPos, direction, false);

        if(safeFinalDestination)
            return false;
    }

    return true;
}

bool GameStateMaintainer::explodesAfterPassingFrom(const Definitions::Position& currentPos) const
{
    bool result {true};
    Definitions::Position fianlPos;
    bool safeFinalDest {false};

    for(const auto direction : Constants::kAllDirections)
    {
        std::tie(fianlPos, safeFinalDest, std::ignore) = finalDestinationPlusTrace(currentPos, direction, false);

        if(safeFinalDest)
        {
            std::tie(std::ignore, safeFinalDest, std::ignore) = finalDestinationPlusTrace(fianlPos, InertiaUtility::oppositeDirection(direction), false);

            if(safeFinalDest)
            {
                result = false;
                break;
            }
        }
    }

    return result;
}

void GameStateMaintainer::announceBallPosition(QPointF ballPos)
{
    const auto rowIndex {ballPos.y()};
    const auto columnIndex {ballPos.x()};

    if(auto& cellType {m_cells[rowIndex][columnIndex]}; cellType == Definitions::CellType::Waiting)
    {
        cellType = Definitions::CellType::Clear;
        const auto cellIndex {index(rowIndex, columnIndex)};
        static_cast<InertiaModel*>(m_gameModel)->notifyDataChange(cellIndex, cellIndex);
    }
}

bool GameStateMaintainer::isClear(const Definitions::Position& pos) const
{
    return m_cells[pos.rowIndex][pos.columnIndex] == Definitions::CellType::Clear;
}

void GameStateMaintainer::resetCells(bool initializeCells)
{
    std::vector<std::vector<Definitions::CellType>>{}.swap(m_cells);
    m_cells.resize(m_rowsCount);

    for(auto& row : m_cells)
        if(initializeCells)
            row.resize(m_columnsCount, Definitions::CellType::Clear);
        else
            row.resize(m_columnsCount);
}

void GameStateMaintainer::restartGame()
{
    m_currentBallPos = m_initialBallPos;
    static_cast<InertiaModel*>(m_gameModel)->notifyBallPosChange(m_currentBallPos);
    m_onGameStart = true;

    beginResetModel();
    m_cells = m_initialCells;
    m_remainingGemsCount = m_gemsCount;
    endResetModel();
}

QString GameStateMaintainer::cellValuesToWrite() const
{
    QString result;

    for(quint32 rowIndex{}; rowIndex < m_rowsCount; ++rowIndex)
        for(quint32 columnIndex{}; columnIndex < m_columnsCount; ++columnIndex)
            result += QString("%1 ").arg(QString::number(static_cast<qint8>(m_cells[rowIndex][columnIndex])));

    return result;
}

QString GameStateMaintainer::initialCellValuesToWrite() const
{
    QString result;

    for(quint32 rowIndex{}; rowIndex < m_rowsCount; ++rowIndex)
        for(quint32 columnIndex{}; columnIndex < m_columnsCount; ++columnIndex)
            result += QString("%1 ").arg(QString::number(static_cast<qint8>(m_initialCells[rowIndex][columnIndex])));

    return result;
}

QString GameStateMaintainer::stuckAreaToWrite() const
{
    QString result;
    QTextStream stream{&result};

    stream << QString("%1 ").arg(m_stuckArea.size());

    for(const auto& pos : m_stuckArea)
        stream << QString("%1 %2 ").arg(pos.rowIndex).arg(pos.columnIndex);

    return result;
}

QString GameStateMaintainer::stuckAreaGemsToWrite() const
{
    QString result;
    QTextStream stream{&result};

    stream << QString("%1 ").arg(m_stuckAreaGems.size());

    for(const auto& pos : m_stuckAreaGems)
        stream << QString("%1 %2 ").arg(pos.rowIndex).arg(pos.columnIndex);

    return result;
}

std::pair<bool, Definitions::Position> GameStateMaintainer::gemReachableInOneMove(const Definitions::Position& currentPos,
                                                                                    Definitions::MovementDirection MoveDir) const
{
    auto [finalDest, safeFinalDest, trace] {finalDestinationPlusTrace(currentPos, MoveDir)};

    if(!safeFinalDest)
        return {false, finalDest};

    for(const auto& pos : trace.value())
        if(cellAt(pos.rowIndex, pos.columnIndex) == Definitions::CellType::Gem)
            return {true, finalDest};

    return {false, finalDest};
}

void GameStateMaintainer::findHintCandidateGems()
{
    std::unordered_set<Definitions::Position>{}.swap(m_hintCandidateGems);

    const auto& gems {StateWrapper::instance().state()->gemsPositions()};

    if(!m_stuckAreaGems.size())
        m_hintCandidateGems.insert(gems.cbegin(), gems.cend());

    else
    {
        auto gemPosesCopy {gems};
        std::sort(gemPosesCopy.begin(), gemPosesCopy.end());

        std::set_difference(gemPosesCopy.cbegin(),
                            gemPosesCopy.cend(),
                            m_stuckAreaGems.cbegin(),
                            m_stuckAreaGems.cend(),
                            std::inserter(m_hintCandidateGems, m_hintCandidateGems.end()));
    }
}

std::unordered_set<Definitions::Position>& GameStateMaintainer::hintCandidateGems()
{
    return m_hintCandidateGems;
}

const std::unordered_set<Definitions::Position>& GameStateMaintainer::hintCandidateGems() const
{
    return m_hintCandidateGems;
}

bool GameStateMaintainer::canEnterStuckArea() const
{
    auto gemsPosesCopy {m_gemsPositions};
    std::sort(gemsPosesCopy.begin(), gemsPosesCopy.end());

    return std::includes(std::execution::par_unseq,
                         m_stuckAreaGems.cbegin(),
                         m_stuckAreaGems.cend(),
                         gemsPosesCopy.cbegin(),
                         gemsPosesCopy.cend());
}

std::atomic<bool>& GameStateMaintainer::onGameStart()
{
    return m_onGameStart;
}

const std::atomic<bool>& GameStateMaintainer::onGameStart() const
{
    return m_onGameStart;
}

void GameStateMaintainer::updatePaths(quint32 rowsCount, quint32 columnsCount, const QString& filePath) const
{
    QFile file{m_GamesDataFilesPath + "/games-data-paths.txt"};

    if(!file.open(QIODevice::ReadWrite))
        throw std::runtime_error{
            QString("[GameStateMaintainer][pathForDimensions]: Could not open the games data file for reading/writing.\nReason: %1").
            arg(file.errorString()).toStdString()
        };

    QTextStream stream{&file};
    QString fileContent {stream.readAll()};

    QString searchPharse {QString("%1x%2").arg(QString::number(rowsCount)).arg(QString::number(columnsCount))};
    auto parts {fileContent.split('#', Qt::SkipEmptyParts)};

    for(auto it{parts.begin()}; it != parts.end(); ++it)
        if(it->startsWith(searchPharse))
        {
            parts.erase(it);
            break;
        }

    parts.emplaceBack(QString("%1:%2").arg(searchPharse).arg(filePath));
    auto fileNewContent {parts.join('#')};
    fileNewContent += '#';

    file.resize(0);
    file.write(fileNewContent.toLatin1());
}

QString GameStateMaintainer::pathForDimensions(quint32 rowsCount, quint32 columnsCount) const
{
    QString fileContent;

    {
        QFile file{m_GamesDataFilesPath + "/games-data-paths.txt"};

        if(!file.open(QIODevice::ReadOnly))
            throw std::runtime_error
                {
                    QString("[GameStateMaintainer][pathForDimensions]: Could not open the games data file for reading.\nReason: %1").
                    arg(file.errorString()).toStdString()
                };

        QTextStream stream{&file};
        fileContent = stream.readAll();
    }

    QString searchPharse {QString("%1x%2").arg(QString::number(rowsCount)).arg(QString::number(columnsCount))};
    const auto parts {fileContent.split('#', Qt::SkipEmptyParts)};

    for(const auto& part : parts)
        if(part.startsWith(searchPharse))
            return part.sliced(searchPharse.length() + 1);

    return {};
}

QString GameStateMaintainer::gamesDataFilesPath() const
{
    return m_GamesDataFilesPath;
}

void GameStateMaintainer::setGamesDataFilesPath(QString path)
{
    m_GamesDataFilesPath = std::move(path);
}
