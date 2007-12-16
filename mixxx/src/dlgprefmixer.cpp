/***************************************************************************
                          dlgprefmixer.cpp  -  description
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

#include "dlgprefmixer.h"
#define MIXXX
#include "enginefilteriir.h"
#include "controlobject.h"
#include "enginexfader.h"
#include <qlineedit.h>
#include <q3filedialog.h>
#include <qwidget.h>
#include <qslider.h>
#include <qlabel.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qgraphicsscene.h>

#include <assert.h>

#define CONFIG_KEY "[Mixer Profile]"

extern "C" {
    double fid_design_coef(double * coef, int n_coef, char * spec, double rate, double freq0, double freq1, int adj);
}

DlgPrefMixer::DlgPrefMixer(QWidget * parent, ConfigObject<ConfigValue> * _config) :  QWidget(parent), Ui::DlgPrefMixerDlg()
{
    config = _config;

    setupUi(this);

    // Connection
#ifndef __LOFI__
    connect(SliderHiEQ,         SIGNAL(valueChanged(int)), this, SLOT(slotUpdateHiEQ()));
    connect(SliderHiEQ,         SIGNAL(sliderMoved(int)), this, SLOT(slotUpdateHiEQ()));
    connect(SliderHiEQ,         SIGNAL(sliderReleased()), this, SLOT(slotUpdateHiEQ()));

    connect(SliderLoEQ,         SIGNAL(valueChanged(int)), this, SLOT(slotUpdateLoEQ()));
    connect(SliderLoEQ,         SIGNAL(sliderMoved(int)), this,  SLOT(slotUpdateLoEQ()));
    connect(SliderLoEQ,         SIGNAL(sliderReleased()), this,  SLOT(slotUpdateLoEQ()));

	connect(CheckBoxLoFi,		SIGNAL(stateChanged(int)), this,	SLOT(slotLoFiChanged()));
#else
	CheckBoxLoFi->setChecked(true);
	slotLoFiChanged();
	CheckBoxLoFi->setEnabled(false);
#endif
	connect(SliderXFader,         SIGNAL(valueChanged(int)), this, SLOT(slotUpdateXFader()));
    connect(SliderXFader,         SIGNAL(sliderMoved(int)), this,  SLOT(slotUpdateXFader()));
    connect(SliderXFader,         SIGNAL(sliderReleased()), this,  SLOT(slotUpdateXFader()));

	connect(PushButtonReset,	  SIGNAL(clicked(bool)), this,	SLOT(setDefaults()));


	m_pxfScene = NULL;
	loadSettings();
}

DlgPrefMixer::~DlgPrefMixer()
{
}

void DlgPrefMixer::loadSettings()
{
	QString val = config->getValueString(ConfigKey(CONFIG_KEY, "HiEQFrequency"));
	if(config->getValueString(ConfigKey(CONFIG_KEY, "HiEQFrequency")) == QString(""))
		setDefaults();
	else
	{
		SliderHiEQ->setValue( getSliderPosition(config->getValueString(ConfigKey(CONFIG_KEY, "HiEQFrequency")).toInt()));
		SliderLoEQ->setValue( getSliderPosition(config->getValueString(ConfigKey(CONFIG_KEY, "LoEQFrequency")).toInt()));

		m_transform = 1. + ((double) SliderXFader->value() / SliderXFader->maximum());
		double sliderTransform = config->getValueString(ConfigKey(CONFIG_KEY, "xFaderCurve")).toDouble();
		double sliderVal = SliderXFader->maximum() * (sliderTransform - 1.);
		SliderXFader->setValue(sliderVal);

		if(config->getValueString(ConfigKey(CONFIG_KEY, "LoFiEQs")) == QString("yes"))
			CheckBoxLoFi->setChecked(true);
		else
			CheckBoxLoFi->setChecked(false);

		slotUpdate();
		slotApply();
	}
}

void DlgPrefMixer::setDefaults()
{
	SliderHiEQ->setValue(getSliderPosition(3000));
	SliderLoEQ->setValue(getSliderPosition(250));
	SliderXFader->setValue(0);
	slotUpdate();
	slotApply();
}

void DlgPrefMixer::slotLoFiChanged()
{
	GroupBoxHiEQ->setEnabled(! CheckBoxLoFi->isChecked());
	GroupBoxLoEQ->setEnabled(! CheckBoxLoFi->isChecked());
	if(CheckBoxLoFi->isChecked())
		config->set(ConfigKey(CONFIG_KEY, "LoFiEQs"), ConfigValue(QString("yes")));
	else
		config->set(ConfigKey(CONFIG_KEY, "LoFiEQs"), ConfigValue(QString("no")));
	slotApply();
}

void DlgPrefMixer::slotUpdateHiEQ()
{
    if(SliderHiEQ->value() < SliderLoEQ->value())
    {
        SliderHiEQ->setValue( SliderLoEQ->value());
    }
	m_highEqFreq = getEqFreq(SliderHiEQ->value());
	if(m_highEqFreq < 1000)
		TextHiEQ->setText( QString("%1 Hz").arg(m_highEqFreq));
	else
		TextHiEQ->setText( QString("%1 Khz").arg(m_highEqFreq / 1000.));
    config->set(ConfigKey(CONFIG_KEY, "HiEQFrequency"), ConfigValue(m_highEqFreq));

	slotApply();
}

void DlgPrefMixer::slotUpdateLoEQ()
{
    if(SliderLoEQ->value() > SliderHiEQ->value())
    {
        SliderLoEQ->setValue( SliderHiEQ->value());
    }
	m_lowEqFreq = getEqFreq(SliderLoEQ->value());
    if(m_lowEqFreq < 1000)
		TextLoEQ->setText( QString("%1 Hz").arg(m_lowEqFreq));
	else
		TextLoEQ->setText( QString("%1 Khz").arg(m_lowEqFreq / 1000.));
    config->set(ConfigKey(CONFIG_KEY, "LoEQFrequency"), ConfigValue(m_lowEqFreq));

	slotApply();
}


void DlgPrefMixer::slotApply()
{
#ifndef __LOFI__
	ControlObject::getControl(ConfigKey(CONFIG_KEY, "LoEQFrequency"))->set(m_lowEqFreq);
	ControlObject::getControl(ConfigKey(CONFIG_KEY, "HiEQFrequency"))->set(m_highEqFreq);
	ControlObject::getControl(ConfigKey(CONFIG_KEY, "LoFiEQs"))->set(CheckBoxLoFi->isChecked());
#endif

	ControlObject::getControl(ConfigKey(CONFIG_KEY, "xFaderCurve"))->set(m_transform);
	ControlObject::getControl(ConfigKey(CONFIG_KEY, "xFaderCalibration"))->set(m_cal);
}

void DlgPrefMixer::slotUpdate()
{
	slotUpdateLoEQ();
	slotUpdateHiEQ();
	slotUpdateXFader();
	slotLoFiChanged();
}

int DlgPrefMixer::getEqFreq(int sliderVal)
{
	if(sliderVal == 480)
		return 20050;	//normalize maximum to match label
	else
	{
		double dsliderVal = (double) sliderVal / 40;
		double result = (dsliderVal * dsliderVal * dsliderVal * dsliderVal);
		return (int) result;
	}
}

int DlgPrefMixer::getSliderPosition(int eqFreq)
{
	if(eqFreq >= 20050)
		return 480;
	double dfreq = eqFreq;
	double dsliderPos = pow(eqFreq, 1./4.);
	dsliderPos *= 40;
	return dsliderPos;
}


void DlgPrefMixer::drawXfaderDisplay()
{
#define GRID_X_LINES 4
#define GRID_Y_LINES 6

	int sizeX = graphicsViewXfader->width();
	int sizeY = graphicsViewXfader->height();

	//Initialize Scene
	delete m_pxfScene;
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
	FLOAT_TYPE gain1, gain2;
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
		pointTotal = QPoint(i - 2, ((1. - sum) * ((double) sizeY)));
		point1 = QPoint(i - 2, ((1. - gain1) * ((double)sizeY)));
		point2 = QPoint(i - 2, ((1. - gain2) * ((double)sizeY)));
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

void DlgPrefMixer::slotUpdateXFader()
{
	m_transform = 1. + ((double) SliderXFader->value() / SliderXFader->maximum() * 4.);
	m_cal = EngineXfader::getCalibration(m_transform);

	QString QS_transform = QString::number(m_transform);
	config->set(ConfigKey(CONFIG_KEY, "xFaderCurve"), ConfigValue(QS_transform));
	config->set(ConfigKey(CONFIG_KEY, "xFaderCalibration"), ConfigValue(m_cal));

	drawXfaderDisplay();
	slotApply();
}