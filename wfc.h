#ifndef WFC_H
#define WFC_H

#include "tile.h"
#include "view.h"
#include <QObject>
#include <QBitArray>

struct TileSlot{
    QBitArray tileIdBitset;
    int x, y;
    bool isCollapsed = false;
    bool isPermament = false;
};

struct Coords{
    int x, y; //can be negative?
};

class wfc : public QObject
{
    Q_OBJECT
public:
    explicit wfc(View *generatorView, QObject *parent = nullptr);

    View *generatorView;

    QList<Tile> tiles;
    QList<Pattern> patterns;
    QList<TileSlot> grid;
    QList<TileSlot> collapsableSlots;
    int gridWidth;
    int gridHeight;

    QList<TileSlot> clearGrid(const QList<Tile> &tiles, int width, int height); //fill whole grid with uncollapsed slots, add middle to collapsable list
    QList<TileSlot> generateGrid(QList<TileSlot> &grid, int width, int height); //the big one

    void collapseSlot(TileSlot &slot, const QList<Tile> &tiles); //collapses a slot using weighted random. returns tile id (should use pattern weights too?)
    QList<TileSlot> propagateUpdate(QList<TileSlot> &grid, const TileSlot &toCollapse, const QList<Pattern> &patterns, const QList<Tile> &tiles);
    //helpers
    QList<Coords> getAffectedPatternCoords(const TileSlot &slot, int patternSize); //return affected pattern coords (ones that contain uncollapsed slots)
    QList<TileSlot> getPatternTiles(const Coords &c, int patternSize, QList<TileSlot> &grid);
    TileSlot& getSlotAt(int x, int y);
    bool isInBounds(int x, int y);

public slots:
    void generate();

signals:


};

#endif // WFC_H
