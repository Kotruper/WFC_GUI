#ifndef TILEPATTERNCREATOR_H
#define TILEPATTERNCREATOR_H

#include <QObject>
#include "tile.h"

class TilePatternCreator : public QObject
{
    Q_OBJECT
public:
    explicit TilePatternCreator(QObject *parent = nullptr);
    QList<Tile> tiles; //while creating tiles, insert only unique
    QList<Pattern> patterns; //while creating patterns, insert only unique, count duplicates
    QImage baseImage;
    int tileSize;

    QList<Pattern> createPatterns(QList<short> idMap, int patternSize, int mapX, int mapY);
    QList<Tile> createTiles(QImage baseImage, int tileSize);

signals:

public slots:

};

#endif // TILEPATTERNCREATOR_H
