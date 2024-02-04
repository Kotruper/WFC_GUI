#include "wavefunctioncollapser.h"
#include "qdatetime.h"
#include "qgraphicssceneevent.h"
#include "qmimedata.h"
#include "qtimer.h"
#include <QRandomGenerator>

void TileSlotDisplayItem::dropEvent(QGraphicsSceneDragDropEvent* e){
    qDebug()<<"Drop detected! by slot at"<<this->pos();
    if (e->mimeData()->hasImage()) {
        //auto wfc = (WaveFunctionCollapser*)this->parentWidget();
        e->accept();
    } else {
        QGraphicsItem::dropEvent(e);
    }
}
void TileSlotDisplayItem::dragEnterEvent(QGraphicsSceneDragDropEvent* e){
    e->accept();
    qDebug()<<"DragEnter detected!";
    QGraphicsItem::dragEnterEvent(e);
}

void TileSlotDisplayItem::dragMoveEvent(QGraphicsSceneDragDropEvent* e){
    e->accept();
    qDebug()<<"DragMove detected!";
    QGraphicsItem::dragMoveEvent(e);
}
/////////////////////////////////////////////

WaveFunctionCollapser::WaveFunctionCollapser(View *generatorView, QObject *parent)
    : QObject{parent}, generatorView(generatorView)
{
    gridHeight = 30;
    gridWidth = 30;
    collapseCandidatePos = {};
    this->wfc_thread = nullptr;
    this->updateGridTimer = new QTimer(this);
    updateGridTimer->setInterval(500); //10/sec at first
    connect(updateGridTimer, &QTimer::timeout, this, &WaveFunctionCollapser::updateGrid);
    connect((GraphicsView*)generatorView->view(), &GraphicsView::sendTileId, this, &WaveFunctionCollapser::receiveDroppedTile);
    QTimer::singleShot(100, [&](){emit setWFCEnabled(false);});
}

QBitArray WaveFunctionCollapser::getWallPatterns(const QList<Pattern> &patterns, const QList<Tile> &tiles, const WallPos &wallPos){
    QBitArray result{patterns.size()};
    if(wallPos > WallPos::None){
        for(const auto &p:patterns) if(p.isWallPattern()) result.setBit(p.id);
    }
    return result;
}

QList<TileSlot> WaveFunctionCollapser::createEmptyGrid(int width, int height){ //probably works, in observation though
    auto newGrid = QList<TileSlot>{};
    QBitArray allowedWallPatterns = getWallPatterns(patterns, tiles, wallPos) & allowedPatterns;
    int wallWidth = 0;

    if(wallPos > WallPos::None) wallWidth = patterns.first().size - 1;
    if((bool)(wallPos & WallPos::RightWall)) width += wallWidth;
    if((bool)(wallPos & WallPos::BottomWall)) height += wallWidth;

    for(int y=0; y < height; y++){
        for(int x=0; x < width; x++){
            newGrid.append(TileSlot{allowedPatterns & ~allowedWallPatterns, QPoint{x,y}}); //normal tile, bitset has allowed patterns that arent wall patterns
            if(((x >= width  - wallWidth) && (bool)(wallPos & WallPos::RightWall))//if right tile allow only walls
                || ((y >= height - wallWidth) && (bool)(wallPos & WallPos::BottomWall))){//or if bottom tile allow only walls
                newGrid.back().patternIdBitset = allowedWallPatterns;
                newGrid.back().permamentTileId = 0;
                //collapseCandidatePos.append(newGrid.back().pos);
            }
        }
    }
    if(wallPos == WallPos::BothWall){
        QBitArray cornerPattern = QBitArray(allowedWallPatterns.size());
        for(const auto &p:patterns) if(p.isCornerWallPattern()) cornerPattern.setBit(p.id);
        newGrid.back().patternIdBitset = cornerPattern; //if corner, only allow corner wall tiles
        newGrid.back().permamentTileId = 0;
    }
    return newGrid;
}

void WaveFunctionCollapser::displayGrid(const QList<TileSlot> &grid, const QList<Tile> &tiles, int width, int height){
    auto scene = this->generatorView->view()->scene();
    scene->clear(); //Needed?
    if(tiles.empty()) return;
    int tileSize = tiles.first().size;
    bool skipWall = tiles.first().isWall;
    bool includeText = false;

    QGraphicsItemGroup* gridDisplay = new QGraphicsItemGroup(); //not much effect, maybe faster

    auto getCollapsedItem = [&](const TileSlot &ts){
        const short &tileId = patterns.at(ts.collapsedId).tileIDs.first();

        if((tileId == 0) && skipWall) return (TileSlotDisplayItem*) nullptr; //skip rendering wall tiles
        TileSlotDisplayItem* item = (TileSlotDisplayItem*) new TileGraphicsItem(tiles.at(tileId));
        if(includeText){
            auto innerText = new QGraphicsSimpleTextItem(QString("%1").arg(ts.collapsedId), item);
            innerText->setBrush(QBrush(Qt::BrushStyle::SolidPattern));
            innerText->setScale(0.01);
        }
        return item;
    };

    auto getUncollapsedTextItem = [&](const TileSlot &ts){
        QString text;
        if(ts.patternIdBitset.count(true) >= 36){ //PERF test, changed from ==
            text = "Uncollapsed";
        }
        else{
            int amnt = 0;
            for(int i = 0; i < ts.patternIdBitset.size(); i++){
                if(ts.patternIdBitset.testBit(i)){
                    text.append(std::to_string(i).append(","));
                    amnt++;
                    if((amnt%6) == 0) text.append('\n');
                }
                //text.append((ts.patternIdBitset.testBit(i)) ? '1' : '0');
            }
        }
        auto rectItem = new QGraphicsRectItem(QRect(0,0,tileSize,tileSize));
        rectItem->setPen(QPen(QBrush(Qt::BrushStyle::SolidPattern),0));
        auto innerText = new QGraphicsSimpleTextItem(text, rectItem);
        innerText->setBrush(QBrush(Qt::BrushStyle::SolidPattern));
        innerText->setScale(0.01);
        return (TileSlotDisplayItem*) rectItem;
    };

    auto getUncollapsedBlendedItem = [&](const TileSlot &ts){
        QVarLengthArray<int> tileAmounts(static_cast<int>(tiles.size()),0);
        if(ts.patternIdBitset.count(true) <= -1){ //change to return blended tile
            //text = "Uncollapsed";
        }
        const auto item = new QGraphicsItemGroup();
        for(int i = 0; i<ts.patternIdBitset.size(); i++){
            if(ts.patternIdBitset.testBit(i)) tileAmounts[patterns.at(i).tileIDs.first()]++;
        }
        const int totalItems = ts.patternIdBitset.count(true);
        for(int tileID = (tiles.first().isWall) ? (1) : (0); tileID < tileAmounts.size(); tileID++){
            if(tileAmounts.at(tileID) > 0)
                item->addToGroup(new TileGraphicsItem(tiles.at(tileID), tileAmounts.at(tileID) / (totalItems * 1.0)));
        }
        return (TileSlotDisplayItem*) item;
    };

    for (const auto &ts: grid) { //Display the tiles
        TileSlotDisplayItem *item;
        if(ts.collapsedId >= 0){
            item = getCollapsedItem(ts);
        }
        else if(ts.collapsedId == -1){ //Display uncollapsed tile, change to blended tiles
            //item = getUncollapsedTextItem(ts);
            item = getUncollapsedBlendedItem(ts);
        }
        else if(ts.collapsedId == -2){ //Display uncollapsible tile
            auto rectItem = new QGraphicsRectItem(QRect(0,0,tileSize,tileSize));
            rectItem->setBrush(QBrush(QColor(255,0,200),Qt::BrushStyle::SolidPattern));
            rectItem->setPen(QPen(QBrush(Qt::BrushStyle::SolidPattern),0));
            item =(TileSlotDisplayItem*) rectItem;
        }
        if(item == nullptr) continue;
        item->setPos(ts.pos * tileSize);
        item->setAcceptDrops(true); //EDIT, maybe?
        if(ts.permamentTileId > -1) item->setCursor(QCursor(Qt::CursorShape::PointingHandCursor)); //hoped it'd work
        gridDisplay->addToGroup(item);
        //qDebug()<<tiles[ts.collapsedId].id;
    }
    gridDisplay->setPos(0,0);
    scene->addItem(gridDisplay);
    scene->setSceneRect(scene->itemsBoundingRect());
    //generatorView->view()->scene()->setSceneRect(0,0,gridWidth * tiles.first().size, gridHeight * tiles.first().size); //exporting?
}

QBitArray WaveFunctionCollapser::getAllowedPatterns(const QList<Pattern> &patterns, const QList<Tile> &tiles){
    auto enabledPatterns = QBitArray{patterns.size(), true};

    for(const auto &p:patterns)//if any patterns are disabled, make sure to 0 out those ones
        if(!p.enabled) enabledPatterns.setBit(p.id, false);

    auto disabledTilesIds = QList<int>{}; //if any tiles are disabled, disable the patterns that contain them. Is this slow?

    for(const auto &t:tiles)
        if(!t.enabled) disabledTilesIds.append(t.id);

    for(const auto &id:disabledTilesIds){
        for(const auto &p:patterns){
            if(p.tileIDs.contains(id)) enabledPatterns.setBit(p.id, false);
        }
    }

    //if(wallPos > WallPos::None) enabledPatterns &= ~getWallPatterns(patterns,tiles,wallPos);//if walls are enabled, remove wall patterns from regular tiles
    //problem: loses dissallowed wall pattern info
    return enabledPatterns;
}

void WaveFunctionCollapser::setPatterns(QList<Tile> newTiles, QList<Pattern> newPatterns, WallPos wallPos){
    this->tiles = newTiles;
    this->patterns = newPatterns;
    this->wallPos = wallPos;
    this->allowedPatterns = getAllowedPatterns(patterns, tiles); //calculates the initial pattern state

    emit setWFCEnabled(patterns.size() > 0);

    clearGrid();
    qDebug()<<"Received tiles ("<<newTiles.size()<<") and patterns("<<newPatterns.size()<<")";
}

void WaveFunctionCollapser::changeGridHeight(int val){ //should take into consideration changes during generation, such as height, width and different tiles and patterns
    if(val > 0)
        this->gridHeight = val;
    clearGrid();
}

void WaveFunctionCollapser::changeGridWidth(int val){
    if(val > 0)
        this->gridWidth = val;
    clearGrid();
}

void WaveFunctionCollapser::clearGrid(){
    //if(patterns.empty() || tiles.empty()) //maybe put out an exception? send an error signal?
        //return;

    this->grid = createEmptyGrid(gridWidth, gridHeight);
    this->generatorView->view()->scene()->clear();
    displayGrid(grid,tiles,gridWidth,gridHeight);
}

void WaveFunctionCollapser::generate(int iters){
    if(patterns.empty() || tiles.empty()) //maybe put out an exception? send an error signal?
        return;
    if(wfc_thread != nullptr){
        wfc_thread->requestInterruption();//received cancel command, somehow
        return;
    }

    if(iters == 0){ //for 0 generate remaining grid
        for(const auto &ts:grid) if(ts.collapsedId == -1) iters++;
        if(iters == 0){ //if none remain, generate new grid
            clearGrid();
            for(const auto &ts:grid) if(ts.collapsedId == -1) iters++;
        }
    }
    qDebug()<<"Generation started. Iters: "<<iters;
    const int wallWidth = (wallPos == WallPos::None) ? (0) : (patterns.first().size - 1);
    const int collapserGridHeight = ((bool)(wallPos & WallPos::BottomWall)) ? (gridHeight + wallWidth) : (gridHeight);
    const int collapserGridWidth = ((bool)(wallPos & WallPos::RightWall)) ? (gridWidth + wallWidth) : (gridWidth);

    auto activeCollapserThread = new WaveFunctionThread(grid, patterns, tiles, collapserGridWidth, collapserGridHeight, iters, collapseCandidatePos, seed, wallPos, this); //currently not deleted?
    this->wfc_thread = activeCollapserThread;

    connect(activeCollapserThread, &WaveFunctionThread::sendGrid, this, &WaveFunctionCollapser::updateGrid);
    connect(activeCollapserThread, &WaveFunctionThread::displayError, this, &WaveFunctionCollapser::displayError);
    connect(activeCollapserThread, &WaveFunctionThread::finishedSuccessfully, this, &WaveFunctionCollapser::saveCandidates);
    connect(activeCollapserThread, &WaveFunctionThread::finished, this, &WaveFunctionCollapser::handleThreadEnd);
    connect(activeCollapserThread, &WaveFunctionThread::finished, activeCollapserThread, &WaveFunctionThread::deleteLater);

    activeCollapserThread->start();
    updateGridTimer->start();
    this->setEnabledButtons(false);//lock ui except for cancel or smth
}

void WaveFunctionCollapser::generateOneStep(){
    generate(1);
}
void WaveFunctionCollapser::cancelGenerate(){
    if(wfc_thread != nullptr){//received cancel command
        wfc_thread->requestInterruption();
    }
}
void WaveFunctionCollapser::handleThreadEnd(){
    this->wfc_thread = nullptr;
    this->setEnabledButtons(true);
    updateGridTimer->stop();
}
void WaveFunctionCollapser::receiveDroppedTile(int id, const QPointF pos){//abomination
    QPointF scenePos = generatorView->view()->mapToScene(pos.toPoint());
    qDebug()<<"Received tile "<<id<<" at scene pos "<<scenePos;
    if(generatorView->view()->sceneRect().contains(scenePos)){ //maybe also check if pos already has an id?
        QPoint tilePos = ((scenePos / tiles.first().size) - QPointF(0.5,0.5)).toPoint(); //abomination 2
        TileSlot &tileRef = grid[tilePos.x() + tilePos.y() * (gridWidth + ((bool)(wallPos & WallPos::RightWall) ? (patterns.first().size-1):(0)))];//abomination 3
        QBitArray tilePatterns(patterns.size(),true);
        if(id > -1) //if not resetting
            for(int i = 0; i<tilePatterns.size(); i++)
                if(patterns[i].tileIDs.first() != id) tilePatterns.clearBit(i);

        tileRef.permamentTileId = id;
        tileRef.patternIdBitset = tilePatterns & allowedPatterns & ~getWallPatterns(patterns,tiles,wallPos);
        displayGrid(grid,tiles,gridWidth,gridHeight);
    }
}
void WaveFunctionCollapser::updateGrid(){ //probably will need major optimisations
    this->grid = wfc_thread->requestGrid();
    //auto& borrowedGrid = wfc_thread->requestGrid();
    displayGrid(grid, tiles, gridWidth, gridHeight);
}

void WaveFunctionCollapser::saveCandidates(QList<QPoint> candidates){
    updateGrid();
    this->collapseCandidatePos = candidates;
}

void WaveFunctionCollapser::setSeed(int newSeed){
    this->seed = newSeed;
}

void WaveFunctionCollapser::exportImage(QString filename){ //help

    QGraphicsScene* scene = this->generatorView->view()->scene();
    //QRectF sceneRect2 = {0,0, gridWidth * tiles.first().size * 100.0, gridHeight * tiles.first().size * 100.0}; //getting desperate

    scene->clearSelection();                                                  // Selections would also render to the file
    scene->setSceneRect(scene->itemsBoundingRect());                            // Re-shrink the scene to it's bounding contents
    QImage image(scene->sceneRect().width(), scene->sceneRect().height(), QImage::Format_ARGB32);  // Create the image with the exact size of the shrunk scene
    image.fill(Qt::transparent);                                              // Start all pixels transparent

    QPainter painter(&image);
    scene->render(&painter); //nope
    image.save(filename);
}

void WaveFunctionCollapser::setUIEnabled(bool enabled){
    emit setEnabledButtons(enabled);
    QString text = (enabled) ? "Generate" : "Cancel";
    emit toggleGenerateCancelButton(text);
}

///////////////////////////////////////////////////////

WaveFunctionThread::WaveFunctionThread(const QList<TileSlot> &starterGrid, const QList<Pattern> &patterns, const QList<Tile> &tiles, int gridWidth, int gridHeight, int iters, QList<QPoint> candidates, int seed, WallPos wallPos, QObject *parent)
    : QThread{parent}, starterGrid(starterGrid), patterns(patterns), tiles(tiles), gridHeight(gridHeight), gridWidth(gridWidth), iters(iters), collapseCandidatePos(candidates), seed(seed), wallPos(wallPos)
{
    this->grid = starterGrid;
    this->randomGenerator = QRandomGenerator().securelySeeded();
    //propagate Permanent
}

short weightedRandom(const QVarLengthArray<double> &weightLimits, QRandomGenerator &gen) {
    double randomWeight = gen.bounded(weightLimits.last()); //TODO: add random seed
    auto it = std::upper_bound(weightLimits.begin(), weightLimits.end(), randomWeight);
    return std::distance(weightLimits.begin(), it) - 1;
}

void WaveFunctionThread::collapseSlot(const QPoint &toCollapse, const QList<Pattern> &patterns, const QList<Tile> &tiles){ //Weighted random TODO: will need to deal with empty slots)
    TileSlot &slot = getSlotRefAt(toCollapse);
    if(slot.patternIdBitset.count(true) == 0){
        slot.collapsedId = -2; //I guess -2 means unsolvable?
        return; //return false?
    }
    QVarLengthArray<short> possibleIds;
    QVarLengthArray<double> weightLimits = {0.0};
    for(int id = 0; id < patterns.size(); id++){
        if(slot.patternIdBitset.testBit(id)){
            possibleIds.append(id);
            double weight = (patterns.at(id).weight * tiles.at(patterns.at(id).tileIDs.first()).weight);
            weightLimits.append(weight + weightLimits.back());
        }
    }
    short patternId = possibleIds[weightedRandom(weightLimits, this->randomGenerator)]; //TODO: will need to update, maybe
    slot.collapsedId = patternId;
    slot.patternIdBitset.fill(false);
    slot.patternIdBitset.setBit(patternId);
}

TileSlot& WaveFunctionThread::getSlotRefAt(const QPoint &pos){ //modulo, accepts negative.
    int wrappedX = (pos.x() + gridWidth) % gridWidth; //adding width/height for neg modulo shenanigans
    int wrappedY = (pos.y() + gridHeight) % gridHeight;
    return grid[( wrappedX + wrappedY*gridWidth)];
}

QList<QPoint> WaveFunctionThread::propagateUpdate(const QPoint &collapsed, const QList<Pattern> &patterns){ //should probably disentagle this mess. unless it works, then fuck it
    int patternSize = patterns.first().size;
    QList<QPoint> updatedSlots = {collapsed}; //maybe set?

    auto ifNotContainedAppend = [&updatedSlots](const QPoint &updatedPos){
        if(updatedSlots.contains(updatedPos))
            return;
        updatedSlots.append(updatedPos);
    };

    int currSlotIndex = 0;
    while(currSlotIndex < updatedSlots.size()){
        //qDebug()<<"propagation: updatedSlotAmount:"<<updatedSlots.size();
        const TileSlot &updatedSlot = getSlotRefAt(updatedSlots.at(currSlotIndex));
        currSlotIndex++;
        if(updatedSlot.collapsedId == -2) continue; //dont propagate uncollapsables, TEST. maybe works? strange

        QVarLengthArray<Pattern> possiblePatterns = {}; //possible patterns for the tileslot
        for(int i=0; i<updatedSlot.patternIdBitset.size(); i++){
            if(updatedSlot.patternIdBitset.testBit(i))
                possiblePatterns.append(patterns.at(i));
        }

        for(int dy = -patternSize + 1; dy < patternSize; dy++){
            for(int dx = -patternSize + 1; dx < patternSize; dx++){

                TileSlot &currentSlot = getSlotRefAt(updatedSlot.pos + QPoint{dx, dy});
                if(currentSlot.collapsedId != -1) continue; //skip collapsed and uncollapsable

                QBitArray possiblePatternsSet = QBitArray();//collapsedPattern.getCompabilityListRefAt({dx, dy}); //slightly spaghetti, might need to go over multiple patterns (for(auto p:compListRef))
                for(auto &pat: possiblePatterns){
                    possiblePatternsSet |= pat.getCompabilityListRefAt({dx,dy});
                }

                QBitArray newBitset = currentSlot.patternIdBitset & possiblePatternsSet;

                if(this->isInterruptionRequested()) return {}; //TODO

                if(newBitset.count(true) == 0){// if uncollapsable found
                    qDebug()<<"unsolvable at pos: "<<currentSlot.pos;
                    currentSlot.collapsedId = -2;
                    currentSlot.patternIdBitset = newBitset;
                    throw unsolvableException(); //throw error, try again
                }

                if(currentSlot.patternIdBitset != newBitset){
                    currentSlot.patternIdBitset = newBitset;
                    ifNotContainedAppend(currentSlot.pos);
                }
            }
        }
    }
    //updatedSlots.pop_front(); //remove the collapsed TileSlot. Necessary. May need to keep track of updated slots somehow

    return updatedSlots; //test this whole thing
}

QPoint WaveFunctionThread::getSlotToCollapse(){ //TEST
    if(collapseCandidatePos.size() > 10000){
        std::sort(collapseCandidatePos.begin(), collapseCandidatePos.end(),
                  [&](const QPoint &a, const QPoint &b){
            return getSlotRefAt(a) > getSlotRefAt(b);
        });
        return collapseCandidatePos.takeLast(); //assumes sorted and pops it out
    }
    else{
        TileSlot minEntropySlot = {QBitArray{grid.first().patternIdBitset.size(), true},{-1,-1}}; //-1,-1 means none were found
        for(const auto &ts:grid){ //get the first uncollapsed slot with the lowest entropy
            if(ts.collapsedId == -1)
                if(ts < minEntropySlot)
                    minEntropySlot = ts;
        }
        return minEntropySlot.pos;
    }
}

bool WaveFunctionThread::generateGridStep(){
    //qDebug()<<"Began generating grid step";

    const QPoint &toCollapse = getSlotToCollapse();

    if(toCollapse == QPoint{-1,-1}) //all are collapsed
        return true;

    collapseSlot(toCollapse, patterns, tiles);
    propagateUpdate(toCollapse, patterns);

/*
 * const auto updatedSlots =
    for(const auto &pos:updatedSlots){
        if(collapseCandidatePos.contains(pos)) continue;
        collapseCandidatePos.append(pos);
    }
*/
    return true;
}

const QList<TileSlot>& WaveFunctionThread::requestGrid(){
    QMutexLocker mLocker(&mutex);
    return this->grid;
}

void WaveFunctionThread::run(){
    this->starterGrid = this->grid;
    this->startingCandidatePos = this->collapseCandidatePos;
    int remainingAttempts = this->attemptLimit;
    auto startTime = QTime::currentTime();
    if(this->seed > -1) randomGenerator.seed(this->seed);
    while(remainingAttempts > 0){
        try{
            for(int i = 0; i < this->iters; i++){
                generateGridStep();
                if(this->isInterruptionRequested()) return;
            }
            if(!this->isInterruptionRequested()){//or if generation failed/gave unsolvable
                emit finishedSuccessfully(this->collapseCandidatePos);
                auto endTime = QTime::currentTime();
                qDebug()<<"Time needed to collapse "<<iters<<"tiles: "<<(startTime.msecsTo(endTime))<<"ms";
                return;
            }
        }
        catch(const unsolvableException &e){ //a mess
            qDebug()<<"Unsolvable pos found, attempting again. Remaining attempts: "<<remainingAttempts;
            remainingAttempts--;
            this->grid = starterGrid;
            //this->collapseCandidatePos = startingCandidatePos;
            continue;
        }
        return;
    }
    emit displayError("Ran out of generation attempts! Change tile/pattern size or settings for better results or try again.");
}
