// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "tile.h"
#include "qbitarray.h"

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
////////////////////////////////////////////////////////////

Pattern::Pattern(int id, QList<short> tileIDs, int size, int weight)
    :id(id), tileIDs(tileIDs), size(size), weight(weight)
{
    int patternCheckSize = size * 2 - 1;
    this->compatibilityList = CompatibilityList{patternCheckSize,QList<QBitArray>{patternCheckSize, QBitArray()}};
}

void Pattern::incrementWeight(int n){
    this->weight+=n;
}

void Pattern::setWeight(double newWeight){
    this->weight = newWeight;
}

QPoint Pattern::indexToPos(const int &index){
    return QPoint(index % size, index / size);
}

const short Pattern::getTileIdAtPos(const QPoint &pos) const{
    return this->tileIDs.at(pos.x() + pos.y()*this->size);
}

QBitArray& Pattern::getCompabilityListRefAt(const QPoint &pos){//accepts -size to +size
    return this->compatibilityList[pos.y() + size - 1][pos.x() + size - 1];
}

bool Pattern::isCompatibleAt(const Pattern &otherPattern, const QPoint &pos){// how the fuck do you compose??? two patterns. also, is pos just a vector?
    int minX = (pos.x() < 0) ? 0 : pos.x(); //good
    int maxX = (pos.x() >= 0) ? this->size : pos.x() + size; //good. technically gives +1, but should cancel out in the loops below
    int minY = (pos.y() < 0) ? 0 : pos.y(); //good
    int maxY = (pos.y() >= 0) ? this->size : pos.y() + size; //good
    for(int dy = minY; dy < maxY; dy++){ //less than is important
        for(int dx = minX; dx < maxX; dx++){
            if(this->getTileIdAtPos({dx,dy}) != otherPattern.getTileIdAtPos(QPoint{dx,dy} - pos)){
                return false;
            }
        }
    }
    return true;
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
    for(int i = 0; i<refPattern.tileIDs.size(); i++){
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
