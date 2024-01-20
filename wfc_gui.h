#ifndef WFC_GUI_H
#define WFC_GUI_H

#include <QWidget>
#include "tile.h"
#include "tilepatterncreator.h"
#include "wavefunctioncollapser.h"
#include "patternlibrary.h"

QT_BEGIN_NAMESPACE
namespace Ui { class WFC_GUI; }
class QGraphicsScene;
class QSplitter;
QT_END_NAMESPACE

class WFC_GUI : public QWidget
{
    Q_OBJECT

public:
    WFC_GUI(QWidget *parent = nullptr);
    ~WFC_GUI();

signals:


private slots:
    void on_selectFileButton_clicked();

private:
    Ui::WFC_GUI *ui;

    void setupMatrix();
    void populateScene();

    QGraphicsScene *scene;
    QGraphicsScene *creatorScene;

    QList<Tile> tileLibrary;
    QList<Pattern> patternLibrary;

    QList<short> tileMap;

    TilePatternCreator* tpCreator;
    WaveFunctionCollapser* wfc_generator;
    PatternLibrary* p_library;
};
#endif // WFC_GUI_H
