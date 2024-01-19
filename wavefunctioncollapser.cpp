#include "wavefunctioncollapser.h"
#include <QRandomGenerator>

WaveFunctionCollapser::WaveFunctionCollapser(View *generatorView, QObject *parent)
    : QObject{parent}, generatorView(generatorView)
{
    gridHeight = 10;
    gridWidth = 10;
}

QList<TileSlot> WaveFunctionCollapser::createEmptyGrid(int width, int height){
    auto newGrid = QList<TileSlot>{};

    int patternCount = patterns.size(); //if any tiles are disabled, make sure to 0 out those ones
    for(int y=0; y < height; y++){
        for(int x=0; x < width; x++){
            newGrid.append(TileSlot{QBitArray(patternCount, true), QPoint{x,y}});
        }
    }
    return newGrid;
}

bool WaveFunctionCollapser::isInBounds(const QPoint &pos){ //is this being used?
    return (pos.x()>=0) && (pos.x()<gridWidth) && (pos.y()>=0) && (pos.y()<gridHeight);
}

void WaveFunctionCollapser::displayGrid(const QList<TileSlot> &grid, const QList<Tile> &tiles, int width, int height){
    this->generatorView->view()->scene()->clear(); //Needed?
    int tileSize = tiles.first().size;

    for (const auto &ts: grid) { //Display the tiles
        QGraphicsItem *item;
        if(ts.collapsedId >= 0){
            short tileId = patterns.at(ts.collapsedId).tileIDs.first();
            item = new TileGraphicsItem(tiles.at(tileId));
        }
        else if(ts.collapsedId == -1){ //Display uncollapsed tile
            QString text;
            if(ts.patternIdBitset.count(false) == 0){
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
            item = rectItem;
        }
        else if(ts.collapsedId == -2){ //Display uncollapsible tile
            auto rectItem = new QGraphicsRectItem(QRect(0,0,tileSize,tileSize));
            rectItem->setBrush(QBrush(QColor(255,0,200),Qt::BrushStyle::Dense4Pattern));
            rectItem->setPen(QPen(QBrush(Qt::BrushStyle::SolidPattern),0));
            item = rectItem;
        }
        item->setPos(ts.pos * tileSize);
        generatorView->view()->scene()->addItem(item);
        //qDebug()<<tiles[ts.collapsedId].id;
    }
}

void WaveFunctionCollapser::setPatterns(QList<Tile> newTiles, QList<Pattern> newPatterns){
    this->tiles = newTiles;
    this->patterns = newPatterns;
    this->grid = createEmptyGrid(gridWidth, gridHeight);
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
    this->grid = createEmptyGrid(gridWidth, gridHeight);
    this->generatorView->view()->scene()->clear();
}

void WaveFunctionCollapser::generate(int iters){
    if(iters == 0){ //for 0 generate full grid
        iters = gridHeight * gridWidth;
        clearGrid();
    }
    //lock ui except for cancel or smth

    auto activeCollapserThread = new WaveFunctionThread(grid, patterns, gridWidth, gridHeight, iters, this); //currently not deleted?

    connect(activeCollapserThread, &WaveFunctionThread::sendGrid, this, &WaveFunctionCollapser::updateGrid);
    //connect(activeCollapserThread, &WaveFunctionThread::sendPatterns, this, &TilePatternCreator::updatePatterns);
    //connect(activeCollapserThread, &WaveFunctionThread::finishedSuccessfully, this, &WaveFunctionCollapser::smth);
    connect(activeCollapserThread, &WaveFunctionThread::finished, activeCollapserThread, &WaveFunctionThread::deleteLater);
    activeCollapserThread->start();
}

void WaveFunctionCollapser::generateOneStep(){
    generate(1);
}

void WaveFunctionCollapser::updateGrid(QList<TileSlot> newGrid){
    this->grid = newGrid;
    displayGrid(grid,tiles,gridWidth, gridHeight);
}

///////////////////////////////////////////////////////

WaveFunctionThread::WaveFunctionThread(const QList<TileSlot> &starterGrid, const QList<Pattern> &patterns, int gridWidth, int gridHeight, int iters, QObject *parent)
    : QThread{parent}, starterGrid(starterGrid), patterns(patterns), gridHeight(gridHeight), gridWidth(gridWidth), iters(iters)
{
    this->grid = starterGrid;
    //propagate Permanent
}

short weightedRandom(const QList<double> &weightLimits) {
    static auto gen = QRandomGenerator().securelySeeded();
    //gen.seed(0); //debug, const seed
    double randomWeight = gen.bounded(weightLimits.last()); //TODO: add random seed
    auto it = std::upper_bound(weightLimits.begin(), weightLimits.end(), randomWeight);
    return std::distance(weightLimits.begin(), it) - 1;
}

void WaveFunctionThread::collapseSlot(QPoint &toCollapse, const QList<Pattern> &patterns){ //Weighted random TODO: will need to deal with empty slots)
    TileSlot &slot = getSlotRefAt(toCollapse);
    if(slot.patternIdBitset.count(true) == 0){
        slot.collapsedId = -2; //I guess -2 means unsolvable?
        return; //return false?
    }
    QList<short> possibleIds;
    QList<double> weightLimits = {0.0};
    for(int id = 0; id < patterns.size(); id++){
        if(slot.patternIdBitset.testBit(id)){
            possibleIds.append(id);
            weightLimits.append(patterns.at(id).weight + weightLimits.back());
        }
    }
    short patternId = possibleIds[weightedRandom(weightLimits)]; //TODO: will need to update, maybe
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

        QList<Pattern> possiblePatterns = {}; //possible patterns for the tileslot

        for(int i=0; i<updatedSlot.patternIdBitset.size(); i++){
            if(updatedSlot.patternIdBitset.testBit(i))
                possiblePatterns.append(patterns.at(i));
        }

        for(int dy = -patternSize + 1; dy < patternSize; dy++){
            for(int dx = -patternSize + 1; dx < patternSize; dx++){

                TileSlot &currentSlot = getSlotRefAt(updatedSlot.pos + QPoint{dx, dy});
                if(currentSlot.collapsedId != -1) continue; //skip collapsed and uncollapsable

                QBitArray possiblePatternsSet = QBitArray();//collapsedPattern.getCompabilityListRefAt({dx, dy}); //slightly spaghetti, might need to go over multiple patterns (for(auto p:compListRef))
                for(auto pat: possiblePatterns){
                    possiblePatternsSet |= pat.getCompabilityListRefAt({dx,dy});
                }

                QBitArray newBitset = currentSlot.patternIdBitset & possiblePatternsSet;

                if(newBitset.count(true) == 0){
                    qDebug()<<"unsolvable, again";
                    currentSlot.collapsedId = -2;
                    return {};
                    //do something, return false, i dunno, handle uncollapsable
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

    QPoint toCollapse = getSlotToCollapse();

    if(toCollapse == QPoint{-1,-1}) //all are collapsed
        return true;

    collapseSlot(toCollapse, patterns);

    const auto &updatedSlots = propagateUpdate(toCollapse, patterns); //put in candidate updating

    for(const auto &pos:updatedSlots){
        if(collapseCandidatePos.contains(pos)) continue;
        collapseCandidatePos.append(pos);
    }

    /*
        while there are uncollapsed (or empty?) slots (in the updated list), do: while(!collapseCandidatePos.empty())
            get the slot with the lowest entropy (TileSlot &currentSlot = collapseCandidatePos.first()) <- put in references!
            collapse it by tile weight (TileSlot.collapse(const QList<Tile> &tiles)), maybe by pattern weight too? how?
            propagate update: (propagateUpdate(TileSlot t)), needs x and y in tileslot
                get a list of coordinates to check patterns at (NxN, out of bounds allowed) (return range x-n+1 - x, y-n+1 - y of coords)
                for each coordinate, generate based on the patterns a list of possible tiles to place in the slot (maybe should be boolean/bitset?) (uhh)
                    out of bounds count as uncollapsed (if( isOutOfBounds(coords)) behave as if its TileId list is full
                    compare the generated list with the actual list in the tile (if TileSlot.idList ^ newLists.at(innerCords/index?) == 0)
                    if the lists are different:
                        update the tile and insert into the list of coordinates affected patterns (TileSlot.idList = newLists.at(innerCords/index), then same as step 2a
                repeat, until the coordinate list is empty (done by the while loop)
            sort collapseCandidatePos by entropy (lenght) (ez)

            TODO: dummy slots for out of bounds(v), updating the view, does this shit even work?
     */
    return true;
}

void WaveFunctionThread::run(){
    //this->grid = this->starterGrid;
    try{
    for(int i = 0; i < this->iters; i++){
        generateGridStep();
        emit sendGrid(grid);
        if(this->isInterruptionRequested()) break;
    }
    if(!this->isInterruptionRequested()) //or if generation failed/gave unsolvable
        emit finishedSuccessfully();
    }
    catch(std::exception e){
        qDebug()<<&e;
    }
}
