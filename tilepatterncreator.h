#ifndef TILEPATTERNCREATOR_H
#define TILEPATTERNCREATOR_H

#include <QObject>
#include "tile.h"
#include "view.h"

class TilePatternCreator : public QObject
{
    Q_OBJECT
public:
    explicit TilePatternCreator(View *creatorView, QObject *parent = nullptr);

    View *creatorView;

    QList<Tile> tiles; //while creating tiles, insert only unique
    QList<Pattern> patterns; //while creating patterns, insert only unique, count duplicates
    QList<short> idMap;
    QImage baseImage;
    int tileSize;
    int patternSize;

    QList<Pattern> createPatterns(QList<short> idMap, int patternSize);
    QList<Tile> createTiles(QImage baseImage, int tileSize);

signals:

public slots:
    void createPatterns();
    void createTiles();
    void setImage(QString filename);
};

#endif // TILEPATTERNCREATOR_H
