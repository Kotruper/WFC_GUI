#ifndef PATTERNLIBRARY_H
#define PATTERNLIBRARY_H

#include <QObject>
#include "qcombobox.h"
#include "qgraphicsview.h"
#include "tile.h"

class PatternLibrary : public QObject
{
    Q_OBJECT
public:
    explicit PatternLibrary(QComboBox *patternSelector, QGraphicsView *patternView,QComboBox *tileSelector, QGraphicsView *tileView, QObject *parent = nullptr);

    bool patternsTabSelected = true;

    QList<Tile> originalTiles;
    QList<Tile> tiles;
    QComboBox *tileSelector;
    QGraphicsView *tileView;
    int selectedTileId = -1;

    QList<Pattern> originalPatterns;
    QList<Pattern> patterns;
    QComboBox *patternSelector;
    QGraphicsView *patternView;
    int selectedPatternId = -1;

    WallPos wallPos;

    LibraryElement& getElementRefAt(int id);

signals:
    void sendTilesPatterns(QList<Tile> newTiles, QList<Pattern> newPatterns, WallPos wallPos);
    void displayPatternWeight(double weight);
    void displayPatternEnabled(bool checked);
    void setUIEnabled(bool enabled);

public slots:
    void setTilesPatterns(QList<Tile> newTiles, QList<Pattern> newPatterns, WallPos wallPos);
    void setSelectedTab(int tabNum);
    void showElementInfo(int id);
    void setElementEnabled(bool enabled);
    void setElementWeight(double weight);
    void resetElement();
    void sendOutTilesPatterns();
};
#endif // PATTERNLIBRARY_H
