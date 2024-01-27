// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TILE_H
#define TILE_H

#include <QBitArray>
#include <QObject>
#include <QGraphicsItem>

//Should seperate tile and pattern?

class Tile;

class LibraryElement
{
public:
    LibraryElement(int id, int size, qreal weight = 1.0);
    int id;
    qreal weight;
    int size;
    bool enabled = true;
    virtual void setWeight(qreal newWeight) = 0;
    virtual QIcon getElementIcon(const QList<Tile> &tiles) const = 0;
    virtual QGraphicsItem* getGraphicsItem(const QList<Tile> &tiles) const = 0;
};

class Tile : public LibraryElement
{
public:
    Tile(int id, QImage image, int size, qreal weight = 1.0);
    QImage image;
    QPixmap pixmap;

    void incrementWeight(int incVal = 1);
    virtual void setWeight(qreal newWeight) override;
    virtual QIcon getElementIcon(const QList<Tile> &tiles = {}) const override;
    virtual QGraphicsItem* getGraphicsItem(const QList<Tile> &tiles = {}) const override;

    bool operator== (const Tile &tile) const;
    bool operator== (Tile &tile);
    bool operator== (QImage &image);
    bool operator== (const QImage &image) const;
    static void displayTiles(QList<Tile> tiles, int x, int y);
    static const Tile& getWallTile();
};

class Pattern;

class TileGraphicsItem : public QGraphicsItem
{
public:
    TileGraphicsItem(const Tile &tile);
    TileGraphicsItem(const QList<Tile> &tiles);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;
    /*
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
*/
    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent* e) override;
private:
    QList<Tile> refTiles;
};

using CompatibilityList = QList<QList<QBitArray>>; //dy, dx, patternIDs?, can store 128 * 128 patterns

class Pattern : public LibraryElement
{

public:
    Pattern(int id, QList<short> tileIDs, int size, int weight = 1);

    QList<short> tileIDs; //pattern is a square, starting from the top left corner
    CompatibilityList compatibilityList;

    virtual void setWeight(qreal newWeight) override;
    virtual QIcon getElementIcon(const QList<Tile> &tiles) const override;
    virtual QGraphicsItem* getGraphicsItem(const QList<Tile> &tiles) const override;

    void incrementWeight(int incVal = 1);
    QPoint indexToPos(const int &index);
    const short getTileIdAtPos(const QPoint &pos) const;
    QBitArray& getCompabilityListRefAt(const QPoint &pos);
    bool isCompatibleAt(const Pattern &otherP, const QPoint &pos);
    static void displayPatterns(QList<Pattern> patterns, int x, int y);

    bool operator== (const Pattern &pattern) const;
    bool operator== (Pattern &pattern);
    bool operator== (QList<short> &list);
    bool operator== (const QList<short> &list) const;
};

class PatternGraphicsItem : public QGraphicsItem
{
public:
    PatternGraphicsItem(const Pattern &pattern, const QList<Tile> &tiles);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;
    /*
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
*/
private:
    Pattern refPattern;
    QList<Tile> tileset;
    QList<QPointF> stuff; //what
};

#endif // TILE_H
