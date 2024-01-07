#include "qrandom.h"
#include "tile.h"
#include "view.h"

#include <QHBoxLayout>

#include "wfc_gui.h"
#include "./ui_wfc_gui.h"
#include "QFileDialog"
#include "tilepatterncreator.h"

WFC_GUI::WFC_GUI(QWidget *parent)
    : QWidget(parent), ui(new Ui::WFC_GUI),  scene(new QGraphicsScene(this)), creatorScene(new QGraphicsScene(this))
{
    ui->setupUi(this);
    setWindowTitle(tr("Wave Function Collapse GUI"));

    View *wfcView = new View("Tiles view");
    wfcView->view()->setScene(scene);
    wfc_generator = new wfc(wfcView,this);

    QHBoxLayout *wfc_layout = new QHBoxLayout;
    wfc_layout->addWidget(wfcView);
    ui->wfc_frame->setLayout(wfc_layout);

    View *creatorView = new View("Pattern creator view");
    creatorView->view()->setScene(creatorScene);
    tpCreator = new TilePatternCreator(creatorView, this);

    QHBoxLayout *tpc_layout = new QHBoxLayout; //help
    tpc_layout->addWidget(creatorView);
    ui->tpc_frame->setLayout(tpc_layout);

    //connect(ui->generateTilesButton, &QPushButton::clicked, tpCreator, &TilePatternCreator::createTiles);
    //connect(ui->generatePatternsButton, &QPushButton::clicked, tpCreator, &TilePatternCreator::createPatterns);
    connect(ui->exportPatternsButton, &QPushButton::clicked, tpCreator, &TilePatternCreator::exportPatterns);
    connect(ui->tilePixelSizeInput, &QSpinBox::valueChanged, tpCreator, &TilePatternCreator::setTileSize);
    connect(ui->patternSizeInput, &QSpinBox::valueChanged, tpCreator, &TilePatternCreator::setPatternSize);
    connect(tpCreator, &TilePatternCreator::patternsSignal, wfc_generator, &wfc::setPatterns);

    connect(ui->generateGridButton, &QPushButton::clicked, wfc_generator, &wfc::generate);
    connect(ui->generateStepButton, &QPushButton::clicked, wfc_generator, &wfc::generateOneStep);
    connect(ui->gridWidthSelector, &QSpinBox::valueChanged, wfc_generator, &wfc::changeGridWidth);
    connect(ui->gridHeightSelector, &QSpinBox::valueChanged, wfc_generator, &wfc::changeGridHeight);
    connect(ui->clearGridButton, &QPushButton::clicked, wfc_generator, &wfc::clearGrid);
}

WFC_GUI::~WFC_GUI()
{
    delete ui;
}

void WFC_GUI::on_selectFileButton_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,tr("Open Image"), "C:\\Users\\kotru\\Documents\\Studia_7\\Inzynierka", tr("Image files (*.png *.jpg *.bmp)"));
    tpCreator->setImage(filename);
}

void WFC_GUI::populateScene() //test function
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
