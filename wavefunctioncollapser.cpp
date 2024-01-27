#include "wavefunctioncollapser.h"
#include "qgraphicssceneevent.h"
#include "qmimedata.h"
#include "qtimer.h"
#include <QRandomGenerator>

void TileSlotDisplayItem::dropEvent(QGraphicsSceneDragDropEvent* e){
    qDebug()<<"Drop detected!";
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
    gridHeight = 10;
    gridWidth = 10;
    collapseCandidatePos = {};
    this->wfc_thread = nullptr;
    this->updateGridTimer = new QTimer(this);
    updateGridTimer->setInterval(1000); //10/sec at first
    connect(updateGridTimer, &QTimer::timeout, this, &WaveFunctionCollapser::updateGrid);
}

QList<TileSlot> WaveFunctionCollapser::createEmptyGrid(int width, int height){
    auto newGrid = QList<TileSlot>{};

    for(int y=0; y < height; y++){
        for(int x=0; x < width; x++){
            newGrid.append(TileSlot{allowedPatterns, QPoint{x,y}});
        }
    }
    return newGrid;
}

bool WaveFunctionCollapser::isInBounds(const QPoint &pos){ //is this being used?
    return (pos.x()>=0) && (pos.x()<gridWidth) && (pos.y()>=0) && (pos.y()<gridHeight);
}

void WaveFunctionCollapser::displayGrid(const QList<TileSlot> &grid, const QList<Tile> &tiles, int width, int height){
    auto scene = this->generatorView->view()->scene();
    scene->clear(); //Needed?
    int tileSize = tiles.first().size;

    QGraphicsItemGroup* gridDisplay = new QGraphicsItemGroup(); //not much effect, maybe faster

    for (const auto &ts: grid) { //Display the tiles
        TileSlotDisplayItem *item;
        if(ts.collapsedId >= 0){
            short tileId = patterns.at(ts.collapsedId).tileIDs.first();
            item = (TileSlotDisplayItem*) new TileGraphicsItem(tiles.at(tileId));
        }
        else if(ts.collapsedId == -1){ //Display uncollapsed tile, change to blended tiles
            /*
            QList<Tile> slotTiles = {};
            for(int i = 0; i<ts.patternIdBitset.size(); i++)
                if(ts.patternIdBitset.testBit(i)) slotTiles.append(tiles.at(patterns.at(i).tileIDs.first())); //NOPE! extremely slow. but fun

            item = new TileGraphicsItem(slotTiles);
            */

            QString text;
            if(ts.patternIdBitset.count(false) >= 0){ //PERF test, changed from ==
                text = "Uncollapsed";
            }
            else{
                for(int i = 0; i < ts.patternIdBitset.size(); i++){
                    if(ts.patternIdBitset.testBit(i))
                        text.append(std::to_string(i).append(","));
                    //text.append((ts.patternIdBitset.testBit(i)) ? '1' : '0');
                    //if((i%16) == 15) text.append('\n');
                }
            }
            auto rectItem = new QGraphicsRectItem(QRect(0,0,tileSize,tileSize));
            rectItem->setPen(QPen(QBrush(Qt::BrushStyle::SolidPattern),0));
            auto innerText = new QGraphicsSimpleTextItem(text, rectItem);
            innerText->setBrush(QBrush(Qt::BrushStyle::SolidPattern));
            innerText->setScale(0.01);
            item =(TileSlotDisplayItem*) rectItem;
        }
        else if(ts.collapsedId == -2){ //Display uncollapsible tile
            auto rectItem = new QGraphicsRectItem(QRect(0,0,tileSize,tileSize));
            rectItem->setBrush(QBrush(QColor(255,0,200),Qt::BrushStyle::SolidPattern));
            rectItem->setPen(QPen(QBrush(Qt::BrushStyle::SolidPattern),0));
            item =(TileSlotDisplayItem*) rectItem;
        }
        item->setPos(ts.pos * tileSize);
        //item->setAcceptDrops(true); //EDIT, doesn't seem to work
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
    return enabledPatterns;
}

void WaveFunctionCollapser::setPatterns(QList<Tile> newTiles, QList<Pattern> newPatterns){
    this->tiles = newTiles;
    this->patterns = newPatterns;
    this->allowedPatterns = getAllowedPatterns(patterns, tiles); //calculates the initial pattern state

    clearGrid();
    qDebug()<<"Received tiles ("<<newTiles.size()<<") and patterns("<<newPatterns.size()<<")";
}

void WaveFunctionCollapser::changeGridHeight(int val){
    if(val > 0)
        this->gridHeight = val;
}

void WaveFunctionCollapser::changeGridWidth(int val){
    if(val > 0)
        this->gridWidth = val;
}

void WaveFunctionCollapser::clearGrid(){
    if(patterns.empty() || tiles.empty()) //maybe put out an exception? send an error signal?
        return;

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
        for(const auto &ts:grid)
            if(ts.collapsedId == -1) iters++;
        //clearGrid();
    }


    auto activeCollapserThread = new WaveFunctionThread(grid, patterns, gridWidth, gridHeight, iters, collapseCandidatePos, seed, this); //currently not deleted?
    this->wfc_thread = activeCollapserThread;

    connect(activeCollapserThread, &WaveFunctionThread::sendGrid, this, &WaveFunctionCollapser::updateGrid);
    //connect(activeCollapserThread, &WaveFunctionThread::sendPatterns, this, &TilePatternCreator::updatePatterns);
    connect(activeCollapserThread, &WaveFunctionThread::finishedSuccessfully, this, &WaveFunctionCollapser::saveCandidates);
    connect(activeCollapserThread, &WaveFunctionThread::finished, activeCollapserThread, &WaveFunctionThread::deleteLater);
    connect(activeCollapserThread, &WaveFunctionThread::finished, this, [&](){this->wfc_thread = nullptr;});
    connect(activeCollapserThread, &WaveFunctionThread::finished, this, [&](){this->setEnabledButtons(true);});
    connect(activeCollapserThread, &WaveFunctionThread::finished, updateGridTimer, &QTimer::stop);

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

WaveFunctionThread::WaveFunctionThread(const QList<TileSlot> &starterGrid, const QList<Pattern> &patterns, int gridWidth, int gridHeight, int iters, QList<QPoint> candidates, int seed, QObject *parent)
    : QThread{parent}, starterGrid(starterGrid), patterns(patterns), gridHeight(gridHeight), gridWidth(gridWidth), iters(iters), collapseCandidatePos(candidates), seed(seed)
{
    this->grid = starterGrid;
    //propagate Permanent
}

short weightedRandom(const QVarLengthArray<double> &weightLimits, int seed) {
    static auto gen = QRandomGenerator().securelySeeded();
    if(seed > -1)
        gen.seed(seed);

    double randomWeight = gen.bounded(weightLimits.last()); //TODO: add random seed
    auto it = std::upper_bound(weightLimits.begin(), weightLimits.end(), randomWeight);
    return std::distance(weightLimits.begin(), it) - 1;
}

void WaveFunctionThread::collapseSlot(const QPoint &toCollapse, const QList<Pattern> &patterns){ //Weighted random TODO: will need to deal with empty slots)
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
            weightLimits.append(patterns.at(id).weight + weightLimits.back());
        }
    }
    short patternId = possibleIds[weightedRandom(weightLimits, this->seed)]; //TODO: will need to update, maybe
    slot.collapsedId = patternId;
    slot.patternIdBitset = QBitArray(slot.patternIdBitset.size());
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

    auto isUnsolvable = [](const TileSlot &t){return *t.patternIdBitset.bits() == 0;};

    auto ifNotContainedAppend = [&updatedSlots](const QPoint &updatedPos){
        if(updatedSlots.contains(updatedPos))
            return;
        updatedSlots.append(updatedPos);
    };

    int currSlotIndex = 0;
    while(currSlotIndex < updatedSlots.size()){
        //qDebug()<<"propagation: updatedSlotAmount:"<<updatedSlots.size();
        const TileSlot &updatedSlot = getSlotRefAt(updatedSlots.at(currSlotIndex));

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

                if(newBitset.count(true) == 0){ //ok, this is kinda weird. when it finds a possible uncollapsable slot, it just skips the whole propagation phase
                    //this can result in it finding a better collapse that doesnt give uncollapsable results, maybe
                    qDebug()<<"unsolvable at pos: "<<currentSlot.pos;
                    //currentSlot.collapsedId = -2;
                    //updatedSlots.pop_front();
                    //currentSlot.patternIdBitset = newBitset;
                    return {}; //could also return the current updated, food for thought, or empty list
                }

                if(currentSlot.patternIdBitset != newBitset){
                    currentSlot.patternIdBitset = newBitset;
                    ifNotContainedAppend(currentSlot.pos);
                }

                if(this->isInterruptionRequested())
                    return {}; //TODO
                }
            //emit sendGrid(grid);
            //this->msleep(100);
            }
        currSlotIndex++;
        }

    updatedSlots.pop_front(); //remove the collapsed TileSlot. Necessary. May need to keep track of updated slots somehow

    return updatedSlots; //test this whole thing
}

QPoint WaveFunctionThread::getSlotToCollapse(){
    if(collapseCandidatePos.size() > 0){
        std::sort(collapseCandidatePos.begin(), collapseCandidatePos.end(),
                  [&](const QPoint &a, const QPoint &b){
            return getSlotRefAt(a) > getSlotRefAt(b);
        });
        return collapseCandidatePos.takeLast(); //assumes sorted and pops it out
    }
    else{
        TileSlot minEntropySlot = {QBitArray{0},{-1,-1}}; //-1,-1 means none were found
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

    collapseSlot(toCollapse, patterns);

    const auto updatedSlots = propagateUpdate(toCollapse, patterns);

    for(const auto &pos:updatedSlots){
        if(collapseCandidatePos.contains(pos)) continue;
        collapseCandidatePos.append(pos);
    }
    return true;
}

const QList<TileSlot>& WaveFunctionThread::requestGrid(){
    QMutexLocker mLocker(&mutex);
    return this->grid;
}

void WaveFunctionThread::run(){
    //this->grid = this->starterGrid;
    try{
        for(int i = 0; i < this->iters; i++){
            generateGridStep();
            if(this->isInterruptionRequested()) break;
        }
        if(!this->isInterruptionRequested()) //or if generation failed/gave unsolvable
            emit finishedSuccessfully(this->collapseCandidatePos);
        }
    catch(std::exception e){
        qDebug()<<&e;
    }
}
