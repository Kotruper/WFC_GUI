// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "view.h"

#include <QtWidgets>
#include <QtMath>

#if QT_CONFIG(wheelevent)
void GraphicsView::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() & Qt::ControlModifier) {
        if (e->angleDelta().y() > 0)
            view->zoomInBy(6);
        else
            view->zoomOutBy(6);
        e->accept();
    } else {
        QGraphicsView::wheelEvent(e);
    }
}
#endif

void GraphicsView::setPatternSize(int size){
    this->patternSize = size;
    this->resetCachedContent();
}
void GraphicsView::setTileSize(int size){
    this->tileSize = size;
    this->resetCachedContent();
}

void GraphicsView::drawForeground(QPainter *painter, const QRectF &rect)
{
    const int &tileGridSize = this->tileSize;
    const int &patternGridSize = this->patternSize;

    if(tileGridSize < 1 || patternGridSize < 1)
        return;

    qreal left = int(rect.left()) - (int(rect.left()) % (tileGridSize * patternGridSize));
    qreal top = int(rect.top()) - (int(rect.top()) % (tileGridSize * patternGridSize));

    painter->setOpacity(0.1);

    QVarLengthArray<QLineF> tileLines;

    for (qreal x = left; x < rect.right(); x += tileGridSize)
        tileLines.append(QLineF(x, rect.top(), x, rect.bottom()));
    for (qreal y = top; y < rect.bottom(); y += tileGridSize)
        tileLines.append(QLineF(rect.left(), y, rect.right(), y));

    painter->setPen(QPen(QBrush(Qt::blue, Qt::BrushStyle::SolidPattern),0));
    painter->drawLines(tileLines.data(), tileLines.size());

    QVarLengthArray<QLineF, 100> patternLines;

    for (qreal x = left; x < rect.right(); x += tileGridSize * patternGridSize)
        patternLines.append(QLineF(x, rect.top(), x, rect.bottom()));
    for (qreal y = top; y < rect.bottom(); y += tileGridSize * patternGridSize)
        patternLines.append(QLineF(rect.left(), y, rect.right(), y));

    painter->setPen(QPen(QBrush(Qt::red, Qt::BrushStyle::SolidPattern),0));
    painter->drawLines(patternLines.data(), patternLines.size());
}

void GraphicsView::dropEvent(QDropEvent *event){
    qDebug()<<"GV, Drop event received: "<<event->mimeData()<<","<<event->mimeData()->text();
    if(event->mimeData()->hasUrls() && event->mimeData()->urls().first().isLocalFile()){
        //qDebug()<<"has urls too! Should check for file though"<<event->mimeData()->urls().first().toLocalFile();
        emit sendFile(event->mimeData()->urls().first().toLocalFile());
    }
    QWidget::dropEvent(event);
}

void GraphicsView::dragMoveEvent(QDragMoveEvent *event){
    //qDebug()<<"GV, move event";
    QWidget::dragMoveEvent(event);
}

void GraphicsView::dragEnterEvent(QDragEnterEvent *event){
    //qDebug()<<"GV, Drag Enter event received";
    event->accept();
    QWidget::dragEnterEvent(event);
}

///////////////////////////////////////////////////////////////////////////////////////////

View::View(QWidget *parent)
    : QFrame(parent)
{
    setFrameStyle(NoFrame);
    graphicsView = new GraphicsView(this);
    graphicsView->setRenderHint(QPainter::Antialiasing, false);
    graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    graphicsView->setOptimizationFlags(QGraphicsView::DontSavePainterState | QGraphicsView::DontAdjustForAntialiasing);
    graphicsView->setCacheMode(QGraphicsView::CacheModeFlag::CacheBackground); //creates even bigger stripes
    graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    //graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate); //test. Seems pretty good for this, as there are a lot of small updates
    graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    //graphicsView->setBackgroundBrush(QBrush(Qt::BrushStyle::Dense7Pattern));

    int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    QSize iconSize(size, size);

    QToolButton *zoomInIcon = new QToolButton;
    zoomInIcon->setAutoRepeat(true);
    zoomInIcon->setAutoRepeatInterval(33);
    zoomInIcon->setAutoRepeatDelay(0);
    zoomInIcon->setIcon(QPixmap(":/zoomin.png"));
    zoomInIcon->setIconSize(iconSize);
    QToolButton *zoomOutIcon = new QToolButton;
    zoomOutIcon->setAutoRepeat(true);
    zoomOutIcon->setAutoRepeatInterval(33);
    zoomOutIcon->setAutoRepeatDelay(0);
    zoomOutIcon->setIcon(QPixmap(":/zoomout.png"));
    zoomOutIcon->setIconSize(iconSize);
    zoomSlider = new QSlider;
    zoomSlider->setMinimum(0);
    zoomSlider->setMaximum(500);
    zoomSlider->setValue(250);
    zoomSlider->setTickPosition(QSlider::TicksRight);

    // Zoom slider layout
    QVBoxLayout *zoomSliderLayout = new QVBoxLayout;
    zoomSliderLayout->addWidget(zoomInIcon);
    zoomSliderLayout->addWidget(zoomSlider);
    zoomSliderLayout->addWidget(zoomOutIcon);

    resetButton = new QToolButton;
    resetButton->setText(tr("0"));
    resetButton->setEnabled(false);

    QGridLayout *topLayout = new QGridLayout;
    topLayout->addWidget(graphicsView, 0, 0);
    topLayout->addLayout(zoomSliderLayout, 0, 1);
    topLayout->addWidget(resetButton, 0, 1, Qt::AlignmentFlag::AlignRight); //I guess it works
    setLayout(topLayout);

    connect(resetButton, &QAbstractButton::clicked, this, &View::resetView);
    connect(zoomSlider, &QAbstractSlider::valueChanged, this, &View::setupMatrix);
    connect(graphicsView->verticalScrollBar(), &QAbstractSlider::valueChanged, this, &View::setResetButtonEnabled);
    connect(graphicsView->horizontalScrollBar(), &QAbstractSlider::valueChanged, this, &View::setResetButtonEnabled);
    connect(zoomInIcon, &QAbstractButton::clicked, this, &View::zoomIn);
    connect(zoomOutIcon, &QAbstractButton::clicked, this, &View::zoomOut);

    setupMatrix();
    //this->setAcceptDrops(true);
    //graphicsView->setAcceptDrops(true);
}

QGraphicsView *View::view() const
{
    return static_cast<QGraphicsView *>(graphicsView);
    //return graphicsView;
}

void View::resetView()
{
    zoomSlider->setValue(250);
    //rotateSlider->setValue(0);
    setupMatrix();
    graphicsView->ensureVisible(QRectF(0, 0, 0, 0));

    resetButton->setEnabled(false);
}

void View::setResetButtonEnabled()
{
    resetButton->setEnabled(true);
}

void View::setupMatrix()
{
    qreal scale = qPow(qreal(2), (zoomSlider->value() - 250) / qreal(50));

    QTransform matrix;
    matrix.scale(scale, scale);
    //matrix.rotate(rotateSlider->value());

    graphicsView->setTransform(matrix);
    setResetButtonEnabled();
}
/*
void View::togglePointerMode()
{
    graphicsView->setDragMode(selectModeButton->isChecked()
                              ? QGraphicsView::RubberBandDrag
                              : QGraphicsView::ScrollHandDrag);
    graphicsView->setInteractive(selectModeButton->isChecked());
}

void View::toggleAntialiasing()
{
    graphicsView->setRenderHint(QPainter::Antialiasing, antialiasButton->isChecked());
}
*/
void View::zoomIn()
{
    zoomSlider->setValue(zoomSlider->value() + 1);
}

void View::zoomOut()
{
    zoomSlider->setValue(zoomSlider->value() - 1);
}

void View::zoomInBy(int level)
{
    zoomSlider->setValue(zoomSlider->value() + level);
}

void View::zoomOutBy(int level)
{
    zoomSlider->setValue(zoomSlider->value() - level);
}
/*
void View::dropEvent(QDropEvent *event){
    qDebug()<<"V, Drop event received"<<event->mimeData();
    QWidget::dropEvent(event);
}

void View::dragMoveEvent(QDragMoveEvent *event){
    return;
    qDebug()<<"Drag Move event received";
    QWidget::dragMoveEvent(event);
}

void View::dragEnterEvent(QDragEnterEvent *event){
    qDebug()<<"V, Drag Enter event received";
    event->accept();
    QWidget::dragEnterEvent(event);
}
*/
