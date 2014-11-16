/***************************************************************************
                          dlgprefcrossfader.cpp  -  description
                             -------------------
    begin                : Thu Jun 7 2007
    copyright            : (C) 2007 by John Sully
    email                : jsully@scs.ryerson.ca
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtDebug>

#include "dlgprefcrossfader.h"
#include "engine/enginefilterbessel4.h"
#include "controlobject.h"
#include "engine/enginexfader.h"

#define kConfigKey "[Mixer Profile]"

DlgPrefCrossfader::DlgPrefCrossfader(QWidget * parent, ConfigObject<ConfigValue> * _config)
        : DlgPreferencePage(parent),
          config(_config),
          m_pxfScene(NULL),
          m_xFaderMode(MIXXX_XFADER_ADDITIVE),
          m_transform(0.0),
          m_cal(0.0),
          m_COTMode(kConfigKey, "xFaderMode"),
          m_COTCurve(kConfigKey, "xFaderCurve"),
          m_COTCalibration(kConfigKey, "xFaderCalibration"),
          m_COTReverse(kConfigKey, "xFaderReverse"),
          m_COTCrossfader("[Master]", "crossfader"),
          m_xFaderReverse(false) {
    setupUi(this);

    QButtonGroup crossfaderModes;
    crossfaderModes.addButton(radioButtonAdditive);
    crossfaderModes.addButton(radioButtonConstantPower);

    loadSettings();

    connect(SliderXFader, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateXFader()));
    connect(SliderXFader, SIGNAL(sliderMoved(int)), this, SLOT(slotUpdateXFader()));
    connect(SliderXFader, SIGNAL(sliderReleased()), this, SLOT(slotUpdateXFader()));
    connect(SliderXFader, SIGNAL(sliderReleased()), this, SLOT(slotApply()));

    //Update the crossfader curve graph and other setings when the crossfader mode is changed.
    connect(radioButtonAdditive, SIGNAL(clicked(bool)), this, SLOT(slotUpdate()));
    connect(radioButtonConstantPower, SIGNAL(clicked(bool)), this, SLOT(slotUpdate()));
}

DlgPrefCrossfader::~DlgPrefCrossfader() {
    delete m_pxfScene;
}

/** Loads the config keys and sets the widgets in the dialog to match */
void DlgPrefCrossfader::loadSettings() {
    m_transform = 1. + ((double) SliderXFader->value() / SliderXFader->maximum());
    double sliderTransform = config->getValueString(ConfigKey(kConfigKey, "xFaderCurve")).toDouble();
    double sliderVal = SliderXFader->maximum() / MIXXX_XFADER_STEEPNESS_COEFF * (sliderTransform - 1.);
    SliderXFader->setValue((int)sliderVal);

    m_xFaderMode = config->getValueString(ConfigKey(kConfigKey, "xFaderMode")).toInt();

    if (m_xFaderMode == MIXXX_XFADER_CONSTPWR) {
        radioButtonConstantPower->setChecked(true);
        //SliderXFader->setEnabled(true);
    } else {
        radioButtonAdditive->setChecked(true);
        //SliderXFader->setEnabled(false);
    }

    m_xFaderReverse = config->getValueString(ConfigKey(kConfigKey, "xFaderReverse")).toInt() == 1;
    checkBoxReverse->setChecked(m_xFaderReverse);

    slotUpdateXFader();
    slotApply();
}

/** Set the default values for all the widgets */
void DlgPrefCrossfader::slotResetToDefaults() {
    SliderXFader->setValue(0);
    m_xFaderMode = MIXXX_XFADER_ADDITIVE;
    radioButtonAdditive->setChecked(true);
    SliderXFader->setValue(SliderXFader->minimum());
    checkBoxReverse->setChecked(false);
    slotUpdate();
    slotApply();
}

/** Apply and save any changes made in the dialog */
void DlgPrefCrossfader::slotApply() {
    m_COTMode.slotSet(m_xFaderMode);
    m_COTCurve.slotSet(m_transform);
    m_COTCalibration.slotSet(m_cal);
    if (checkBoxReverse->isChecked() != m_xFaderReverse) {
        m_COTReverse.slotSet(checkBoxReverse->isChecked());
        double position = m_COTCrossfader.get();
        m_COTCrossfader.slotSet(0.0 - position);
        m_xFaderReverse = checkBoxReverse->isChecked();
    }
    slotUpdateXFader();
}

/** Update the dialog when the crossfader mode is changed */
void DlgPrefCrossfader::slotUpdate() {
    if (radioButtonAdditive->isChecked()) {
        m_xFaderMode = MIXXX_XFADER_ADDITIVE;
//         SliderXFader->setValue(SliderXFader->minimum());
    }
    if (radioButtonConstantPower->isChecked()) {
        m_xFaderMode = MIXXX_XFADER_CONSTPWR;
        double sliderTransform = config->getValueString(ConfigKey(kConfigKey, "xFaderCurve")).toDouble();
        double sliderVal = SliderXFader->maximum() / MIXXX_XFADER_STEEPNESS_COEFF * (sliderTransform - 1.);
        SliderXFader->setValue((int)sliderVal);
    }

    slotUpdateXFader();
}

/** Draw the crossfader curve graph. Only needs to get drawn when a change has been made.*/
void DlgPrefCrossfader::drawXfaderDisplay()
{
#define GRID_X_LINES 4
#define GRID_Y_LINES 6

    int sizeX = graphicsViewXfader->width();
    int sizeY = graphicsViewXfader->height();

    //Initialize Scene
    if (m_pxfScene) {
        delete m_pxfScene;
        m_pxfScene = NULL;
    }
    m_pxfScene = new QGraphicsScene();
    m_pxfScene->setSceneRect(0,0,sizeX, sizeY);
    m_pxfScene->setBackgroundBrush(Qt::black);

    //Initialize QPens
    QPen gridPen(Qt::green);
    QPen graphLinePen(Qt::white);

    //draw grid
    for (int i=1; i < GRID_X_LINES; i++) {
        m_pxfScene->addLine(QLineF(0, i *(sizeY/GRID_X_LINES),sizeX,i *(sizeY/GRID_X_LINES)), gridPen);
    }
    for (int i=1; i < GRID_Y_LINES; i++) {
        m_pxfScene->addLine(QLineF( i * (sizeX/GRID_Y_LINES), 0, i * (sizeX/GRID_Y_LINES), sizeY), gridPen);
    }

    // Draw graph lines
    QPoint pointTotal, point1, point2;
    QPoint pointTotalPrev, point1Prev, point2Prev;
    for (int i = 0; i < sizeX; i++) {
        double xfadeStep = 2. / (double)sizeX;

        double gain1, gain2;
        EngineXfader::getXfadeGains((-1. + (xfadeStep * (double) i)),
                                    m_transform, m_cal,
                                    (m_xFaderMode == MIXXX_XFADER_CONSTPWR),
                                    checkBoxReverse->isChecked(),
                                    &gain1, &gain2);

        double sum = gain1 + gain2;
        //scale for graph
        gain1 *= 0.80;
        gain2 *= 0.80;
        sum *= 0.80;

        //draw it
        pointTotalPrev = pointTotal;
        point1Prev = point1;
        point2Prev = point2;
        pointTotal = QPoint(i - 2, (int)((1. - sum) * (sizeY)));
        point1 = QPoint(i - 2, (int)((1. - gain1) * (sizeY)));
        point2 = QPoint(i - 2, (int)((1. - gain2) * (sizeY)));
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
    }

    graphicsViewXfader->setScene(m_pxfScene);
    graphicsViewXfader->show();
}

/** Update and save the crossfader's parameters from the dialog's widgets. **/
void DlgPrefCrossfader::slotUpdateXFader() {
    m_transform = 1. + ((double) SliderXFader->value() / SliderXFader->maximum() * MIXXX_XFADER_STEEPNESS_COEFF);

    m_cal = EngineXfader::getCalibration(m_transform);
    QString QS_transform = QString::number(m_transform);
    config->set(ConfigKey(kConfigKey, "xFaderMode"), ConfigValue(m_xFaderMode));
    config->set(ConfigKey(kConfigKey, "xFaderCurve"), ConfigValue(QS_transform));
    //config->set(ConfigKey(CONFIG_KEY, "xFaderCalibration"), ConfigValue(m_cal)); //FIXME: m_cal is a double - be forewarned
    config->set(ConfigKey(kConfigKey, "xFaderReverse"),
                ConfigValue(checkBoxReverse->isChecked() ? 1 : 0));

    drawXfaderDisplay();
}
