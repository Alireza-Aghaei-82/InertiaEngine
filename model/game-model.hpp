#pragma once

#include "movement-result.hpp"
#include "common-definitions.hpp"

#include <QAbstractTableModel>
#include <QtQml/qqmlregistration.h>
#include <QPointF>


class GameStateMaintainer;
class GameGenerator;
class MoveHandler;
class HintHandler;


class InertiaModel : public QAbstractTableModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(quint32 rowsCount READ rowsCount WRITE setRowsCount NOTIFY rowsCountChanged FINAL)
    Q_PROPERTY(quint32 columnsCount READ columnsCount WRITE setColumnsCount NOTIFY columnsCountChanged FINAL)
    Q_PROPERTY(QPointF ballPosition READ ballPosition WRITE setBallPosition NOTIFY ballPositionChanged FINAL)

public:

    explicit InertiaModel(QObject *parent = nullptr);
    ~InertiaModel();

    int rowCount(const QModelIndex& parentIndex) const override;
    int columnCount(const QModelIndex& parentIndex) const override;
    QVariant data(const QModelIndex& index, int role) const override;

    quint32 rowsCount() const;
    quint32 columnsCount() const;
    QPointF ballPosition() const;

    Q_INVOKABLE void initializeModel(bool storeInFile = false,
                                      const QString& filePath = {},
                                      std::optional<quint64> toBeGeneratedGamesCount = {});

    Q_INVOKABLE MovementResult moveBall(Definitions::MovementDirection direction);

    Q_INVOKABLE void generateAllGames(quint32 rowsCount,
                                       quint32 columnsCount,
                                       quint64 gamesCount,
                                       const QString& filePath);

    Q_INVOKABLE void newGameFromFile(quint32 rowsCount, quint32 columnsCount);

    Q_INVOKABLE void undo(QPointF preMovePos, QList<QPointF> pickedGems);
    Q_INVOKABLE QString stuckAreaToWrite() const;
    Q_INVOKABLE QString stuckAreaGemsToWrite() const;
    Q_INVOKABLE QString cellValuesToWrite() const;
    Q_INVOKABLE QString initialCellValuesToWrite() const;

    Q_INVOKABLE void loadSavedGame(quint32 rowsCount,
                                    quint32 columnsCount,
                                    QPointF ballPos,
                                    QString stuckAreaData,
                                    QString stuckAreaGemsData,
                                    QString initialCellTypesData,
                                    QString cellTypesData);

    Q_INVOKABLE void restartGame();
    Q_INVOKABLE void announceBallPosition(QPointF ballPos);
    Q_INVOKABLE void hint();

    void notifyDataChange(const QModelIndex& topLeft, const QModelIndex& downRight);
    void notifyBallPosChange(const Definitions::Position& ballPos);
    void notifyBallPosChange(const QPointF& ballPos);
    void notifyGameCompletion();
    void notifyRowsCountChange(quint32 newRowsCount);
    void notifyColumnsCountChange(quint32 newColumnsCount);
    void notifyHint(Definitions::MovementDirection moveDir);
    void notifyDataModificationStart();
    void notifyDataModificationEnd();
    void notifyGameGenerationCompletion(quint64 gamesGenerated);

public slots:

    void setRowsCount(quint32 newRowsCount);
    void setColumnsCount(quint32 columnsCount);
    void setBallPosition(QPointF pos);

signals:

    void rowsCountChanged(quint32 rowsCount);
    void columnsCountChanged(quint32 columnsCount);
    void ballPositionChanged(QPointF newPos);
    void gameCompleted();
    void gameGenerationCompleted(quint64 gamesGenerated);
    void stuck();
    void showHint(Definitions::MovementDirection direction);

private:

    std::unique_ptr<GameGenerator> m_gameGenerator{};
    std::unique_ptr<MoveHandler> m_moveHandler{};
};
