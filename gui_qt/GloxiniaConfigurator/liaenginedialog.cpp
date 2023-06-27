#include "liaenginedialog.h"
#include "ui_liaenginedialog.h"

LIAEngineDialog::LIAEngineDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LIAEngineDialog)
{
    ui->setupUi(this);
}

LIAEngineDialog::~LIAEngineDialog()
{
    delete ui;
}


void LIAEngineDialog::updateUISettings(const GLIAEngine* engine)
{

    switch(engine->getFrequencyHigh()){
    case GLIAEngine::FREQ_10HZ:
        ui->freqHighBox->setCurrentIndex(0);
        break;
    case GLIAEngine::FREQ_20HZ:
        ui->freqHighBox->setCurrentIndex(1);
        break;
    case GLIAEngine::FREQ_50HZ:
        ui->freqHighBox->setCurrentIndex(2);
        break;
    case GLIAEngine::FREQ_100HZ:
        ui->freqHighBox->setCurrentIndex(3);
        break;
    case GLIAEngine::FREQ_200HZ:
        ui->freqHighBox->setCurrentIndex(4);
        break;
    case GLIAEngine::FREQ_500HZ:
        ui->freqHighBox->setCurrentIndex(5);
        break;
    case GLIAEngine::FREQ_1KHZ:
        ui->freqHighBox->setCurrentIndex(6);
        break;
    case GLIAEngine::FREQ_2KHZ:
        ui->freqHighBox->setCurrentIndex(7);
        break;
    case GLIAEngine::FREQ_5KHZ:
        ui->freqHighBox->setCurrentIndex(8);
        break;
    case GLIAEngine::FREQ_10KHZ:
        ui->freqHighBox->setCurrentIndex(9);
        break;
    case GLIAEngine::FREQ_20KHZ:
        ui->freqHighBox->setCurrentIndex(10);
        break;
    case GLIAEngine::FREQ_50KHZ:
        ui->freqHighBox->setCurrentIndex(11);
        break;
    default:
        ui->freqHighBox->setCurrentIndex(0);
        break;
    }

    switch(engine->getFrequencyLow()){
    case GLIAEngine::FREQ_10HZ:
        ui->freqLowBox->setCurrentIndex(0);
        break;
    case GLIAEngine::FREQ_20HZ:
        ui->freqLowBox->setCurrentIndex(1);
        break;
    case GLIAEngine::FREQ_50HZ:
        ui->freqLowBox->setCurrentIndex(2);
        break;
    case GLIAEngine::FREQ_100HZ:
        ui->freqLowBox->setCurrentIndex(3);
        break;
    case GLIAEngine::FREQ_200HZ:
        ui->freqLowBox->setCurrentIndex(4);
        break;
    case GLIAEngine::FREQ_500HZ:
        ui->freqLowBox->setCurrentIndex(5);
        break;
    case GLIAEngine::FREQ_1KHZ:
        ui->freqLowBox->setCurrentIndex(6);
        break;
    case GLIAEngine::FREQ_2KHZ:
        ui->freqHighBox->setCurrentIndex(7);
        break;
    case GLIAEngine::FREQ_5KHZ:
        ui->freqLowBox->setCurrentIndex(8);
        break;
    case GLIAEngine::FREQ_10KHZ:
        ui->freqLowBox->setCurrentIndex(9);
        break;
    case GLIAEngine::FREQ_20KHZ:
        ui->freqLowBox->setCurrentIndex(10);
        break;
    case GLIAEngine::FREQ_50KHZ:
        ui->freqLowBox->setCurrentIndex(11);
        break;
    default:
        ui->freqLowBox->setCurrentIndex(0);
        break;
    }


    switch(engine->getMode()){
        case GLIAEngine::GLIAEngineMode::LIA_5KHZ:
            ui->freqLowBox->setCurrentIndex(0);
            break;
        case GLIAEngine::GLIAEngineMode::LIA_10KHZ:
            ui->freqLowBox->setCurrentIndex(1);
            break;
        case GLIAEngine::GLIAEngineMode::LIA_25KHZ:
            ui->freqLowBox->setCurrentIndex(2);
            break;
        case GLIAEngine::GLIAEngineMode::LIA_50KHZ:
            ui->freqLowBox->setCurrentIndex(3);
            break;
        case GLIAEngine::GLIAEngineMode::LIA_FS:
            ui->freqLowBox->setCurrentIndex(4);
            break;
        default:
            ui->freqLowBox->setCurrentIndex(0);
            break;
    }
}

void LIAEngineDialog::apply(GLIAEngine* engine)
{
    GLIAEngine::GLIAEngineFrequency freqH, freqL, freq;
    GLIAEngine::GLIAEngineMode mode;

    switch(ui->freqHighBox->currentIndex()){
    case 0:
        freqH = GLIAEngine::FREQ_10HZ;
        break;
    case 1:
        freqH = GLIAEngine::FREQ_20HZ;
        break;
    case 2:
        freqH = GLIAEngine::FREQ_50HZ;
        break;
    case 3:
        freqH = GLIAEngine::FREQ_100HZ;
        break;
    case 4:
        freqH = GLIAEngine::FREQ_200HZ;
        break;
    case 5:
        freqH = GLIAEngine::FREQ_500HZ;
        break;
    case 6:
        freqH = GLIAEngine::FREQ_1KHZ;
        break;
    case 7:
        freqH = GLIAEngine::FREQ_2KHZ;
        break;
    case 8:
        freqH = GLIAEngine::FREQ_5KHZ;
        break;
    case 9:
        freqH = GLIAEngine::FREQ_20KHZ;
        break;
    case 10:
        freqH = GLIAEngine::FREQ_10KHZ;
        break;
    case 11:
        freqH = GLIAEngine::FREQ_50KHZ;
        break;
    default:
        freqH = GLIAEngine::FREQ_50KHZ;
        break;
    }

    switch(ui->freqLowBox->currentIndex()){
    case 0:
        freqL = GLIAEngine::FREQ_10HZ;
        break;
    case 1:
        freqL = GLIAEngine::FREQ_20HZ;
        break;
    case 2:
        freqL = GLIAEngine::FREQ_50HZ;
        break;
    case 3:
        freqL = GLIAEngine::FREQ_100HZ;
        break;
    case 4:
        freqL = GLIAEngine::FREQ_200HZ;
        break;
    case 5:
        freqL = GLIAEngine::FREQ_500HZ;
        break;
    case 6:
        freqL = GLIAEngine::FREQ_1KHZ;
        break;
    case 7:
        freqL = GLIAEngine::FREQ_2KHZ;
        break;
    case 8:
        freqL = GLIAEngine::FREQ_5KHZ;
        break;
    case 9:
        freqL = GLIAEngine::FREQ_10KHZ;
        break;
    case 10:
        freqL = GLIAEngine::FREQ_20KHZ;
        break;
    case 11:
        freqL = GLIAEngine::FREQ_50KHZ;
        break;
    default:
        freqL = GLIAEngine::FREQ_50KHZ;
        break;
    }

    // check if freqL is smaller than freqH, swap otherwise
    if(freqL > freqH)
    {
        freq = freqL;
        freqL = freqH;
        freqH = freq;
    }

    switch(ui->modeBox->currentIndex())
    {
    case 0:
        mode = GLIAEngine::GLIAEngineMode::LIA_5KHZ;
        break;
    case 1:
        mode = GLIAEngine::GLIAEngineMode::LIA_10KHZ;
        break;
    case 2:
        mode = GLIAEngine::GLIAEngineMode::LIA_25KHZ;
        break;
    case 3:
        mode = GLIAEngine::GLIAEngineMode::LIA_50KHZ;
        break;
    case 4:
        mode = GLIAEngine::GLIAEngineMode::LIA_FS;
        break;
    default:
        mode = GLIAEngine::GLIAEngineMode::LIA_5KHZ;
        break;
    }

    engine->setFrequencyHigh(freqH);
    engine->setFrequencyHigh(freqL);
    engine->setMode(mode);
}
