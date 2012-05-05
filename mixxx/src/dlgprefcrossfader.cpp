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

#include "dlgprefcrossfader.h"
#include "engine/enginefilteriir.h"
#include "controlobject.h"
#include "engine/enginexfader.h"
#include <qlineedit.h>
#include <qwidget.h>
#include <qslider.h>
#include <qlabel.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qgraphicsscene.h>
#include <QtDebug>

#include <assert.h>

#define CONFIG_KEY "[Mixer Profile]"

DlgPrefCrossfader::DlgPrefCrossfader(QWidget * parent, ConfigObject<ConfigValue> * _config)
        : QWidget(parent)
        , m_COTCurve(ControlObject::getControl(ConfigKey(CONFIG_KEY, "xFaderCurve")))
        , m_COTCalibration(ControlObject::getControl(ConfigKey(CONFIG_KEY, "xFaderCalibration")))
{
    config = _config;
    m_pxfScene = NULL;

    m_transform = 0;
    m_cal = 0;

    setupUi(this);

    connect(SliderXFader,         SIGNAL(valueChanged(int)), this, SLOT(slotUpdateXFader()));
    connect(SliderXFader,         SIGNAL(sliderMoved(int)), this,  SLOT(slotUpdateXFader()));
    connect(SliderXFader,         SIGNAL(sliderReleased()), this,  SLOT(slotUpdateXFader()));
    connect(SliderXFader,         SIGNAL(sliderReleased()), this,  SLOT(slotApply()));

    connect(PushButtonReset,	  SIGNAL(clicked(bool)), this,	SLOT(setDefaults()));

    //Update the crossfader curve graph and other setings when the crossfader mode is changed.
    connect(radioButtonSlowFade,        SIGNAL(clicked(bool)), this, SLOT(slotUpdate()));
    connect(radioButtonFastCut,         SIGNAL(clicked(bool)), this, SLOT(slotUpdate()));
    connect(radioButtonConstantPower,   SIGNAL(clicked(bool)), this, SLOT(slotUpdate()));

    QButtonGroup crossfaderModes;
    crossfaderModes.addButton(radioButtonSlowFade);
    crossfaderModes.addButton(radioButtonFastCut);
    crossfaderModes.addButton(radioButtonConstantPower);

	loadSettings();

}

DlgPrefCrossfader::~DlgPrefCrossfader()
{
   delete m_pxfScene;
}

/** Loads the config keys and sets the widgets in the dialog to match */
void DlgPrefCrossfader::loadSettings()
{
	m_transform = 1. + ((double) SliderXFader->value() / SliderXFader->maximum());
	double sliderTransform = config->getValueString(ConfigKey(CONFIG_KEY, "xFaderCurve")).toDouble();
	double sliderVal = SliderXFader->maximum() / MIXXX_XFADER_STEEPNESS_COEFF * (sliderTransform - 1.);
	SliderXFader->setValue((int)sliderVal);

	m_xFaderMode = config->getValueString(ConfigKey(CONFIG_KEY, "xFaderMode"));

    qDebug() << "loadSettings:" << sliderTransform << sliderVal << m_xFaderMode;

	if (m_xFaderMode == MIXXX_XFADER_SLOWFADE)
	{
        radioButtonSlowFade->setChecked(true);
        SliderXFader->setEnabled(false);
    }
    else if (m_xFaderMode == MIXXX_XFADER_FASTCUT)
    {
        radioButtonFastCut->setChecked(true);
        SliderXFader->setEnabled(false);
    }
    else if (m_xFaderMode == MIXXX_XFADER_CONSTPWR)
    {
        radioButtonConstantPower->setChecked(true);
        SliderXFader->setEnabled(true);
    }
    else
    {
        radioButtonSlowFade->setChecked(true);
        SliderXFader->setEnabled(false);
    }

	slotUpdateXFader();
	slotApply();
    drawXfaderDisplay();
}

/** Set the default values for all the widgets */
void DlgPrefCrossfader::setDefaults()
{
	SliderXFader->setValue(0);
	m_xFaderMode = MIXXX_XFADER_SLOWFADE;
	radioButtonSlowFade->setChecked(true);
	slotUpdate();
	slotApply();
}

/** Apply and save any changes made in the dialog */
void DlgPrefCrossfader::slotApply()
{
    config->set(ConfigKey(CONFIG_KEY, "xFaderMode"), ConfigValue(m_xFaderMode));
    m_COTCurve.slotSet(m_transform);
    m_COTCalibration.slotSet(m_cal);

	qDebug() << "slotApply crossfader:" << m_transform << m_xFaderMode;
}

/** Update the dialog when the crossfader mode is changed */
void DlgPrefCrossfader::slotUpdate()
{
    if (radioButtonSlowFade->isChecked())
    {
        m_xFaderMode = MIXXX_XFADER_SLOWFADE;
        SliderXFader->setEnabled(false);
        SliderXFader->setValue(SliderXFader->minimum());
    }
    if (radioButtonFastCut->isChecked())
    {
        m_xFaderMode = MIXXX_XFADER_FASTCUT;
        SliderXFader->setEnabled(false);
        SliderXFader->setValue(SliderXFader->maximum());
    }
    if (radioButtonConstantPower->isChecked())
    {
        m_xFaderMode = MIXXX_XFADER_CONSTPWR;
        SliderXFader->setEnabled(true);
	    double sliderTransform = config->getValueString(ConfigKey(CONFIG_KEY, "xFaderCurve")).toDouble();
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
	if (m_pxfScene)
	{
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
	for(int i=1; i < GRID_X_LINES; i++)
	{
		m_pxfScene->addLine(QLineF(0, i *(sizeY/GRID_X_LINES),sizeX,i *(sizeY/GRID_X_LINES)), gridPen);
	}
	for(int i=1; i < GRID_Y_LINES; i++)
	{
		m_pxfScene->addLine(QLineF( i * (sizeX/GRID_Y_LINES), 0, i * (sizeX/GRID_Y_LINES), sizeY), gridPen);
	}

	//Draw graph lines
	float gain1, gain2;
	QPoint pointTotal, point1, point2;
	QPoint pointTotalPrev, point1Prev, point2Prev;
	for(int i=0; i < sizeX; i++)
	{
		double xfadeStep = 2. / (double)sizeX;

		EngineXfader::getXfadeGains(gain1, gain2, (-1. + (xfadeStep * (double) i)), m_transform, m_cal);

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
		if(i == 0)
		{
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
void DlgPrefCrossfader::slotUpdateXFader()
{
	m_transform = 1. + ((double) SliderXFader->value() / SliderXFader->maximum() * MIXXX_XFADER_STEEPNESS_COEFF);

	if (m_xFaderMode == MIXXX_XFADER_SLOWFADE)
	{
	    m_cal = 0.0f;
	    SliderXFader->setValue(SliderXFader->minimum());
	}
	else if (m_xFaderMode == MIXXX_XFADER_FASTCUT)
	{
	    m_cal = 0.0f;
	    SliderXFader->setValue(SliderXFader->maximum());
	}
	else if (m_xFaderMode == MIXXX_XFADER_CONSTPWR)
	{
	    m_cal = EngineXfader::getCalibration(m_transform);
	}

	QString QS_transform = QString::number(m_transform);
	config->set(ConfigKey(CONFIG_KEY, "xFaderCurve"), ConfigValue(QS_transform));
	//config->set(ConfigKey(CONFIG_KEY, "xFaderCalibration"), ConfigValue(m_cal)); //FIXME: m_cal is a double - be forewarned

	drawXfaderDisplay();
}
