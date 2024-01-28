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

    View *wfcView = new View(this);
    wfcView->view()->setScene(scene);
    wfc_generator = new WaveFunctionCollapser(wfcView,this);

    QHBoxLayout *wfc_layout = new QHBoxLayout;
    wfc_layout->addWidget(wfcView);
    ui->wfc_frame->setLayout(wfc_layout);

    View *creatorView = new View(this);
    creatorView->view()->setScene(creatorScene);
    tpCreator = new TilePatternCreator(creatorView, this);

    QHBoxLayout *tpc_layout = new QHBoxLayout; //help
    tpc_layout->addWidget(creatorView);
    ui->tpc_frame->setLayout(tpc_layout);

    p_library = new PatternLibrary(ui->patternSelector, ui->patternDisplay, ui->tileSelector, ui->tileDisplay, this);

    //connect(ui->generateTilesButton, &QPushButton::clicked, tpCreator, &TilePatternCreator::createTiles);
    //connect(ui->generatePatternsButton, &QPushButton::clicked, tpCreator, &TilePatternCreator::createPatterns);
    connect(ui->extractPatternsButton, &QPushButton::clicked, tpCreator, &TilePatternCreator::extractPatterns);
    connect(ui->tilePixelSizeInput, &QSpinBox::valueChanged, tpCreator, &TilePatternCreator::setTileSize);
    connect(ui->patternSizeInput, &QSpinBox::valueChanged, tpCreator, &TilePatternCreator::setPatternSize);
    connect(ui->XWallCheckbox, &QCheckBox::clicked, tpCreator, &TilePatternCreator::toggleXWall);
    connect(ui->YWallCheckbox, &QCheckBox::clicked, tpCreator, &TilePatternCreator::toggleYWall);
    connect(tpCreator, &TilePatternCreator::patternsSignal, wfc_generator, &WaveFunctionCollapser::setPatterns);

    connect(ui->generateGridButton, &QPushButton::clicked, wfc_generator, &WaveFunctionCollapser::generate);
    connect(ui->generateStepButton, &QPushButton::clicked, wfc_generator, &WaveFunctionCollapser::generateOneStep);
    connect(ui->cancelGenerateButton, &QPushButton::clicked, wfc_generator, &WaveFunctionCollapser::cancelGenerate);
    connect(ui->gridWidthSelector, &QSpinBox::valueChanged, wfc_generator, &WaveFunctionCollapser::changeGridWidth);
    connect(ui->gridHeightSelector, &QSpinBox::valueChanged, wfc_generator, &WaveFunctionCollapser::changeGridHeight);
    connect(ui->clearGridButton, &QPushButton::clicked, wfc_generator, &WaveFunctionCollapser::clearGrid);
    connect(ui->seedSpinBox, &QSpinBox::valueChanged, wfc_generator, &WaveFunctionCollapser::setSeed);
    connect(wfc_generator, &WaveFunctionCollapser::setEnabledButtons, ui->generateStepButton, &QPushButton::setEnabled);
    connect(wfc_generator, &WaveFunctionCollapser::setEnabledButtons, ui->clearGridButton, &QPushButton::setEnabled);
    connect(wfc_generator, &WaveFunctionCollapser::setEnabledButtons, ui->generateGridButton, &QPushButton::setEnabled);
    connect(wfc_generator, &WaveFunctionCollapser::setEnabledButtons, ui->cancelGenerateButton, &QPushButton::setDisabled);

    connect(tpCreator, &TilePatternCreator::patternsSignal, p_library, &PatternLibrary::setTilesPatterns);
    connect(ui->libraryTabs, &QTabWidget::currentChanged, p_library, &PatternLibrary::setSelectedTab);
    connect(p_library, &PatternLibrary::displayPatternWeight, ui->weightDoubleSpinBox, &QDoubleSpinBox::setValue);
    connect(p_library, &PatternLibrary::displayPatternWeight, ui->weightDoubleSpinBox_2, &QDoubleSpinBox::setValue);
    connect(p_library, &PatternLibrary::displayPatternEnabled, ui->enabledCheckbox, &QCheckBox::setChecked);
    connect(p_library, &PatternLibrary::displayPatternEnabled, ui->enabledCheckbox_2, &QCheckBox::setChecked);
    connect(ui->enabledCheckbox, &QCheckBox::clicked, p_library, &PatternLibrary::setElementEnabled);
    connect(ui->enabledCheckbox_2, &QCheckBox::clicked, p_library, &PatternLibrary::setElementEnabled);
    connect(ui->weightDoubleSpinBox, &QDoubleSpinBox::valueChanged, p_library, &PatternLibrary::setElementWeight);
    connect(ui->weightDoubleSpinBox_2, &QDoubleSpinBox::valueChanged, p_library, &PatternLibrary::setElementWeight);
    connect(ui->resetLibraryButton, &QPushButton::clicked, p_library, &PatternLibrary::resetElement);
    connect(ui->resetLibraryButton_2, &QPushButton::clicked, p_library, &PatternLibrary::resetElement);
    connect(ui->librarySendPatternsButton, &QPushButton::clicked, p_library, &PatternLibrary::sendOutTilesPatterns);
    connect(ui->librarySendPatternsButton_2, &QPushButton::clicked, p_library, &PatternLibrary::sendOutTilesPatterns);
    connect(p_library, &PatternLibrary::sendTilesPatterns, wfc_generator, &WaveFunctionCollapser::setPatterns);
    connect(p_library, &PatternLibrary::setUIEnabled, ui->libraryTabs, &QTabWidget::setEnabled);

    connect(ui->saveImageButton, &QPushButton::clicked, this, &WFC_GUI::saveGeneratedImage);
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

void WFC_GUI::saveGeneratedImage(){
    QString filename = QFileDialog::getSaveFileName(this,tr("Save Result"), "C:\\Users\\kotru\\Documents\\Studia_7\\Inzynierka\\wfc_result.png", tr("Image files (*.png *.jpg *.bmp)"));
    wfc_generator->exportImage(filename);
}
