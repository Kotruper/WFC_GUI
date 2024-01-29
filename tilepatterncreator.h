#ifndef TILEPATTERNCREATOR_H
#define TILEPATTERNCREATOR_H

#include <QObject>
#include <QThread>
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
    QImage baseImage;
    int tileSize;
    int patternSize;
    WallPos wallPos = WallPos::BothWall; //both walls means no perceived looping

signals:
    void patternsSignal(QList<Tile> tiles, QList<Pattern> patterns, WallPos wallPos);

public slots:
    void extractPatterns();
    void setImage(QString filename);
    void setTileSize(int size);
    void setPatternSize(int size);
    void toggleXWall();
    void toggleYWall();

    void updateTiles(QList<Tile> tiles);
    void updatePatterns(QList<Pattern> patterns);
    void exportPatterns();
};

/////////////////////////////////////////

class TilePatternThread : public QThread
{
    Q_OBJECT
public:
    explicit TilePatternThread(TilePatternCreator* creator, QObject *parent = nullptr);

private:
    TilePatternCreator* creator;
    QList<short> idMap;
    QImage baseImage;
    int tileSize;
    int patternSize;
    WallPos wallPos;

    QList<Pattern> generatePatterns(QList<short> idMap, int patternSize);
    QList<Tile> generateTiles(QImage baseImage, int tileSize);
    void updatePatternCompability(QList<Pattern> &patterns, int patternSize);

    void run() override;

signals:
    void sendTiles(QList<Tile>);
    void sendPatterns(QList<Pattern>);
    void finishedSuccessfully(); //add finished status?
};

#endif // TILEPATTERNCREATOR_H
