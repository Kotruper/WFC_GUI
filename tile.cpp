// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "tile.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

int Tile::commonID = 0;
int Pattern::commonID = 0;

TileGraphicsItem::TileGraphicsItem(const QImage &image, int x, int y)
{
    this->x = x;
    this->y = y;
    this->image = image;
    setZValue((x + y) % 2);

    //setFlags(ItemIsSelectable | ItemIsMovable);
    //setAcceptHoverEvents(true);
}

QRectF TileGraphicsItem::boundingRect() const
{
    return this->image.rect();
}

QPainterPath TileGraphicsItem::shape() const
{
    QPainterPath path;
    path.addRect(this->image.rect());
    return path;
}

void TileGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    painter->drawImage(this->boundingRect(),this->image);

}

Tile::Tile(QImage image, int size, int id){
    this->image = image;
    this->size = size;
    this->id = id;
}

void Tile::setWeight(qreal newWeight){
    this->weight = newWeight;
}

void Tile::incrementWeight(){
    this->weight++;
}

bool Tile::operator==(QImage &other){
    return this->image == other;
}

bool Tile::operator==(const QImage &other) const{
    return this->image == other;
}

bool Tile::operator==(const Tile &other) const{
    return this->image == other.image;
}

bool Tile::operator==(Tile &other){
    return this->image == other.image;
}

Pattern::Pattern(const QList<short> tileIDs, int size, int weight, int id){
    this->tileIDs = tileIDs;
    this->weight = weight;
    this->size = size;
    this->id = id;
}
/*
void Chip::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    update();
}

void Chip::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->modifiers() & Qt::ShiftModifier) {
        stuff << event->pos();
        update();
        return;
    }
    QGraphicsItem::mouseMoveEvent(event);
}

void Chip::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    update();
}
*/
