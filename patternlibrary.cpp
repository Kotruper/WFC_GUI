#include "patternlibrary.h"

PatternLibrary::PatternLibrary(QComboBox *patternSelector, QGraphicsView *patternView, QComboBox *tileSelector, QGraphicsView *tileView, QObject *parent)
    : QObject{parent}, patternSelector(patternSelector), patternView(patternView), tileSelector(tileSelector), tileView(tileView)
{
    this->tiles = QList<Tile>{};
    this->originalTiles = QList<Tile>{};
    this->patterns = QList<Pattern>{};
    this->originalPatterns = QList<Pattern>{};

    patternView->setScene(new QGraphicsScene(this));
    tileView->setScene(new QGraphicsScene(this));
    connect(patternSelector, &QComboBox::activated, this, &PatternLibrary::showElementInfo);
    connect(tileSelector, &QComboBox::activated, this, &PatternLibrary::showElementInfo);
}

LibraryElement& PatternLibrary::getElementRefAt(int id){
    if(patternsTabSelected)
        return patterns[id];
    else
        return tiles[id];
}

void PatternLibrary::setTilesPatterns(QList<Tile> newTiles, QList<Pattern> newPatterns){
    this->tiles = newTiles;
    this->originalTiles = newTiles;
    this->patterns = newPatterns;
    this->originalPatterns = newPatterns;

    selectedPatternId = (patterns.isEmpty()) ? -1 : 0;
    selectedTileId = (tiles.isEmpty()) ? -1 : 0;

    patternSelector->clear();
    tileSelector->clear();
    patternSelector->setCurrentIndex(selectedPatternId); //why doesnt work
    tileSelector->setCurrentIndex(selectedTileId);

    for(const Pattern& p: patterns){
        patternSelector->insertItem(p.id, p.getElementIcon(tiles), QString("Pattern #").append(std::to_string(p.id)));
    }
    for(const Tile& t: tiles){
        tileSelector->insertItem(t.id, t.getElementIcon({}), QString("Tile #").append(std::to_string(t.id)));
    }
}

void PatternLibrary::setSelectedTab(int tabIndex){
    if(tabIndex == 0){
        patternsTabSelected = true;
        showElementInfo(selectedPatternId);
    }
    else{
        patternsTabSelected = false;
        showElementInfo(selectedTileId);
    }
}

void PatternLibrary::showElementInfo(int id){
    if(id < 0 || patterns.empty() || tiles.empty())
        return;

    const LibraryElement &selectedElement = getElementRefAt(id);
    QGraphicsView* elementView;

    if(patternsTabSelected){
        selectedPatternId = id;
        elementView = patternView;
    }
    else{
        selectedTileId = id;
        elementView = tileView;
    }

    emit displayPatternEnabled(selectedElement.enabled);
    emit displayPatternWeight(selectedElement.weight);

    elementView->scene()->clear();
    auto elementItem = selectedElement.getGraphicsItem(tiles);
    elementView->scene()->addItem(elementItem);
    elementView->fitInView(elementItem);
}

void PatternLibrary::setElementWeight(double newWeight){
    if(selectedPatternId < 0 || patterns.empty() || tiles.empty())
        return;

    LibraryElement &selectedElement = getElementRefAt((patternsTabSelected)?(selectedPatternId):(selectedTileId));
    selectedElement.setWeight(newWeight);
}

void PatternLibrary::setElementEnabled(bool isEnabled){
    if(selectedPatternId < 0 || patterns.empty() || tiles.empty())
        return;

    LibraryElement &selectedElement = getElementRefAt((patternsTabSelected)?(selectedPatternId):(selectedTileId));
    selectedElement.enabled = isEnabled;
}

void PatternLibrary::resetElement(){
    if(selectedPatternId < 0 || patterns.empty() || tiles.empty())
        return;

    if(patternsTabSelected)
        patterns = originalPatterns;
    else
        tiles = originalTiles;

    const LibraryElement &selectedElement = getElementRefAt((patternsTabSelected)?(selectedPatternId):(selectedTileId));
    emit displayPatternEnabled(selectedElement.enabled);
    emit displayPatternWeight(selectedElement.weight);
}

void PatternLibrary::sendOutTilesPatterns(){
    emit sendTilesPatterns(tiles, patterns);
}


