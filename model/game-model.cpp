#include "game-model.hpp"
#include "state-wrapper.hpp"
#include "game-generator/game-generator.hpp"
#include "service/move-handler.hpp"

#include <QThread>


using namespace Definitions;


InertiaModel::InertiaModel(QObject *parent) : QAbstractTableModel(parent),
                                                m_gameGenerator(std::make_unique<GameGenerator>()),
                                                m_moveHandler(std::make_unique<MoveHandler>())
{
    qRegisterMetaType<InertiaModel*>("InertiaModel *");
}

InertiaModel::~InertiaModel() = default;

void InertiaModel::setRowsCount(quint32 newRowsCount)
{
    StateWrapper::instance().state()->setRowsCount(newRowsCount, false);
}

void InertiaModel::setColumnsCount(quint32 columnsCount)
{
    StateWrapper::instance().state()->setColumnsCount(columnsCount, false);
}

void InertiaModel::setBallPosition(QPointF pos)
{
    StateWrapper::instance().state()->setBallPosition(pos);
}

int InertiaModel::rowCount(const QModelIndex& parentIndex) const
{
    Q_UNUSED(parentIndex);
    return StateWrapper::instance().state()->rowsCount();
}

int InertiaModel::columnCount(const QModelIndex& parentIndex) const
{
    Q_UNUSED(parentIndex);
    return StateWrapper::instance().state()->columnsCount();
}

QVariant InertiaModel::data(const QModelIndex& index, int role) const
{
    return StateWrapper::instance().state()->data(index, role);
}

quint32 InertiaModel::rowsCount() const
{
    return StateWrapper::instance().state()->rowsCount();
}

quint32 InertiaModel::columnsCount() const
{
    return StateWrapper::instance().state()->columnsCount();
}

QPointF InertiaModel::ballPosition() const
{
    return StateWrapper::instance().state()->ballPositionPoint();
}

void InertiaModel::initializeModel(bool storeInFile,
                                    const QString& filePath,
                                    std::optional<quint64> toBeGeneratedGamesCount)
{
    auto thread {QThread::create(&GameGenerator::initializeModel,
                                 m_gameGenerator.get(),
                                 storeInFile,
                                 filePath,
                                 toBeGeneratedGamesCount)};

    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start(QThread::TimeCriticalPriority);
}

MovementResult InertiaModel::moveBall(MovementDirection direction)
{
    return m_moveHandler->moveBall(direction);
}

void InertiaModel::generateAllGames(quint32 rowsCount,
                                     quint32 columnsCount,
                                     quint64 gamesCount,
                                     const QString& filePath)
{
    m_gameGenerator->generateAllGames(rowsCount, columnsCount, gamesCount, filePath);
}

void InertiaModel::newGameFromFile(quint32 rowsCount, quint32 columnsCount)
{
    auto thread {QThread::create(&GameGenerator::newGameFromFile,
                                 m_gameGenerator.get(),
                                 rowsCount,
                                 columnsCount)};

    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start(QThread::TimeCriticalPriority);
}

void InertiaModel::undo(QPointF preMovePos, QList<QPointF> pickedGems)
{
    m_moveHandler->undo(preMovePos, pickedGems);
}

QString InertiaModel::stuckAreaToWrite() const
{
    return StateWrapper::instance().state()->stuckAreaToWrite();
}

QString InertiaModel::stuckAreaGemsToWrite() const
{
    return StateWrapper::instance().state()->stuckAreaGemsToWrite();
}

QString InertiaModel::cellValuesToWrite() const
{
    return StateWrapper::instance().state()->cellValuesToWrite();
}

QString InertiaModel::initialCellValuesToWrite() const
{
    return StateWrapper::instance().state()->initialCellValuesToWrite();
}

void InertiaModel::loadSavedGame(quint32 rowsCount,
                                  quint32 columnsCount,
                                  QPointF ballPos,
                                  QString stuckAreaData,
                                  QString stuckAreaGemsData,
                                  QString initialCellTypesData,
                                  QString cellTypesData)
{
    auto thread {QThread::create(&GameGenerator::loadSavedGame,
                                 m_gameGenerator.get(),
                                 rowsCount,
                                 columnsCount,
                                 ballPos,
                                 stuckAreaData,
                                 stuckAreaGemsData,
                                 initialCellTypesData,
                                 cellTypesData)};

    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start(QThread::TimeCriticalPriority);
}

void InertiaModel::restartGame()
{
    m_gameGenerator->resetStopParam();
    auto thread {QThread::create(&GameStateMaintainer::restartGame, StateWrapper::instance().state())};
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start(QThread::TimeCriticalPriority);
}

void InertiaModel::announceBallPosition(QPointF ballPos)
{
    StateWrapper::instance().state()->announceBallPosition(ballPos);
}

void InertiaModel::hint()
{
    m_moveHandler->hint();
}

void InertiaModel::notifyDataChange(const QModelIndex& topLeft, const QModelIndex& downRight)
{
    emit dataChanged(topLeft, downRight);
}

void InertiaModel::notifyBallPosChange(const Definitions::Position& ballPos)
{
    emit ballPositionChanged(QPointF(ballPos.columnIndex, ballPos.rowIndex));
}

void InertiaModel::notifyBallPosChange(const QPointF& ballPos)
{
    emit ballPositionChanged(ballPos);
}

void InertiaModel::notifyGameCompletion()
{
    emit gameCompleted();
}

void InertiaModel::notifyRowsCountChange(quint32 newRowsCount)
{
    emit rowsCountChanged(newRowsCount);
}

void InertiaModel::notifyColumnsCountChange(quint32 newColumnsCount)
{
    emit columnsCountChanged(newColumnsCount);
}

void InertiaModel::notifyHint(Definitions::MovementDirection moveDir)
{
    emit showHint(moveDir);
}

void InertiaModel::notifyDataModificationStart()
{
    beginResetModel();
}

void InertiaModel::notifyDataModificationEnd()
{
    endResetModel();
}

void InertiaModel::notifyGameGenerationCompletion(quint64 gamesGenerated)
{
    emit gameGenerationCompleted(gamesGenerated);
}
