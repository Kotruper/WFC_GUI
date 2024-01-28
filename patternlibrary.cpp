#include "patternlibrary.h"
#include "qtimer.h"

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

    QTimer::singleShot(100, [&](){emit setUIEnabled(false);}); //funky workaround :)))
}

LibraryElement& PatternLibrary::getElementRefAt(int id){
    if(patternsTabSelected)
        return patterns[id];
    else
        return tiles[id];
}

void PatternLibrary::setTilesPatterns(QList<Tile> newTiles, QList<Pattern> newPatterns, WallPos wallPos){
    this->tiles = newTiles;
    this->originalTiles = newTiles;
    this->patterns = newPatterns;
    this->originalPatterns = newPatterns;
    this->wallPos = wallPos;

    selectedPatternId = (patterns.isEmpty()) ? -1 : 0;
    selectedTileId = (tiles.isEmpty()) ? -1 : 0;

    patternSelector->clear();
    tileSelector->clear();
    patternView->scene()->clear();
    tileView->scene()->clear();

    emit setUIEnabled(patterns.isEmpty() ? false : true);

    for(const Pattern& p: patterns){
        QString name = QString("Pattern #").append(std::to_string(p.id));
        patternSelector->insertItem(p.id, p.getElementIcon(tiles), name);
    }
    for(const Tile& t: tiles){
        QString name = (t.isWall) ? ("Border Tile") : QString("Tile #").append(std::to_string(t.id));
        tileSelector->insertItem(t.id, t.getElementIcon({}), name);
    }

    showElementInfo(patternsTabSelected ? selectedPatternId : selectedTileId);

    patternSelector->setCurrentIndex(selectedPatternId); //why doesnt work
    tileSelector->setCurrentIndex(selectedTileId);
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
    elementView->scene()->setSceneRect(elementItem->boundingRect());
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
    emit sendTilesPatterns(tiles, patterns, wallPos);
}


