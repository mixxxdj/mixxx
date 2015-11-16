#include <QtDebug>

#include "dlgprefcrossfader.h"
#include "engine/enginefilterbessel4.h"
#include "controlobject.h"
#include "engine/enginexfader.h"

#define kConfigKey "[Mixer Profile]"

namespace {
const double kTransformMax = 1000.0;
} // anonymous namespace

DlgPrefCrossfader::DlgPrefCrossfader(
        QWidget* parent, ConfigObject<ConfigValue>* config)
        : DlgPreferencePage(parent),
          m_config(config),
          m_pxfScene(NULL),
          m_xFaderMode(MIXXX_XFADER_ADDITIVE),
          m_transform(1.0),
          m_cal(0.0),
          m_mode(kConfigKey, "xFaderMode"),
          m_curve(kConfigKey, "xFaderCurve"),
          m_calibration(kConfigKey, "xFaderCalibration"),
          m_reverse(kConfigKey, "xFaderReverse"),
          m_crossfader("[Master]", "crossfader"),
          m_xFaderReverse(false) {
    setupUi(this);

    QButtonGroup crossfaderModes;
    crossfaderModes.addButton(radioButtonAdditive);
    crossfaderModes.addButton(radioButtonConstantPower);

    loadSettings();

    connect(SliderXFader, SIGNAL(valueChanged(int)), this,
            SLOT(slotUpdateXFader()));
    connect(SliderXFader, SIGNAL(sliderMoved(int)), this,
            SLOT(slotUpdateXFader()));
    connect(SliderXFader, SIGNAL(sliderReleased()), this,
            SLOT(slotUpdateXFader()));
    connect(SliderXFader, SIGNAL(sliderReleased()), this,
            SLOT(slotApply()));

    // Update the crossfader curve graph and other settings when the
    // crossfader mode is changed.
    connect(radioButtonAdditive, SIGNAL(clicked(bool)), this,
            SLOT(slotUpdate()));
    connect(radioButtonConstantPower, SIGNAL(clicked(bool)), this,
            SLOT(slotUpdate()));
}

DlgPrefCrossfader::~DlgPrefCrossfader() {
    delete m_pxfScene;
}

// Loads the config keys and sets the widgets in the dialog to match
void DlgPrefCrossfader::loadSettings() {
    // Range xFaderCurve 1 .. kXFaderSteepnessCoeff
    m_transform = m_config->getValueString(
            ConfigKey(kConfigKey, "xFaderCurve")).toDouble();

    // Range SliderXFader 0 .. 100
    double sliderVal = ((kTransformMax / m_transform) - 1) /
            (kTransformMax - 1) * SliderXFader->maximum();
    SliderXFader->setValue(SliderXFader->maximum() - (int)sliderVal);

    m_xFaderMode =
            m_config->getValueString(ConfigKey(kConfigKey, "xFaderMode")).toInt();

    if (m_xFaderMode == MIXXX_XFADER_CONSTPWR) {
        radioButtonConstantPower->setChecked(true);
    } else {
        radioButtonAdditive->setChecked(true);
    }

    m_xFaderReverse = m_config->getValueString(
            ConfigKey(kConfigKey, "xFaderReverse")).toInt() == 1;
    checkBoxReverse->setChecked(m_xFaderReverse);

    slotUpdateXFader();
    slotApply();
}

// Set the default values for all the widgets
void DlgPrefCrossfader::slotResetToDefaults() {
    SliderXFader->setValue(0);
    m_xFaderMode = MIXXX_XFADER_ADDITIVE;
    radioButtonAdditive->setChecked(true);
    checkBoxReverse->setChecked(false);
    slotUpdate();
    slotApply();
}

// Apply and save any changes made in the dialog
void DlgPrefCrossfader::slotApply() {
    m_mode.set(m_xFaderMode);
    m_curve.set(m_transform);
    m_calibration.set(m_cal);
    if (checkBoxReverse->isChecked() != m_xFaderReverse) {
        m_reverse.set(checkBoxReverse->isChecked());
        double position = m_crossfader.get();
        m_crossfader.set(0.0 - position);
        m_xFaderReverse = checkBoxReverse->isChecked();
    }
    slotUpdateXFader();
}

// Update the dialog when the crossfader mode is changed
void DlgPrefCrossfader::slotUpdate() {
    if (radioButtonAdditive->isChecked()) {
        m_xFaderMode = MIXXX_XFADER_ADDITIVE;
    }
    if (radioButtonConstantPower->isChecked()) {
        m_xFaderMode = MIXXX_XFADER_CONSTPWR;
    }

    slotUpdateXFader();
}

// Draw the crossfader curve graph. Only needs to get drawn when a change
// has been made.
void DlgPrefCrossfader::drawXfaderDisplay()
{
    const int kGrindXLines = 4;
    const int kGrindYLines = 6;

    int sizeX = graphicsViewXfader->width();
    int sizeY = graphicsViewXfader->height();

    // Initialize Scene
    if (m_pxfScene) {
        delete m_pxfScene;
        m_pxfScene = NULL;
    }
    m_pxfScene = new QGraphicsScene();
    m_pxfScene->setSceneRect(0,0,sizeX, sizeY);
    m_pxfScene->setBackgroundBrush(Qt::black);

    // Initialize QPens
    QPen gridPen(Qt::green);
    QPen graphLinePen(Qt::white);

    // draw grid
    for (int i = 1; i < kGrindXLines; i++) {
        m_pxfScene->addLine(
                QLineF(0, i * (sizeY / kGrindXLines), sizeX,
                        i * (sizeY / kGrindXLines)), gridPen);
    }
    for (int i = 1; i < kGrindYLines; i++) {
        m_pxfScene->addLine(
                QLineF(i * (sizeX / kGrindYLines), 0,
                        i * (sizeX / kGrindYLines), sizeY), gridPen);
    }

    // Draw graph lines
    QPointF pointTotal, point1, point2;
    QPointF pointTotalPrev, point1Prev, point2Prev;
    int pointCount = sizeX - 4;
    // reduced by 2 x 1 for border + 2 x 1 for inner distance to border
    double xfadeStep = 2. / (pointCount - 1);
    for (int i = 0; i < pointCount; i++) {
        double gain1, gain2;
        EngineXfader::getXfadeGains((-1. + (xfadeStep * i)),
                                    m_transform, m_cal,
                                    (m_xFaderMode == MIXXX_XFADER_CONSTPWR),
                                    checkBoxReverse->isChecked(),
                                    &gain1, &gain2);

        double sum = gain1 + gain2;
        // scale for graph
        gain1 *= 0.80;
        gain2 *= 0.80;
        sum *= 0.80;

        // draw it
        pointTotal = QPointF(i + 1, (1. - sum) * (sizeY) - 3);
        point1 = QPointF(i + 1, (1. - gain1) * (sizeY) - 3);
        point2 = QPointF(i + 1, (1. - gain2) * (sizeY) - 3);

        if(i == 0) {
            pointTotalPrev = pointTotal;
            point1Prev = point1;
            point2Prev = point2;
        }

        if(pointTotal != point1)
            m_pxfScene->addLine(QLineF(point1, point1Prev), graphLinePen);
        if(pointTotal != point2)
            m_pxfScene->addLine(QLineF(point2, point2Prev), graphLinePen);
        m_pxfScene->addLine(QLineF(pointTotal, pointTotalPrev), QPen(Qt::red));

        // Save old values
        pointTotalPrev = pointTotal;
        point1Prev = point1;
        point2Prev = point2;
    }

    graphicsViewXfader->setScene(m_pxfScene);
    graphicsViewXfader->show();
}

// Update and save the crossfader's parameters from the dialog's widgets.
void DlgPrefCrossfader::slotUpdateXFader() {
    m_transform = kTransformMax /
            (((double)(SliderXFader->maximum() - SliderXFader->value()) /
                    SliderXFader->maximum() *
                    (kTransformMax - 1)) + 1);
    m_cal = EngineXfader::getPowerCalibration(m_transform);
    m_config->set(ConfigKey(kConfigKey, "xFaderMode"), ConfigValue(m_xFaderMode));
    m_config->set(ConfigKey(kConfigKey, "xFaderCurve"),
            ConfigValue(QString::number(m_transform)));
    m_config->set(ConfigKey(kConfigKey, "xFaderReverse"),
                ConfigValue(checkBoxReverse->isChecked() ? 1 : 0));

    drawXfaderDisplay();
}
