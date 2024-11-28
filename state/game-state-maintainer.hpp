#pragma once

#include "game-model.hpp"

#include <QtQml/qqmlregistration.h>

#include <unordered_set>


class GameStateMaintainer : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(InertiaModel* model READ model WRITE setModel)
    Q_PROPERTY(QString gamesDataFilesPath READ gamesDataFilesPath WRITE setGamesDataFilesPath FINAL)

public:

    GameStateMaintainer(QObject* parent = nullptr);

    GameStateMaintainer(const GameStateMaintainer&) = delete;
    GameStateMaintainer(GameStateMaintainer&&) = delete;
    GameStateMaintainer& operator=(GameStateMaintainer&) = delete;
    GameStateMaintainer& operator=(GameStateMaintainer&&) = delete;

    std::vector<std::vector<Definitions::CellType>>& cells();
    const std::vector<std::vector<Definitions::CellType>>& cells() const;
    Definitions::CellType& cellAt(quint32 rowIndex, quint32 columnIndex);
    const Definitions::CellType& cellAt(quint32 rowIndex, quint32 columnIndex) const;
    std::vector<std::vector<Definitions::CellType>>& initialCells();
    const std::vector<std::vector<Definitions::CellType>>& initialCells() const;

    quint32 rowsCount() const;
    quint32 columnsCount() const;
    void setRowsCount(quint32 newRowsCount, bool emitSignal = true);
    void setColumnsCount(quint32 columnsCount, bool emitSignal = true);
    QModelIndex index(quint32 rowIndex, quint32 columnIndex);
    QVariant data(const QModelIndex& index, int role) const;

    void beginResetModel();
    void endResetModel();
    void resetGameData(quint32 rowsCount, quint32 columnsCount);
    void restartGame();
    void resetCells(bool initializeCells = true);

    Definitions::Position& ballPos();
    const Definitions::Position& ballPos() const;
    QPointF ballPositionPoint() const;
    void setBallPosition(QPointF pos);
    Definitions::Position& initialBallPos();
    const Definitions::Position& initialBallPos() const;

    void notifyBallPosChange(const Definitions::Position& ballPos);
    void notifyDataChange(const QModelIndex& topLeft, const QModelIndex& downRight) const;
    void notifyGameGenerationCompletion(quint64 gamesGenerated);
    void notifyGameCompletion();

    quint32& gemsCount();
    const quint32& gemsCount() const;
    std::vector<Definitions::Position>& gemsPositions();
    const std::vector<Definitions::Position>& gemsPositions() const;
    quint32& remainingGemsCount();


    void showHint(Definitions::MovementDirection moveDir);
    std::unordered_set<Definitions::Position>& stuckArea();
    const std::unordered_set<Definitions::Position>& stuckArea() const;
    std::vector<Definitions::Position>& stuckAreaGems();
    const std::vector<Definitions::Position>& stuckAreaGems() const;

    std::optional<Definitions::Position> nextCellPos(const Definitions::Position& currentPos,
                                                     Definitions::MovementDirection direction) const;

    std::tuple<Definitions::Position, bool, std::optional<std::unordered_set<Definitions::Position>>>
    finalDestinationPlusTrace(const Definitions::Position& sourcePos,
                                Definitions::MovementDirection directon,
                                bool needTrace = true,
                                bool duringGameGeneration = false) const;
    bool explodesAfterPassingFrom(const Definitions::Position& currentPos) const;
    bool isClear(const Definitions::Position& pos) const;

    void announceBallPosition(QPointF ballPos);
    QString cellValuesToWrite() const;
    QString initialCellValuesToWrite() const;
    QString stuckAreaToWrite() const;
    QString stuckAreaGemsToWrite() const;

    void findHintCandidateGems();
    std::unordered_set<Definitions::Position>& hintCandidateGems();
    const std::unordered_set<Definitions::Position>& hintCandidateGems() const;
    bool canEnterStuckArea() const;

    std::atomic<bool>& onGameStart();
    const std::atomic<bool>& onGameStart() const;

    void updatePaths(quint32 rowsCount, quint32 columnsCount, const QString& filePath) const;
    QString pathForDimensions(quint32 rowsCount, quint32 columnsCount) const;
    QString gamesDataFilesPath() const;


public slots:

    void setModel(InertiaModel* model);
    void setGamesDataFilesPath(QString path);
private:

    InertiaModel* model() const;

    const quint32& remainingGemsCount() const;
    std::optional<Definitions::CellType> nextCell(const Definitions::Position& pos,
                                                  Definitions::MovementDirection direction) const;

    std::tuple<Definitions::Position, bool, std::unordered_set<Definitions::Position>>
    finalDestinationPlusTraceHelper(const Definitions::Position& sourcePos,
                                       Definitions::MovementDirection directon,
                                       std::unordered_set<Definitions::Position>& trace,
                                       bool needTrace,
                                       bool duringGameGeneration) const;

    bool explodesWithAnyMove(const Definitions::Position& currentPos) const;
    std::pair<bool, Definitions::Position> gemReachableInOneMove(const Definitions::Position& currentPos,
                                                                   Definitions::MovementDirection MoveDir) const;
    void notifyBallPosChange(const QPointF& ballPos);


    quint32 m_rowsCount{}, m_columnsCount{};
    Definitions::Position m_currentBallPos, m_initialBallPos;
    quint32 m_gemsCount, m_remainingGemsCount{};
    std::vector<std::vector<Definitions::CellType>> m_cells, m_initialCells;
    std::unordered_set<Definitions::Position> m_stuckArea;
    std::vector<Definitions::Position> m_stuckAreaGems;
    std::vector<Definitions::Position> m_gemsPositions;
    std::unordered_set<Definitions::Position> m_hintCandidateGems;
    std::atomic<bool> m_onGameStart {true};
    QString m_GamesDataFilesPath;
    InertiaModel* m_gameModel{};

};
