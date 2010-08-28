/***************************************************************************
                          dlgprefeq.cpp  -  description
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

#include "dlgprefeq.h"
#include "engine/enginefilteriir.h"
#include "controlobject.h"
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

DlgPrefEQ::DlgPrefEQ(QWidget * parent, ConfigObject<ConfigValue> * _config) :  QWidget(parent), Ui::DlgPrefEQDlg()
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
	connect(PushButtonReset,	  SIGNAL(clicked(bool)), this,	SLOT(setDefaults()));

	m_lowEqFreq = 0;
	m_highEqFreq = 0;

	loadSettings();
}

DlgPrefEQ::~DlgPrefEQ()
{
}

void DlgPrefEQ::loadSettings()
{
	QString val = config->getValueString(ConfigKey(CONFIG_KEY, "HiEQFrequency"));
	if(config->getValueString(ConfigKey(CONFIG_KEY, "HiEQFrequency")) == QString(""))
		setDefaults();
	else
	{
		SliderHiEQ->setValue( getSliderPosition(config->getValueString(ConfigKey(CONFIG_KEY, "HiEQFrequency")).toInt()));
		SliderLoEQ->setValue( getSliderPosition(config->getValueString(ConfigKey(CONFIG_KEY, "LoEQFrequency")).toInt()));

		if(config->getValueString(ConfigKey(CONFIG_KEY, "LoFiEQs")) == QString("yes"))
			CheckBoxLoFi->setChecked(true);
		else
			CheckBoxLoFi->setChecked(false);

		slotUpdate();
		slotApply();
	}
}

void DlgPrefEQ::setDefaults()
{
	CheckBoxLoFi->setChecked(true);

	slotUpdate();
	slotApply();
}

void DlgPrefEQ::slotLoFiChanged()
{
	GroupBoxHiEQ->setEnabled(! CheckBoxLoFi->isChecked());
	GroupBoxLoEQ->setEnabled(! CheckBoxLoFi->isChecked());
	if(CheckBoxLoFi->isChecked())
		config->set(ConfigKey(CONFIG_KEY, "LoFiEQs"), ConfigValue(QString("yes")));
	else
		config->set(ConfigKey(CONFIG_KEY, "LoFiEQs"), ConfigValue(QString("no")));
	slotApply();
}

void DlgPrefEQ::slotUpdateHiEQ()
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

void DlgPrefEQ::slotUpdateLoEQ()
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

int DlgPrefEQ::getSliderPosition(int eqFreq)
{
        if(eqFreq >= 20050)
                return 480;
        double dsliderPos = pow(eqFreq, 1./4.);
        dsliderPos *= 40;
        return dsliderPos;
}


void DlgPrefEQ::slotApply()
{
#ifndef __LOFI__
	ControlObject::getControl(ConfigKey(CONFIG_KEY, "LoEQFrequency"))->set(m_lowEqFreq);
	ControlObject::getControl(ConfigKey(CONFIG_KEY, "HiEQFrequency"))->set(m_highEqFreq);
	ControlObject::getControl(ConfigKey(CONFIG_KEY, "LoFiEQs"))->set(CheckBoxLoFi->isChecked());
#endif
}

void DlgPrefEQ::slotUpdate()
{
	slotUpdateLoEQ();
	slotUpdateHiEQ();
	slotLoFiChanged();
}

int DlgPrefEQ::getEqFreq(int sliderVal)
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

