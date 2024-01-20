#ifndef PATTERNSETTINGSEDITOR_H
#define PATTERNSETTINGSEDITOR_H

#include "tile.h"
#include <QObject>

class PatternSettingsEditor : public QObject
{
    Q_OBJECT
public:
    explicit PatternSettingsEditor(QObject *parent = nullptr);

    QList<Tile> tiles;
    QList<Pattern> patterns;

signals:
    void sendPatterns(QList<Tile> tiles, QList<Pattern> patterns);

private slots:
    void receivePatterns(QList<Tile> tiles, QList<Pattern> patterns);

};

#endif // PATTERNSETTINGSEDITOR_H
