#include <QButtonGroup>
#include <QtDebug>

#include "preferences/dialog/dlgprefcrossfader.h"
#include "control/controlobject.h"
#include "engine/enginexfader.h"
#include "util/math.h"
#include "util/rescaler.h"

DlgPrefCrossfader::DlgPrefCrossfader(
        QWidget* parent, UserSettingsPointer config)
        : DlgPreferencePage(parent),
          m_config(config),
          m_pxfScene(NULL),
          m_xFaderMode(MIXXX_XFADER_ADDITIVE),
          m_transform(EngineXfader::kTransformDefault),
          m_cal(0.0),
          m_mode(EngineXfader::kXfaderConfigKey, "xFaderMode"),
          m_curve(EngineXfader::kXfaderConfigKey, "xFaderCurve"),
          m_calibration(EngineXfader::kXfaderConfigKey, "xFaderCalibration"),
          m_reverse(EngineXfader::kXfaderConfigKey, "xFaderReverse"),
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
    // Range xFaderCurve EngineXfader::kTransformMin .. EngineXfader::kTransformMax
    m_transform = m_config->getValue(
            ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderCurve"),
            EngineXfader::kTransformDefault);

    // Range SliderXFader 0 .. 100
    double sliderVal = RescalerUtils::oneByXToLinear(
            m_transform - EngineXfader::kTransformMin + 1,
            EngineXfader::kTransformMax - EngineXfader::kTransformMin + 1,
            SliderXFader->minimum(),
            SliderXFader->maximum());
    SliderXFader->setValue(static_cast<int>(sliderVal + 0.5));

    m_xFaderMode =
            m_config->getValueString(ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderMode")).toInt();

    if (m_xFaderMode == MIXXX_XFADER_CONSTPWR) {
        radioButtonConstantPower->setChecked(true);
    } else {
        radioButtonAdditive->setChecked(true);
    }

    m_xFaderReverse = m_config->getValueString(
            ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderReverse")).toInt() == 1;
    checkBoxReverse->setChecked(m_xFaderReverse);

    slotUpdateXFader();
    slotApply();
}

// Set the default values for all the widgets
void DlgPrefCrossfader::slotResetToDefaults() {
    double sliderVal = RescalerUtils::oneByXToLinear(
            EngineXfader::kTransformDefault - EngineXfader::kTransformMin + 1,
            EngineXfader::kTransformMax - EngineXfader::kTransformMin + 1,
            SliderXFader->minimum(),
            SliderXFader->maximum());
    SliderXFader->setValue(sliderVal);

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
                                    m_xFaderMode,
                                    checkBoxReverse->isChecked(),
                                    &gain1, &gain2);

        double gain = sqrt(gain1 * gain1 + gain2 * gain2);
        // scale for graph
        gain1 *= 0.71;
        gain2 *= 0.71;
        gain *= 0.71;

        // draw it
        pointTotal = QPointF(i + 1, (1. - gain) * (sizeY) - 3);
        point1 = QPointF(i + 1, (1. - gain1) * (sizeY) - 3);
        point2 = QPointF(i + 1, (1. - gain2) * (sizeY) - 3);

        if (i > 0) {
            m_pxfScene->addLine(QLineF(pointTotal, pointTotalPrev), QPen(Qt::red));
            m_pxfScene->addLine(QLineF(point1, point1Prev), graphLinePen);
            m_pxfScene->addLine(QLineF(point2, point2Prev), graphLinePen);
        }

        // Save old values
        pointTotalPrev = pointTotal;
        point1Prev = point1;
        point2Prev = point2;
    }

    graphicsViewXfader->setScene(m_pxfScene);
    graphicsViewXfader->show();
    graphicsViewXfader->repaint();
}

// Update and save the crossfader's parameters from the dialog's widgets.
void DlgPrefCrossfader::slotUpdateXFader() {
    // m_transform is in the range of 1 to 1000 while 50 % slider results
    // to ~2, which represents a medium rounded fader curve.
    m_transform = RescalerUtils::linearToOneByX(
            SliderXFader->value(),
            SliderXFader->minimum(),
            SliderXFader->maximum(),
            EngineXfader::kTransformMax) - 1 + EngineXfader::kTransformMin;
    m_cal = EngineXfader::getPowerCalibration(m_transform);
    m_config->set(ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderMode"),
            ConfigValue(m_xFaderMode));
    m_config->set(ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderCurve"),
            ConfigValue(QString::number(m_transform)));
    m_config->set(ConfigKey(EngineXfader::kXfaderConfigKey, "xFaderReverse"),
                ConfigValue(checkBoxReverse->isChecked() ? 1 : 0));

    drawXfaderDisplay();
}
