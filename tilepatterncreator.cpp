#include "tilepatterncreator.h"

TilePatternCreator::TilePatternCreator(QObject *parent)
    : QObject{parent}
{
    tiles = QList<Tile>(); //?
}

QList<Tile> TilePatternCreator::createTiles(QImage baseImage, int tileSize){
    QList<Tile> newTiles{};
    QList<int> IDmap{};

    int mapHeight = baseImage.height() / tileSize;
    int mapWidth = baseImage.width() / tileSize;

    for(int iY = 0; iY < mapHeight; iY++){
        for(int iX = 0; iX < mapWidth; iX++){
            const QImage potentialTile = baseImage.copy(iX*tileSize, iY*tileSize, tileSize, tileSize);
            int findIndex = newTiles.indexOf(potentialTile);
            if(findIndex == -1){
                Tile newTile = Tile(potentialTile,tileSize);
                newTiles.append(newTile);
                IDmap.append(newTile.id);
            }
            else{
                newTiles[findIndex].incrementWeight();
                IDmap.append(newTiles.at(findIndex).id);
            }
        }
    }
    qDebug()<<IDmap;
    return newTiles;
}

QList<Pattern> TilePatternCreator::createPatterns(QList<short> IDmap, int patternSize, int mapX, int mapY){
    QList<Pattern> newPatterns{};
    return newPatterns;
}
