// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef VIEW_H
#define VIEW_H

#include <QFrame>
#include <QGraphicsView>

QT_BEGIN_NAMESPACE
class QLabel;
class QSlider;
class QToolButton;
QT_END_NAMESPACE

class View;

class GraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    GraphicsView(View *v) : QGraphicsView(), view(v) {}
    int patternSize = -1;
    int tileSize = -1;

    void setTileSize(int size);
    void setPatternSize(int size);

signals:
    void sendFile(QString filename);
    void sendTileId(int id, QPointF pos);

protected:
#if QT_CONFIG(wheelevent)
    void wheelEvent(QWheelEvent *) override;
#endif
    void drawForeground(QPainter *painter, const QRectF &rect) override;

    void dropEvent(QDropEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;

private:
    View *view;
};

class View : public QFrame
{
    Q_OBJECT
public:
    explicit View(QWidget *parent = nullptr);

    QGraphicsView *view() const;

public slots:
    void zoomIn();
    void zoomOut();
    void zoomInBy(int level);
    void zoomOutBy(int level);

private slots:
    void resetView();
    void setResetButtonEnabled();
    void setupMatrix();
    //void togglePointerMode();
    //void toggleAntialiasing();

private:
    GraphicsView *graphicsView;
    QLabel *label;
    QLabel *label2;
    QToolButton *selectModeButton;
    QToolButton *resetButton;
    QSlider *zoomSlider;
    //QSlider *rotateSlider;

protected:
    //void dropEvent(QDropEvent *event) override;
    //void dragMoveEvent(QDragMoveEvent *event) override;
    //void dragEnterEvent(QDragEnterEvent *event) override;

};

#endif // VIEW_H
