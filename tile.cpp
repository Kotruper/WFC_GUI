// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "tile.h"
#include "qvectornd.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

TileGraphicsItem::TileGraphicsItem(const Tile& tile): refTile(tile)
{
}

QRectF TileGraphicsItem::boundingRect() const
{
    return this->refTile.image.rect();
}

QPainterPath TileGraphicsItem::shape() const
{
    QPainterPath path;
    path.addRect(this->refTile.image.rect());
    return path;
}

void TileGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    painter->drawImage(this->boundingRect(),this->refTile.image);

}

Tile::Tile(int id, QImage image, int size)
    :id(id), image(image), size(size)
{
}

void Tile::setWeight(qreal newWeight){
    this->weight = newWeight;
}

void Tile::incrementWeight(int n){
    this->weight+=n;
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

Pattern::Pattern(int id, QList<short> tileIDs, int size, int weight)
    :id(id), tileIDs(tileIDs), size(size), weight(weight)
{
}

void Pattern::incrementWeight(int n){
    this->weight+=n;
}

QPoint Pattern::indexToPos(const int &index){
    return QPoint(index % size, index / size);
}

short Pattern::getIdFromPos(const QPoint &pos){
    return this->tileIDs.at(pos.x() + pos.y()*this->size);
}

bool Pattern::operator==(QList<short> &other){
    return this->tileIDs == other;
}

bool Pattern::operator==(const QList<short> &other) const{
    return this->tileIDs == other;
}

bool Pattern::operator==(const Pattern &other) const{
    return this->tileIDs == other.tileIDs;
}

bool Pattern::operator==(Pattern &other){
    return this->tileIDs == other.tileIDs;
}

PatternGraphicsItem::PatternGraphicsItem(const Pattern &pattern, const QList<Tile> &tiles): refPattern(pattern), tileset(tiles)
{
}

QRectF PatternGraphicsItem::boundingRect() const
{
    QRectF tileRect = tileset.first().image.rect();
    return QRectF(tileRect.topLeft(), tileRect.bottomRight() * refPattern.size);
}

QPainterPath PatternGraphicsItem::shape() const
{
    QPainterPath path;
    path.addRect(this->boundingRect());
    return path;
}

void PatternGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    for(int i = 0;i<refPattern.tileIDs.size();i++){
        Tile tileToDraw = tileset.at(refPattern.tileIDs.at(i));
        QPoint offset = refPattern.indexToPos(i) * tileToDraw.size;
        painter->setOpacity(1); //thingy
        painter->drawImage(offset, tileToDraw.image);
    }
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
