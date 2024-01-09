#include "wfc.h"
#include <QRandomGenerator>

wfc::wfc(View *generatorView, QObject *parent)
    : QObject{parent}, generatorView(generatorView)
{
    gridHeight = 5; //connect these later
    gridWidth = 5;
}

QList<TileSlot> wfc::createEmptyGrid(const QList<Tile> &tiles, int width, int height){
    auto newGrid = QList<TileSlot>{};

    int tileAmount = tiles.size(); //if any tiles are disabled, make sure to 0 out those ones
    for(int y=0; y < height; y++){
        for(int x=0; x < width; x++){
            newGrid.append(TileSlot{QBitArray(tileAmount, true), QPoint{x,y}});
        }
    }
    QPoint middleSlot = QPoint{width/2, height/2};
    this->collapseCandidatePos.clear();
    this->collapseCandidatePos.append(middleSlot);
    return newGrid;
}

short weightedRandom(const QList<double> &weightLimits) {
    double randomWeight = QRandomGenerator::global()->bounded(weightLimits.last()); //TODO: add random seed
    auto it = std::upper_bound(weightLimits.begin(), weightLimits.end(), randomWeight);
    return std::distance(weightLimits.begin(), it) - 1;
}

void wfc::collapseSlot(QPoint toCollapse, const QList<Tile> &tiles){ //Weighted random TODO: will need to deal with empty slots)
    TileSlot &slot = getSlotAt(toCollapse);
    if(slot.tileIdBitset.count(true) == 0){
        slot.collapsedId = -2; //I guess -2 means unsolvable?
        return;
    }
    QList<short> possibleIds;
    QList<double> weightLimits = {0.0};
    for(int id = 0; id < tiles.size(); id++){
        if(slot.tileIdBitset.testBit(id)){
            possibleIds.append(id);
            weightLimits.append(tiles.at(id).weight + weightLimits.back());
        }
    }
    short tileId = possibleIds[weightedRandom(weightLimits)];
    slot.collapsedId = tileId;
    slot.tileIdBitset = QBitArray(slot.tileIdBitset.size());
    slot.tileIdBitset.setBit(tileId);
}

TileSlot& wfc::getSlotAt(const QPoint &pos){
    return grid[(pos.x() + pos.y()*this->gridWidth)];
}

bool wfc::isInBounds(const QPoint &pos){
    return (pos.x()>=0) && (pos.x()<gridWidth) && (pos.y()>=0) && (pos.y()<gridHeight);
}

QList<QPoint> wfc::getAffectedPatternCoords(const QPoint &p, int patternSize){ //NxN coords, with slot at p in the bottom right
    QList<QPoint> resultCoords;
    for(int y = -patternSize + 1; y <= 0; y++){
        for(int x = -patternSize + 1; x <= 0; x++){
            resultCoords.append(QPoint{x,y} + p);
        }
    }
    return resultCoords;
}

QList<TileSlot> wfc::getPatternTiles(const QPoint &p, int patternSize, const QList<TileSlot> &grid){ //NxN coords, with point p at the top left
    static TileSlot dummy = {QBitArray{grid.first().tileIdBitset.size(),true}, {-1,-1}}; //Dummy Slot, for applying patterns on out of bounds tiles

    auto isUnsolvable = [](const TileSlot &t){return *t.tileIdBitset.bits() == 0;}; //test

    QList<TileSlot> resultSlots;

    for(int y = p.y(); y < p.y() + patternSize; y++){
        for(int x = p.x(); x < p.x() + patternSize; x++){
            if(isInBounds({x,y}) && !isUnsolvable(getSlotAt({x,y}))) //test
                resultSlots.append(getSlotAt({x,y})); //TODO: addDummySlots
            else
                resultSlots.append(dummy);
        }
    }
    return resultSlots;
}

bool doesPatternFit(const Pattern &p, const QList<QBitArray> &bitCube){ //TODO: dont i only have to check the updated tile?
    for(int i = 0; i < p.size*p.size; i++){
        if(bitCube.at(i).testBit(p.tileIDs.at(i)) == false)
            return false;
    }
    return true;
}

void updateCube(const Pattern &p, QList<QBitArray> &bitCube){
    for(int i = 0; i < p.size*p.size; i++){
        bitCube[i].setBit(p.tileIDs.at(i));
    }
}

QList<QPoint> wfc::propagateUpdate(QList<TileSlot> &grid, const QPoint &collapsed, const QList<Pattern> &patterns, const QList<Tile> &tiles){ //should probably disentagle this mess. unless it works, then fuck it
    int patternSize = patterns.first().size;
    QList<QPoint> updatedSlots = {collapsed}; //maybe set?
    //QList<QPoint> resultUpdatedSlots; test

    auto isUnsolvable = [](const TileSlot &t){return *t.tileIdBitset.bits() == 0;};

    auto printBitCube = [](const QList<QBitArray> &bitcube, QString name = ""){
        qDebug()<<name<<":[";
        for(auto b: bitcube){
            qDebug()<<b<<" ";
        }
        qDebug()<<"]"<<Qt::endl;
    };

    int currSlotIndex = 0;
    while(currSlotIndex < updatedSlots.size()){
        qDebug()<<"propagation: updatedSlotAmount:"<<updatedSlots.size();

        QList<QPoint> testPatternCoords = getAffectedPatternCoords(updatedSlots.at(currSlotIndex), patternSize);
        //resultUpdatedSlots.append(updatedSlots.takeFirst());

        for(auto p: testPatternCoords){
            QList<TileSlot> slotsInPattern = getPatternTiles(p, patternSize, grid);
            QList<QBitArray> oldBitCube;
            for(auto t:slotsInPattern){
                oldBitCube.append(t.tileIdBitset); //fills the bitCube
            }
            QList<QBitArray> newBitCube{patternSize * patternSize, QBitArray{tiles.size(),false}}; //test this shit
            for(auto p: patterns){
                if(doesPatternFit(p,oldBitCube))
                    updateCube(p, newBitCube);
            }
            for(int i = 0; i < patternSize * patternSize; i++){ //update slots
                QPoint slotPos = slotsInPattern[i].pos;
                if((oldBitCube.at(i) != newBitCube.at(i)) && isInBounds(slotPos) && (!isUnsolvable(slotsInPattern[i]))){
                    getSlotAt(slotPos).tileIdBitset = newBitCube.at(i);

                    if(*newBitCube.at(i).bits() <= 1) //test
                        collapseSlot(slotPos, tiles);
                    else if(!updatedSlots.contains(slotPos))
                        updatedSlots.append(slotPos);

                    //printBitCube(oldBitCube,"oldCube");
                    //printBitCube(newBitCube,"newCube");
                }
            }
        }
        currSlotIndex++;
    }
    updatedSlots.pop_front(); //remove the collapsed TileSlot. Necessary. May need to keep track of updated slots somehow
    return updatedSlots; //test this whole thing
}

QList<TileSlot> wfc::generateGridStep(QList<TileSlot> &grid, const QList<Tile> &tiles, int width, int height){
    qDebug()<<"Began generating grid step";

    QPoint toCollapse = collapseCandidatePos.takeFirst();

    collapseSlot(toCollapse, tiles);
    auto updatedSlots = propagateUpdate(grid, toCollapse, patterns, tiles);
    for(QPoint sid:updatedSlots){
        if(!collapseCandidatePos.contains(sid)) //still doesnt work properly apparently
            collapseCandidatePos.append(updatedSlots);
    }


    displayGrid(grid, tiles, width, height); //TEST

    if(collapseCandidatePos.empty()){ //if no slots were updated, take the first avalible slot, if possible
        for(auto ts:grid)
            if(ts.collapsedId == -1){
                collapseCandidatePos.append(ts.pos);
                break;
            }
    }
    else{
        std::sort(collapseCandidatePos.begin(), collapseCandidatePos.end(),
                  [&](const QPoint &a, const QPoint &b){return getSlotAt(a).tileIdBitset.count(true) < getSlotAt(b).tileIdBitset.count(true);}
                  );
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
    return grid;
}

void wfc::displayGrid(const QList<TileSlot> &grid, const QList<Tile> &tiles, int width, int height){
    this->generatorView->view()->scene()->clear(); //Needed?
    int tileSize = tiles.first().size;

        for (auto ts: grid) { //Display the tiles
            QGraphicsItem *item;
            if(ts.collapsedId >= 0){
                item = new TileGraphicsItem(tiles.at(ts.collapsedId));
            }
            else if(ts.collapsedId == -1){ //Display uncollapsed tile
                QString text;
                for(int i = 0; i < ts.tileIdBitset.size(); i++)
                    text.append((ts.tileIdBitset.testBit(i)) ? '1' : '0');

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

void wfc::generate(){
    if(tiles.empty() || patterns.empty() || (gridWidth <= 0) || (gridHeight <= 0))
        throw("Error");
    this->grid = createEmptyGrid(tiles, gridWidth, gridHeight);
    while(!collapseCandidatePos.empty()){
        qDebug()<<collapseCandidatePos.size()<<" slots left to collapse";
        generateGridStep(this->grid,this->tiles,this->gridWidth, this->gridHeight);
    }
}

void wfc::generateOneStep(){
    if(tiles.empty() || patterns.empty() || (gridWidth <= 0) || (gridHeight <= 0))
        throw("Error");
    if(!collapseCandidatePos.empty())
        generateGridStep(this->grid,this->tiles,this->gridWidth, this->gridHeight);
}

void wfc::setPatterns(QList<Tile> newTiles, QList<Pattern> newPatterns){
    this->tiles = newTiles;
    this->patterns = newPatterns;
    this->grid = createEmptyGrid(tiles, gridWidth, gridHeight);
    qDebug()<<"Received tiles ("<<newTiles.size()<<") and patterns("<<newPatterns.size()<<")";
}

void wfc::changeGridHeight(int val){
    if(val > 0)
        this->gridHeight = val;
}

void wfc::changeGridWidth(int val){
    if(val > 0)
        this->gridWidth = val;
}

void wfc::clearGrid(){
    this->grid = createEmptyGrid(tiles, gridWidth, gridHeight);
    this->generatorView->view()->scene()->clear();
}
