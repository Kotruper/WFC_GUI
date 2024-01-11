#ifndef WAVEFUNCTIONCOLLAPSER_H
#define WAVEFUNCTIONCOLLAPSER_H

#include "qthread.h"
#include "tile.h"
#include "view.h"
#include <QObject>
#include <QBitArray>

struct TileSlot{
    QBitArray tileIdBitset; //change to patternIdBitset?
    QPoint pos;
    short collapsedId = -1;
    bool isPermament = false;
};

class WaveFunctionCollapser : public QObject
{
    Q_OBJECT
public:
    explicit WaveFunctionCollapser(View *generatorView, QObject *parent = nullptr);

    View *generatorView;

    QList<Tile> tiles;
    QList<Pattern> patterns;
    QList<TileSlot> grid;
    QList<QPoint> collapseCandidatePos;
    int gridWidth;
    int gridHeight;

    QList<TileSlot> createEmptyGrid(const QList<Tile> &tiles, int width, int height); //fill whole grid with uncollapsed slots, add middle to collapsable list
    QList<TileSlot> generateGridStep(QList<TileSlot> &grid, const QList<Tile> &tiles, int width, int height); //the big one

    void collapseSlot(QPoint slotPos, const QList<Tile> &tiles); //collapses a slot using weighted random. returns tile id (should use pattern weights too?)
    QList<QPoint> propagateUpdate(QList<TileSlot> &grid, const QPoint &collapsed, const QList<Pattern> &patterns, const QList<Tile> &tiles);
    //helpers
    QList<QPoint> getAffectedPatternCoords(const QPoint &p, int patternSize); //return affected pattern coords (ones that contain uncollapsed slots)
    QList<TileSlot> getPatternTiles(const QPoint &p, int patternSize, const QList<TileSlot> &grid);
    TileSlot& getSlotAt(const QPoint &pos);
    bool isInBounds(const QPoint &pos);

    void displayGrid(const QList<TileSlot> &grid, const QList<Tile> &tiles, int width, int height);

public slots:
    void generate();
    void generateOneStep();
    void setPatterns(QList<Tile> tiles, QList<Pattern> patterns);
    void changeGridHeight(int val);
    void changeGridWidth(int val);
    void clearGrid();

signals:


};

class WaveFunctionThread : public QThread
{
    Q_OBJECT
public:
    explicit WaveFunctionThread(const QList<TileSlot> &grid, const QList<Pattern> &patterns, int gridWidth, int gridHeight, QObject *parent = nullptr);

private:
    int gridWidth;
    int gridHeight;
    QList<TileSlot> grid;
    QList<QPoint> collapseCandidatePos;

    void collapseSlot(QPoint &slotPos, const QList<Pattern> &patterns); //collapses a slot using weighted random. returns tile id (should use pattern weights too?)
    QList<QPoint> propagateUpdate(QList<TileSlot> &grid, const QPoint &collapsed, const QList<Pattern> &patterns, const QList<Tile> &tiles);
    QList<QPoint> getAffectedPatternCoords(const QPoint &p, int patternSize);

    void run() override;

signals:
    void sendGrid(QList<TileSlot>);
    void finishedSuccessfully(); //add finished status?
};

#endif // WAVEFUNCTIONCOLLAPSER_H
