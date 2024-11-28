#pragma once

#include "common-definitions.hpp"

#include <QFile>
#include <unordered_set>


class QTimer;


class GameGenerator : public QObject
{
    Q_OBJECT

public:

    void generateAllGames(quint32 rowsCount,
                           quint32 columnsCount,
                           quint64 gamesCount,
                           const QString& filePath);

    void initializeModel(bool storeInFile = false,
                          const QString& filePath = {},
                          std::optional<quint64> toBeGeneratedGamesCount = {});

    void newGameFromFile(quint32 rowsCount, quint32 columnsCount);

    void loadSavedGame(quint32 rowsCount,
                        quint32 columnsCount,
                        QPointF ballPosPoint,
                        QString stuckAreaData,
                        QString stuckAreaGemsData,
                        QString initialCellTypesData,
                        QString cellTypesData);


    void resetStopParam();

public slots:

    void setStopperVar();

private:

    void placeBall();
    void placeObstacles(bool plantMines);
    void buildUpWalls();
    void plantMines();
    void placeGems();

    quint64 placeStops(QTimer* stopsSelectortimer,
                        QTextStream* fileStream = {},
                        std::optional<quint64> targetedGamesCount = {});

    std::vector<std::vector<bool>> obstaclesInitialCandidates(const std::vector<Definitions::Position>& walls,
                                                                bool plantMines) const;

    void placeObstaclesHelper(std::vector<std::vector<bool>>& properCells,
                                std::vector<Definitions::Position>& walls,
                                bool plantMines);
    std::vector<Definitions::Position> obstacleCandidatePositionsGenerator(const std::vector<std::vector<bool>>& properCells);
    std::vector<Definitions::Position> stopCandidateCells() const;

    std::pair<std::optional<std::unordered_set<Definitions::Position>>, std::unordered_set<Definitions::Position>>
    visitableCells(const Definitions::Position& currentPosition, bool onlyStoppedByCells = false) const;

    void visitableCellsHelper(const Definitions::Position& startPos,
                                std::unordered_set<Definitions::Position>& processedSourcePositions,
                                std::unordered_set<Definitions::Position>& currentResult,
                                std::optional<Definitions::Position>& stuckInPos,
                                std::optional<Definitions::MovementDirection> excludedDir = {}) const;

    void applyStopPattern(const std::vector<Definitions::Position>& stopPattern);

    quint64 generateTryStopPatterns(const std::vector<Definitions::Position>& availableCells,
                                      QTimer* stopsSelectortimer,
                                      QTextStream* fileStream = {},
                                      std::optional<quint64> targetedGamesCount = {});

    std::vector<bool> generateStopsSelectionsModel(std::size_t availableCellsCount, bool generateAllGames) const;

    bool isConnectedTo(const Definitions::Position& sourcePos,
                        const Definitions::Position& targetPos) const;

    bool isConnectedToHelper(const Definitions::Position& sourcePos,
                              const Definitions::Position& targetPos,
                              std::unordered_set<Definitions::Position>& alreadyChecked) const;

    std::vector<Definitions::Position> nonObstacleNeighbours(const Definitions::Position& pos,
                                                               std::unordered_set<Definitions::Position>& alreadyChecked) const;
    void resetGameData(quint32 rowsCount, quint32 columnsCount);
    void loadGameFromData(QString& gameData);
    void resetStopperVar();
    QTimer* createStopsSelectorTimer(bool generateMultipleGames);
    void storeInFile(QTextStream* file);
    std::pair<bool, std::unordered_set<Definitions::Position>>
    checkSolvability(const std::unordered_set<Definitions::Position>& originalStoppedByCells);
    std::vector<Definitions::Position> findStuckAreaGems() const;

    QFile m_file{};
    std::atomic_bool m_stopNewGameGeneration {false};
};
