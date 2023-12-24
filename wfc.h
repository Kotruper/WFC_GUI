#ifndef WFC_H
#define WFC_H

#include "tile.h"
#include "view.h"
#include <QObject>

typedef QList<short> TileSlot;

struct TileSlot2{
    QList<short> ids;
    bool isPermament = false;

};

class wfc : public QObject
{
    Q_OBJECT
public:
    explicit wfc(View *creatorView, QObject *parent = nullptr);
    QList<Tile> tiles;
    QList<Pattern> patterns;
    QList<TileSlot> grid;
    int gridWidth;
    int gridHeight;

    QList<TileSlot> clearGrid(int width, int height);
    QList<TileSlot> generateGrid(QList<TileSlot> &grid,int width, int height);



public slots:
    void generate();

signals:


};

#endif // WFC_H
