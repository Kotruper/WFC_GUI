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
    wfc_generator = new WaveFunctionCollapser(wfcView,this);

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
    connect(ui->extractPatternsButton, &QPushButton::clicked, tpCreator, &TilePatternCreator::extractPatterns);
    connect(ui->tilePixelSizeInput, &QSpinBox::valueChanged, tpCreator, &TilePatternCreator::setTileSize);
    connect(ui->patternSizeInput, &QSpinBox::valueChanged, tpCreator, &TilePatternCreator::setPatternSize);
    connect(tpCreator, &TilePatternCreator::patternsSignal, wfc_generator, &WaveFunctionCollapser::setPatterns);

    connect(ui->generateGridButton, &QPushButton::clicked, wfc_generator, &WaveFunctionCollapser::generate);
    connect(ui->generateStepButton, &QPushButton::clicked, wfc_generator, &WaveFunctionCollapser::generateOneStep);
    connect(ui->gridWidthSelector, &QSpinBox::valueChanged, wfc_generator, &WaveFunctionCollapser::changeGridWidth);
    connect(ui->gridHeightSelector, &QSpinBox::valueChanged, wfc_generator, &WaveFunctionCollapser::changeGridHeight);
    connect(ui->clearGridButton, &QPushButton::clicked, wfc_generator, &WaveFunctionCollapser::clearGrid);

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
