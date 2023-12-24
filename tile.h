// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TILE_H
#define TILE_H

#include <QColor>
#include <QGraphicsItem>

//Should seperate tile and pattern?

class Tile
{
private:
    static int commonID;
public:
    Tile(QImage image, int size, int id = commonID++);
    int id;
    //hash?
    QString hash;
    //get image
    QImage image;
    int size;
    //weight
    qreal weight;
    //surrounded weights?
    //edges?
    void setWeight(qreal newWeight);
    void incrementWeight();
    bool operator== (const Tile &tile) const;
    bool operator== (Tile &tile);
    bool operator== (QImage &image);
    bool operator== (const QImage &image) const;
    static void displayTiles(QList<Tile> tiles, int x, int y);
};

class TileGraphicsItem : public QGraphicsItem
{
public:
    TileGraphicsItem(const Tile &tile);

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
    Tile refTile;
    QList<QPointF> stuff; //what
};

class Pattern
{
private:
    static int commonID;

public:
    Pattern(const QList<short> tileIDs, int size = 3, int weight = 1, int id = commonID++);

    int id;
    QString hash;
    QList<short> tileIDs; //pattern is a square, starting from the top left corner
    int size;
    qreal weight;
    void setWeight(qreal);
    void incrementWeight();
    bool operator== (const Pattern &pattern) const;
    bool operator== (Pattern &pattern);
    bool operator== (QList<short> &list);
    bool operator== (const QList<short> &list) const;

    QVector2D indexToVector(int index);
    static void displayPatterns(QList<Pattern> patterns, int x, int y);
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
