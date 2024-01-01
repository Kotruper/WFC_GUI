#include "tilepatterncreator.h"
#include "QFileDialog"
#include "qvectornd.h"

TilePatternCreator::TilePatternCreator(View *view, QObject *parent)
    : QObject{parent}, creatorView(view){
    tileSize = 5; //TODO: connect to a selector
    patternSize = 2; //TODO: connect to a selector
}

void TilePatternCreator::createTiles(){
    //qDebug("got into creating");
    this->tiles = createTiles(baseImage, tileSize);

    auto displayTiles = [&](QList<Tile> tiles, int yOffset){
        for (int i = 0; i < tiles.size(); ++i) { //Display the tiles
            QGraphicsItem *item = new TileGraphicsItem(tiles.at(i));
            item->setPos(QPointF(i*(tileSize+1), -20));
            creatorView->view()->scene()->addItem(item);
            //qDebug()<<tiles[i].id;
        }
    };
    displayTiles(this->tiles, 20);
}

QList<Tile> TilePatternCreator::createTiles(QImage baseImage, int tileSize){ //sideEfect: fills the idMap
    QList<Tile> newTiles{};

    int mapHeight = baseImage.height() / tileSize;
    int mapWidth = baseImage.width() / tileSize;

    idMap.clear();

    for(int iY = 0; iY < mapHeight; iY++){
        for(int iX = 0; iX < mapWidth; iX++){
            const QImage potentialTile = baseImage.copy(iX*tileSize, iY*tileSize, tileSize, tileSize);
            int findIndex = newTiles.indexOf(potentialTile);
            if(findIndex == -1){
                Tile newTile = Tile(potentialTile, tileSize, newTiles.size()); //maybe rethink id
                newTiles.append(newTile);
                idMap.append(newTile.id);
            }
            else{
                newTiles[findIndex].incrementWeight();
                idMap.append(newTiles.at(findIndex).id);
            }
        }
    }
    qDebug()<<"IdMap: "<<idMap;
    return newTiles;
}

void TilePatternCreator::createPatterns(){
    this->patterns = createPatterns(idMap, patternSize);
    qDebug("got to patterns");

    auto displayPatterns = [&](QList<Pattern> patterns, int yOffset){
        for (int i = 0; i < patterns.size(); ++i) { //Display the tiles
            QGraphicsItem *item = new PatternGraphicsItem(patterns[i],tiles);
            item->setPos(QPointF(i*(patternSize*tileSize + 1), yOffset));
            creatorView->view()->scene()->addItem(item);
            //qDebug()<<tiles[i].id;
        }
    };
    displayPatterns(this->patterns, 30);
}

QList<Pattern> TilePatternCreator::createPatterns(QList<short> IDmap, int patternSize){


    QList<Pattern> newPatterns{};
    int mapHeight = baseImage.height() / tileSize;
    int mapWidth = baseImage.width() / tileSize;

    auto getAdjacent = [&](int x, int y){
        QList<short> adjacent{};
        for(int iY = 0; iY < patternSize; iY++){
            for(int iX = 0; iX < patternSize; iX++){
                adjacent.append(IDmap.at(x + iX + (y + iY) * (mapWidth - 0)));
            }
        }
        return adjacent;
    };

    for(int iY = 0; iY < mapHeight - (patternSize - 1); iY++){ //pattern y limit
        for(int iX = 0; iX < mapWidth - (patternSize - 1); iX++){ //pattern x limit
            QList<short> potentialPattern = getAdjacent(iX, iY);
            int findIndex = newPatterns.indexOf(potentialPattern);
            if(findIndex == -1){
                Pattern newPattern = Pattern(potentialPattern, patternSize, 1, newPatterns.size());
                newPatterns.append(newPattern);
            }
            else{
                newPatterns[findIndex].incrementWeight();
            }
        }
    }
    return newPatterns;
}

void TilePatternCreator::setImage(QString filename){
    creatorView->view()->scene()->clear();
    this->baseImage.load(filename);
    creatorView->view()->scene()->addPixmap(QPixmap::fromImage(baseImage));
}

void TilePatternCreator::exportPatterns(){
    emit patternsSignal(this->tiles, this->patterns);
}
