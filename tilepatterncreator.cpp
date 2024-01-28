#include "tilepatterncreator.h"
#include "QFileDialog"
#include "qbitarray.h"

/* //nah, I dunno
void TPC_Scene::dropEvent(QGraphicsSceneDragDropEvent* e){
    if (e->mimeData()->hasImage()) {
        qDebug()<<"Drop detected!";
        auto tpc = (TilePatternCreator*)parent();
        tpc->setImage(e->mimeData()->urls().first().toLocalFile());
        e->accept();
    } else {
        QGraphicsScene::dropEvent(e);
    }
}
void TPC_Scene::dragEnterEvent(QGraphicsSceneDragDropEvent* e){
    e->accept();
    QGraphicsScene::dragEnterEvent(e);
}

void TPC_Scene::dragMoveEvent(QGraphicsSceneDragDropEvent* e){
    e->accept();
    QGraphicsScene::dragMoveEvent(e);
}
*/
TilePatternCreator::TilePatternCreator(View *view, QObject *parent)
    : QObject{parent}, creatorView(view){
    setTileSize(1);
    setPatternSize(3);
    //creatorView->setAcceptDrops(true);
    //creatorView->view()->setAcceptDrops(true);
}
/*
TilePatternThread::TilePatternThread(QImage baseImage, int tilePixelSize, int patternSize, bool wallX, bool wallY, bool repeatX, bool repeatY, QObject *parent)
    : QThread{parent}, baseImage(baseImage), tileSize(tilePixelSize), patternSize(patternSize), wallX(wallX), wallY(wallY), repeatX(repeatX), repeatY(repeatY)
{
}
*/

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
            id->setPos(0, -2);
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
            item->setPos(QPointF((i%20)*(patternSize*tileSize + tileSize), yOffset + (i/20)*(patternSize*tileSize + tileSize)));

            auto id = new QGraphicsSimpleTextItem(QString::number(patterns[i].id), item);
            //id->setPos(0,-patternSize);
            id->setBrush(QBrush(Qt::BrushStyle::SolidPattern));
            id->setScale(0.1);
            id->setPos(0, -2);
            creatorView->view()->scene()->addItem(item);
            //qDebug()<<tiles[i].id;
        }
    };
    displayPatterns(this->patterns, baseImage.height() + 10);
}

void TilePatternCreator::setImage(QString filename){ //will need some changes for the wall, maybe here or in tiledRead?
    auto currentScene = creatorView->view()->scene();
    currentScene->clear();
    this->baseImage.load(filename);
    currentScene->addPixmap(QPixmap::fromImage(baseImage));
    creatorView->view()->fitInView(baseImage.rect(), Qt::AspectRatioMode::KeepAspectRatio);
}

void TilePatternCreator::extractPatterns(){
    creatorView->view()->scene()->clear();
    creatorView->view()->scene()->addPixmap(QPixmap::fromImage(baseImage));
    //lock ui except for cancel or smth

    auto activeTilePatternThread = new TilePatternThread(this,this); //currently not deleted?

    connect(activeTilePatternThread, &TilePatternThread::sendTiles, this, &TilePatternCreator::updateTiles);
    connect(activeTilePatternThread, &TilePatternThread::sendPatterns, this, &TilePatternCreator::updatePatterns);
    connect(activeTilePatternThread, &TilePatternThread::finishedSuccessfully, this, &TilePatternCreator::exportPatterns);
    connect(activeTilePatternThread, &TilePatternThread::finished, activeTilePatternThread, &TilePatternThread::deleteLater);
    activeTilePatternThread->start();
}

void TilePatternCreator::exportPatterns(){
    emit patternsSignal(this->tiles, this->patterns, this->wallPos); //this wallPos could get unsynced, but unlikely
    //unlock ui. will also need to do if interrupted
}

void TilePatternCreator::setPatternSize(int size){
    this->patternSize = size;
    ((GraphicsView*)this->creatorView->view())->setPatternSize(size); //slighty cursed. Fun :)
}

void TilePatternCreator::setTileSize(int size){
    this->tileSize = size;
    ((GraphicsView*)this->creatorView->view())->setTileSize(size);
}

void TilePatternCreator::toggleXWall(){
    this->wallPos = wallPos ^ WallPos::BottomWall;
}
void TilePatternCreator::toggleYWall(){
    this->wallPos = wallPos ^ WallPos::RightWall;
}

//////////////////////////////////////////////////////////////////

TilePatternThread::TilePatternThread(TilePatternCreator *creator, QObject *parent)
    : QThread{parent}, creator(creator)
{
    this->baseImage = creator->baseImage;
    this->patternSize = creator->patternSize;
    this->tileSize = creator->tileSize;
    this->wallPos = creator->wallPos;
}

QList<Tile> TilePatternThread::generateTiles(QImage baseImage, int tileSize){ //sideEfect: fills the idMap
    QList<Tile> newTiles{};

    int mapHeight = baseImage.height() / tileSize;
    int mapWidth = baseImage.width() / tileSize;

    idMap.clear();

    if(wallPos > WallPos::None){
        newTiles.append(Tile::getWallTile(tileSize));
    }

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
            if(iX == (mapWidth-1) && (bool)(wallPos & WallPos::RightWall)){ //if YWall
                idMap.append(0); //id of WallTile
            }

            if(this->isInterruptionRequested())
                return newTiles;
        }
    }
    if((bool)(wallPos & WallPos::BottomWall)){
        for(int iX = 0; iX <= mapWidth; iX++){
            idMap.append(0);
        }
    }
    emit sendTiles(newTiles);
    qDebug()<<"IdMap: "<<idMap;
    return newTiles;
}

void debugCompabilityCheck(const QList<Pattern> &patterns){
    for(const auto &pat:patterns){
        for(const auto &yComp : pat.compatibilityList){
            for(const auto &xComp : yComp){
                if(xComp.count(true) == 0){
                    qDebug()<<"found a problem in the patterns";
                }
            }
        }
    }
}

void TilePatternThread::updatePatternCompability(QList<Pattern> &patterns, int patternSize){
    for(Pattern &pRef: patterns){
        for(int dy = -patternSize + 1; dy < patternSize; dy++){
            for(int dx = -patternSize + 1; dx < patternSize; dx++){
                auto &compListRef = pRef.getCompabilityListRefAt({dx, dy});
                compListRef = QBitArray(patterns.size());
                //if((dx==0) && (dy==0)) continue; //skip self, actually, maybe better not
                for(const Pattern &otherP: patterns){
                    if(pRef.isCompatibleAt(otherP, {dx, dy}))
                        compListRef.setBit(otherP.id); //bitset

                    if(this->isInterruptionRequested())
                        return;
                }
            }
        }
    }
    //debugCompabilityCheck(patterns);
}

QList<Pattern> TilePatternThread::generatePatterns(QList<short> IDmap, int patternSize){

    QList<Pattern> newPatterns{};
    int mapHeight = baseImage.height() / tileSize;
    int mapWidth = baseImage.width() / tileSize;

    if((bool)(wallPos & WallPos::BottomWall)) mapHeight++;
    if((bool)(wallPos & WallPos::RightWall)) mapWidth++;


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

    for(int iY = 0; iY < mapHeight; iY++){ //pattern y limit
        for(int iX = 0; iX < mapWidth; iX++){ //pattern x limit
            QList<short> potentialPattern = getAdjacent(iX, iY);
            int findIndex = newPatterns.indexOf(potentialPattern);
            if(findIndex == -1){
                Pattern newPattern = Pattern(newPatterns.size(), potentialPattern, patternSize);
                newPatterns.append(newPattern);
            }
            else{
                newPatterns[findIndex].incrementWeight();
            }

            if(this->isInterruptionRequested())
                return newPatterns;
            //this->msleep(500); //holy shit this works
        }
    }
    updatePatternCompability(newPatterns, patternSize);
    //maybe normalize the weight?
    emit sendPatterns(newPatterns);
    return newPatterns;
}

void TilePatternThread::run(){
    generateTiles(this->baseImage,this->tileSize);
    generatePatterns(this->idMap, this->patternSize);
    if(!this->isInterruptionRequested())
        emit finishedSuccessfully();
}
