#include "wfc.h"
#include <QRandomGenerator>

wfc::wfc(View *generatorView, QObject *parent)
    : QObject{parent}, generatorView(generatorView)
{
    gridHeight = 5; //connect these later
    gridWidth = 5;
}

QList<TileSlot> wfc::clearGrid(const QList<Tile> &tiles, int width, int height){
    auto newGrid = QList<TileSlot>{};

    int tileAmount = tiles.size(); //if any tiles are disabled, make sure to 0 out those ones
    for(int y=0; y < height; y++){
        for(int x=0; x < width; x++){
            newGrid.append(TileSlot{QBitArray(tileAmount, true),x,y});
        }
    }
    TileSlot &middleSlot = newGrid[(height*width)/2];
    this->collapsableSlots.clear();
    this->collapsableSlots.append(std::move(middleSlot));
    return newGrid;
}

void wfc::collapseSlot(TileSlot &slot, const QList<Tile> &tiles){
    QList<short> possibleIds;
    QList<double> weightLimits = {0.0};
    for(int id = 0; id < tiles.size(); id++){
        if(slot.tileIdBitset.testBit(id)){
            possibleIds.append(id);
            weightLimits.append(tiles.at(id).weight + weightLimits.back());
        }
    }
    double randomWeight = QRandomGenerator::global()->bounded(weightLimits.last()); //TODO: add random seed
    auto tileIndex = std::distance(weightLimits.begin(), std::upper_bound(weightLimits.begin(),weightLimits.end(),randomWeight)) - 1;

    slot.collapsedId = tileIndex;
    slot.tileIdBitset = QBitArray(slot.tileIdBitset.size());
    slot.tileIdBitset.setBit(possibleIds.at(tileIndex));
}

TileSlot& wfc::getSlotAt(int x, int y){
    return grid[(x + y*this->gridWidth)];
}

bool wfc::isInBounds(int x, int y){
    return (x>=0) && (x<gridWidth) && (y>=0) && (y<gridHeight);
}

QList<Coords> wfc::getAffectedPatternCoords(const TileSlot &t, int patternSize){ //NxN coords, top left to the Slot
    QList<Coords> resultCoords;
    for(int y = -patternSize; y <= 0; y++){
        for(int x = -patternSize; x <= 0; x++){
            resultCoords.append(Coords{t.x + x, t.y + y});
        }
    }
    return resultCoords;
}

QList<TileSlot> wfc::getPatternTiles(const Coords &c, const TileSlot &dummy, int patternSize, QList<TileSlot> &grid){ //NxN coords, bottom right of the Coords
    QList<TileSlot> resultSlots;
    for(int y = c.y; y < c.y + patternSize; y++){
        for(int x = c.x; x < c.x + patternSize; x++){
            if(isInBounds(x,y))
                resultSlots.append(getSlotAt(x,y)); //TODO: addDummySlots
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

QList<TileSlot> wfc::propagateUpdate(QList<TileSlot> &grid, const TileSlot &collapsed, const QList<Pattern> &patterns, const QList<Tile> &tiles){ //should probably disentagle this mess. unless it works, then fuck it
    int patternSize = patterns.first().size;
    QList<TileSlot> updatedSlots = {collapsed}; //maybe set?
    QList<TileSlot> resultUpdatedSlots;
    TileSlot dummySlot = {QBitArray{tiles.size(),true},-1,-1};
    auto isSlotDummy = [](TileSlot t){return t.x < 0;};

    while(!updatedSlots.empty()){

        QList<Coords> testPatternCoords = getAffectedPatternCoords(updatedSlots.first(), patternSize);
        resultUpdatedSlots.append(updatedSlots.front());
        updatedSlots.pop_front();
        for(auto c: testPatternCoords){
            QList<TileSlot> slotsInPattern = getPatternTiles(c, dummySlot, patternSize, grid);
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
                if((oldBitCube.at(i) != newBitCube.at(i)) && !isSlotDummy(slotsInPattern[i])){
                    slotsInPattern[i].tileIdBitset = newBitCube.at(i);
                    updatedSlots.append(slotsInPattern[i]);
                }
            }
        }
    }
    resultUpdatedSlots.pop_front(); //remove the collapsed TileSlot. May need to keep track of updated slots somehow
    return resultUpdatedSlots; //test this whole thing, make sure references work. Avoid doing things by reference in the future, Jesus Christ
}

QList<TileSlot> wfc::generateGrid(QList<TileSlot> &grid, const QList<Tile> &tiles, int width, int height){
    qDebug()<<"Began generating grid";
    while(!collapsableSlots.empty()){
        TileSlot &toCollapse = collapsableSlots.first();
        collapsableSlots.pop_front();

        collapseSlot(toCollapse, tiles);
        auto updatedSlots(propagateUpdate(grid, toCollapse, patterns, tiles));
        collapsableSlots.append(updatedSlots);

        displayGrid(grid, tiles, width, height); //TEST

        std::sort(collapsableSlots.begin(),collapsableSlots.end(),
                  [](const TileSlot &a, const TileSlot &b){return a.tileIdBitset.count(true) < b.tileIdBitset.count(true);}
                 );
    }
    /*
        while there are uncollapsed (or empty?) slots (in the updated list), do: while(!collapsableSlots.empty())
            get the slot with the lowest entropy (TileSlot &currentSlot = collapsableSlots.first()) <- put in references!
            collapse it by tile weight (TileSlot.collapse(const QList<Tile> &tiles)), maybe by pattern weight too? how?
            propagate update: (propagateUpdate(TileSlot t)), needs x and y in tileslot
                get a list of coordinates to check patterns at (NxN, out of bounds allowed) (return range x-n+1 - x, y-n+1 - y of coords)
                for each coordinate, generate based on the patterns a list of possible tiles to place in the slot (maybe should be boolean/bitset?) (uhh)
                    out of bounds count as uncollapsed (if( isOutOfBounds(coords)) behave as if its TileId list is full
                    compare the generated list with the actual list in the tile (if TileSlot.idList ^ newLists.at(innerCords/index?) == 0)
                    if the lists are different:
                        update the tile and insert into the list of coordinates affected patterns (TileSlot.idList = newLists.at(innerCords/index), then same as step 2a
                repeat, until the coordinate list is empty (done by the while loop)
            sort collapsableSlots by entropy (lenght) (ez)

            TODO: dummy slots for out of bounds(v), updating the view, does this shit even work?
     */
    return grid;
}

void wfc::displayGrid(const QList<TileSlot> &grid, const QList<Tile> &tiles, int width, int height){
    this->generatorView->view()->scene()->clear(); //Needed?
    int tileSize = tiles.first().size;

        for (auto ts: grid) { //Display the tiles
            if(ts.collapsedId >= 0){
                QGraphicsItem *item = new TileGraphicsItem(tiles.at(ts.collapsedId));
                item->setPos(QPointF(ts.x * tileSize, ts.y * tileSize));
                generatorView->view()->scene()->addItem(item);
            }
            //qDebug()<<tiles[ts.collapsedId].id;
        }
}

void wfc::generate(){
    if(tiles.empty() || patterns.empty() || (gridWidth <= 0) || (gridHeight <= 0))
        throw("Error");
    this->grid = clearGrid(tiles, gridWidth, gridHeight);
    generateGrid(this->grid,this->tiles,this->gridWidth, this->gridHeight);
}

void wfc::setPatterns(QList<Tile> newTiles, QList<Pattern> newPatterns){
    this->tiles = newTiles;
    this->patterns = newPatterns;
    qDebug()<<"Received tiles ("<<newTiles.size()<<") and patterns("<<newPatterns.size()<<")";
}
