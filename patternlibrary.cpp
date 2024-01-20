#include "patternlibrary.h"

PatternLibrary::PatternLibrary(QComboBox *patternSelector, QGraphicsView *patternView, QObject *parent)
    : QObject{parent}, patternSelector(patternSelector), patternView(patternView)
{
    this->tiles = QList<Tile>{};
    this->patterns = QList<Pattern>{};
    this->originalPatterns = QList<Pattern>{};
    patternView->setScene(new QGraphicsScene(this));
    connect(patternSelector, &QComboBox::activated, this, &PatternLibrary::showPatternInfo);
}

void PatternLibrary::setTilesPatterns(QList<Tile> newTiles, QList<Pattern> newPatterns){
    this->tiles = newTiles;
    this->patterns = newPatterns;
    this->originalPatterns = newPatterns;

    patternSelector->clear();
    selectedPatternId = -1;

    for(const Pattern& p: patterns){
        patternSelector->insertItem(p.id, QString("Pattern #").append(std::to_string(p.id)));
    }
}

void PatternLibrary::showPatternInfo(int id){
    if(id < 0 || patterns.empty())
        return;

    selectedPatternId = id;

    const Pattern &selectedPattern = patterns.at(selectedPatternId);

    emit displayPatternEnabled(selectedPattern.enabled);
    emit displayPatternWeight(selectedPattern.weight);

    patternView->scene()->clear();
    auto patternItem = new PatternGraphicsItem(selectedPattern, tiles);
    patternView->scene()->addItem(patternItem);
    patternView->fitInView(patternItem);
}

void PatternLibrary::setWeight(double newWeight){
    if(selectedPatternId < 0 || patterns.empty())
        return;

    Pattern &selectedPatternRef = patterns[selectedPatternId];
    selectedPatternRef.setWeight(newWeight);
}

void PatternLibrary::setEnabled(bool isEnabled){
    if(selectedPatternId < 0 || patterns.empty())
        return;

    Pattern &selectedPatternRef = patterns[selectedPatternId];
    selectedPatternRef.enabled = isEnabled;
}

void PatternLibrary::resetPattern(){
    if(selectedPatternId < 0 || patterns.empty())
        return;

    patterns = originalPatterns;

    const Pattern &selectedPattern = patterns.at(selectedPatternId);
    emit displayPatternEnabled(selectedPattern.enabled);
    emit displayPatternWeight(selectedPattern.weight);
}

void PatternLibrary::sendOutTiles(){
    emit sendTilesPatterns(tiles, patterns);
}


