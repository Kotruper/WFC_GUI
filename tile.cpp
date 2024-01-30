// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "tile.h"
#include "qbitarray.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

LibraryElement::LibraryElement(int id, int size, qreal weight): id(id), weight(weight), size(size)
{
}

TileGraphicsItem::TileGraphicsItem(const Tile &refTile, double opacity): refTile(refTile), opacity(opacity) //is this copying?
{
}

QRectF TileGraphicsItem::boundingRect() const
{
    return this->refTile.pixmap.rect();
}

QPainterPath TileGraphicsItem::shape() const
{
    QPainterPath path;
    path.addRect(this->refTile.pixmap.rect());
    return path;
}

void TileGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    if(opacity < 1.0) painter->setOpacity(opacity);
    painter->drawPixmap(this->boundingRect().toRect(), this->refTile.pixmap);
    /*
    if(refTiles.size() <= 1)

    else{

        for(const auto &t:refTiles)
            painter->drawPixmap(this->boundingRect().toRect(), t.pixmap); //is there a pixmap blend?
        }
    */
}
void TileGraphicsItem::dragEnterEvent(QGraphicsSceneDragDropEvent* e){
    qDebug()<<"test tgi drageEnter event";
    QGraphicsItem::dragEnterEvent(e);
}

Tile::Tile(int id, QImage image, int size, qreal weight)
    :LibraryElement(id,size, weight), image(image)
{
    this->pixmap = QPixmap::fromImage(image);
}

void Tile::setWeight(qreal newWeight){
    this->weight = newWeight;
}

void Tile::incrementWeight(int n){
    this->weight+=n;
}

const Tile Tile::getWallTile(int tileSize){
    QImage img{tileSize,tileSize,QImage::Format_ARGB32};
    QPainter paint{&img};
    paint.setPen(QPen(QBrush(Qt::red,Qt::BrushStyle::SolidPattern),0));
    paint.drawLine(0,0,tileSize,tileSize);
    paint.drawLine(0,tileSize,tileSize,0);
    Tile wallTile(0,img,tileSize);
    wallTile.isWall = true;

    return wallTile;
}

QIcon Tile::getElementIcon(const QList<Tile> &tiles) const{
    return QIcon(pixmap.scaled(32,32));
}

QGraphicsItem* Tile::getGraphicsItem(const QList<Tile> &tiles) const {
    return new TileGraphicsItem(*this);
};

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
    :LibraryElement(id, size, weight), tileIDs(tileIDs)
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

const bool Pattern::isWallPattern() const{ //ONLY WORKS IF WALLS ARE ENABLED. Maybe reserve 0 for walls?
    return this->tileIDs.at(0) == 0;
}

const bool Pattern::isCornerWallPattern() const{ //ONLY WORKS IF WALLS ARE ENABLED. BREAKS WITH PATTERN SIZE == 1 Maybe reserve 0 for walls?
    return (tileIDs.at(0) == 0) && (tileIDs.at(1) == 0) && (tileIDs.at(size) == 0) && (tileIDs.at(size+1) != 0);
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

QIcon Pattern::getElementIcon(const QList<Tile> &tiles) const{ //Big test
    int size = this->size * tiles.first().size;
    QPixmap patternIcon = QPixmap(size, size);
    patternIcon.fill(Qt::transparent);
    QPainter painter{&patternIcon};
    PatternGraphicsItem(*this, tiles).paint(&painter, nullptr, nullptr);
    return QIcon(patternIcon.scaled(32,32));
}

QGraphicsItem* Pattern::getGraphicsItem(const QList<Tile> &tiles) const {
    return new PatternGraphicsItem(*this, tiles);
};

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
        const Tile &tileToDraw = tileset.at(refPattern.tileIDs.at(i));
        QPoint offset = refPattern.indexToPos(i) * tileToDraw.size;
        //painter->setOpacity(1); //thingy
        painter->drawPixmap(offset, tileToDraw.pixmap);
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
