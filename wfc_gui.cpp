#include "qrandom.h"
#include "tile.h"
#include "view.h"

#include <QHBoxLayout>

#include "wfc_gui.h"
#include "./ui_wfc_gui.h"
#include "QFileDialog"

WFC_GUI::WFC_GUI(QWidget *parent)
    : QWidget(parent), ui(new Ui::WFC_GUI),  scene(new QGraphicsScene(this))
{
    ui->setupUi(this);

    populateScene();

    View *view = new View("Tiles view");
    view->view()->setScene(scene);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(view);
    ui->frame->setLayout(layout);

    setWindowTitle(tr("Wave Function Collapse GUI"));

    creatorScene = new QGraphicsScene(this); //MOVE
    View *creatorView = new View("Pattern creator view");

    QHBoxLayout *layout2 = new QHBoxLayout; //help
    layout2->addWidget(creatorView);
    ui->frame_2->setLayout(layout2);
    creatorView->view()->setScene(creatorScene);

    tpCreator = new TilePatternCreator(creatorView, this);

    connect(ui->generateTilesButton, SIGNAL(clicked()), tpCreator, SLOT(createTiles()));
    connect(ui->generatePatternsButton, SIGNAL(clicked()), tpCreator, SLOT(createPatterns()));
}

WFC_GUI::~WFC_GUI()
{
    delete ui;
}

void WFC_GUI::on_selectFileButton_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,tr("Open Image"), "/.", tr("Image files (*.png *.jpg *.bmp)"));
    tpCreator->setImage(filename);
}

void WFC_GUI::populateScene()
{
    auto imageset = new QList<Tile>();
    //imageset->append(new QImage(":/fileprint.png"));
    //imageset->append(new QImage(":/rotateleft.png"));
    imageset->append(Tile(QImage(":/fileprint.png"),1,0));
    imageset->append(Tile(QImage(":/rotateleft.png"),1,1));
    auto generator = new QRandomGenerator(2);
    // Populate scene
    int xx = 0;
    for (int i = -11000; i < 11000; i += 110) {
        ++xx;
        int yy = 0;
        for (int j = -7000; j < 7000; j += 70) {
            ++yy;
            //qreal x = (i + 11000) / 22000.0;
            //qreal y = (j + 7000) / 14000.0;

            //QColor color(image.pixel(int(image.width() * x), int(image.height() * y)));
            QGraphicsItem *item = new TileGraphicsItem(imageset->at(qRound(generator->bounded(1.0))));
            item->setPos(QPointF(i, j));
            scene->addItem(item);
        }
    }
}
