// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TILE_H
#define TILE_H

#include <QColor>
#include <QGraphicsItem>

class Tile;

class TileGraphicsItem : public QGraphicsItem
{
public:
    TileGraphicsItem(const QImage &image, int x, int y);

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
    int x;
    int y;
    QImage image; //TODO
    Tile *refTile;
    QList<QPointF> stuff;
};

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
};

class Pattern
{
private:
    static int commonID;

public:
    Pattern(const QList<short> tileIDs, int size = 3, int weight = 1, int id = commonID++);

    int id;
    QString hash;
    QList<short> tileIDs;
    int size;
    qreal weight;
    void setWeight(qreal);
};

#endif // TILE_H
