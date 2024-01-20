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
    explicit PatternLibrary(QComboBox *patternSelector, QGraphicsView *patternView, QObject *parent = nullptr);

    QList<Tile> tiles;
    QList<Pattern> originalPatterns;
    QList<Pattern> patterns;
    QComboBox *patternSelector;
    QGraphicsView *patternView;
    int selectedPatternId = -1;

signals:
    void sendTilesPatterns(QList<Tile> newTiles, QList<Pattern> newPatterns);
    void displayPatternWeight(double weight);
    void displayPatternEnabled(bool checked);

public slots:
    void setTilesPatterns(QList<Tile> newTiles, QList<Pattern> newPatterns);
    void showPatternInfo(int id);
    void setEnabled(bool enabled);
    void setWeight(double weight);
    void resetPattern();
    void sendOutTiles();
};
#endif // PATTERNLIBRARY_H
