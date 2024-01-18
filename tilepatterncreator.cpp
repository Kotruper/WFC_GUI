#include "tilepatterncreator.h"
#include "QFileDialog"
#include "qbitarray.h"

TilePatternCreator::TilePatternCreator(View *view, QObject *parent)
    : QObject{parent}, creatorView(view){
    tileSize = 5; //TODO: connect to a selector
    patternSize = 2; //TODO: connect to a selector
}

TilePatternThread::TilePatternThread(QImage baseImage, int tilePixelSize, int patternSize, QObject *parent)
    : QThread{parent}, baseImage(baseImage), tileSize(tilePixelSize), patternSize(patternSize){

}

void TilePatternCreator::updateTiles(QList<Tile> newTiles){
    //qDebug("got into creating");
    this->tiles = newTiles;

    auto displayTiles = [&](QList<Tile> tiles, int yOffset){
        for (int i = 0; i < tiles.size(); ++i) { //Display the tiles
            QGraphicsItem *item = new TileGraphicsItem(tiles.at(i));
            item->setPos(QPointF(i*(tileSize+1), yOffset));

            auto id = new QGraphicsSimpleTextItem(QString::number(tiles[i].id), item);
            //id->setPos(0,-tileSize);
            id->setBrush(QBrush(Qt::BrushStyle::SolidPattern));
            id->setScale(0.1);
            creatorView->view()->scene()->addItem(item);
            //qDebug()<<tiles[i].id;
        }
    };
    displayTiles(this->tiles, -10);
}

void TilePatternCreator::updatePatterns(QList<Pattern> newPatterns){
    this->patterns = newPatterns;
    //qDebug("got to patterns");

    auto displayPatterns = [&](QList<Pattern> patterns, int yOffset){
        for (int i = 0; i < patterns.size(); ++i) { //Display the tiles
            QGraphicsItem *item = new PatternGraphicsItem(patterns[i],tiles);
            item->setPos(QPointF(i*(patternSize*tileSize + tileSize), yOffset));

            auto id = new QGraphicsSimpleTextItem(QString::number(patterns[i].id), item);
            //id->setPos(0,-patternSize);
            id->setBrush(QBrush(Qt::BrushStyle::SolidPattern));
            id->setScale(0.1);
            creatorView->view()->scene()->addItem(item);
            //qDebug()<<tiles[i].id;
        }
    };
    displayPatterns(this->patterns, baseImage.height() + 10);
}

void TilePatternCreator::setImage(QString filename){ //will need some changes for the wall, maybe here or in tiledRead?
    creatorView->view()->scene()->clear();
    this->baseImage.load(filename);
    creatorView->view()->scene()->addPixmap(QPixmap::fromImage(baseImage));
}

void TilePatternCreator::extractPatterns(){
    creatorView->view()->scene()->clear();
    creatorView->view()->scene()->addPixmap(QPixmap::fromImage(baseImage));
    //lock ui except for cancel or smth

    auto activeTilePatternThread = new TilePatternThread(this->baseImage,this->tileSize,this->patternSize,this); //currently not deleted?

    connect(activeTilePatternThread, &TilePatternThread::sendTiles, this, &TilePatternCreator::updateTiles);
    connect(activeTilePatternThread, &TilePatternThread::sendPatterns, this, &TilePatternCreator::updatePatterns);
    connect(activeTilePatternThread, &TilePatternThread::finishedSuccessfully, this, &TilePatternCreator::exportPatterns);
    connect(activeTilePatternThread, &TilePatternThread::finished, activeTilePatternThread, &TilePatternThread::deleteLater);
    activeTilePatternThread->start();
}

void TilePatternCreator::exportPatterns(){
    emit patternsSignal(this->tiles, this->patterns);
    //unlock ui. will also need to do if interrupted
}

void TilePatternCreator::setPatternSize(int size){
    this->patternSize = size;
}

void TilePatternCreator::setTileSize(int size){
    this->tileSize = size;
}

//////////////////////////////////////////////////////////////////

QList<Tile> TilePatternThread::generateTiles(QImage baseImage, int tileSize){ //sideEfect: fills the idMap
    QList<Tile> newTiles{};

    int mapHeight = baseImage.height() / tileSize;
    int mapWidth = baseImage.width() / tileSize;

    idMap.clear();

    for(int iY = 0; iY < mapHeight; iY++){
        for(int iX = 0; iX < mapWidth; iX++){
            const QImage potentialTile = baseImage.copy(iX*tileSize, iY*tileSize, tileSize, tileSize);
            int findIndex = newTiles.indexOf(potentialTile);
            if(findIndex == -1){
                Tile newTile = Tile(newTiles.size(), potentialTile, tileSize); //maybe rethink id
                newTiles.append(newTile);
                idMap.append(newTile.id);
            }
            else{
                newTiles[findIndex].incrementWeight();
                idMap.append(newTiles.at(findIndex).id);
            }

            emit sendTiles(newTiles);
            if(this->isInterruptionRequested())
                return newTiles;
        }
    }
    qDebug()<<"IdMap: "<<idMap;
    return newTiles;
}

void TilePatternThread::updatePatternCompability(QList<Pattern> &patterns, int patternSize){
    for(Pattern &pRef: patterns){
        for(int dy = -patternSize + 1; dy < patternSize; dy++){
            for(int dx = -patternSize + 1; dx < patternSize; dx++){
                auto &compListRef = pRef.getCompabilityListRefAt({dx, dy});
                compListRef = QBitArray(patterns.size());
                if((dx==0) && (dy==0)) continue; //skip self. remember to skip in wfc as well
                for(const Pattern &otherP: patterns){
                    if(pRef.isCompatibleAt(otherP, {dx, dy}))
                        compListRef.setBit(otherP.id); //bitset

                    if(this->isInterruptionRequested())
                        return;
                }
            }
        }
    }
}

QList<Pattern> TilePatternThread::generatePatterns(QList<short> IDmap, int patternSize){

    QList<Pattern> newPatterns{};
    int mapHeight = baseImage.height() / tileSize;
    int mapWidth = baseImage.width() / tileSize;

    auto getAdjacent = [&](int x, int y){ //wrapping implemented
        QList<short> adjacent{};
        for(int dy = 0; dy < patternSize; dy++){
            for(int dx = 0; dx < patternSize; dx++){
                int wrappedX = (x + dx) % mapWidth;
                int wrappedY = (y + dy) % mapHeight;
                adjacent.append(IDmap.at(wrappedX + wrappedY * mapWidth));
            }
        }
        return adjacent;
    };

    int patternYlimit = mapHeight;// - (patternSize - 1); //will need to change with wrapping to just mapHeight
    int patternXlimit = mapWidth;// - (patternSize - 1); //will need to change with wrapping

    for(int iY = 0; iY < patternYlimit; iY++){ //pattern y limit
        for(int iX = 0; iX < patternXlimit; iX++){ //pattern x limit
            QList<short> potentialPattern = getAdjacent(iX, iY);
            int findIndex = newPatterns.indexOf(potentialPattern);
            if(findIndex == -1){
                Pattern newPattern = Pattern(newPatterns.size(), potentialPattern, patternSize);
                newPatterns.append(newPattern);
            }
            else{
                newPatterns[findIndex].incrementWeight();
            }

            emit sendPatterns(newPatterns);
            if(this->isInterruptionRequested())
                return newPatterns;
            //this->msleep(500); //holy shit this works
        }
    }
    updatePatternCompability(newPatterns, patternSize);
    emit sendPatterns(newPatterns);
    return newPatterns;
}

void TilePatternThread::run(){
    generateTiles(this->baseImage,this->tileSize);
    generatePatterns(this->idMap, this->patternSize);
    if(!this->isInterruptionRequested())
        emit finishedSuccessfully();
}
