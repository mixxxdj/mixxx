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
#include <qlineedit.h>
#include <q3filedialog.h>
#include <qwidget.h>
#include <qslider.h>
#include <qlabel.h>
#include <qstring.h>
#include <qpushbutton.h>

#define CONFIG_KEY "[Mixer Profile]"

extern "C" {
    double fid_design_coef(double * coef, int n_coef, char * spec, double rate, double freq0, double freq1, int adj);
}

DlgPrefMixer::DlgPrefMixer(QWidget * parent, ConfigObject<ConfigValue> * _config) :  QWidget(parent), Ui::DlgPrefMixerDlg()
{
    config = _config;

    setupUi(this);
    slotUpdateHiEQ();
    slotUpdateLoEQ();
    setMidEQ();

    // Connection
    connect(SliderHiEQ,         SIGNAL(valueChanged(int)), this, SLOT(slotUpdateHiEQ()));
    connect(SliderHiEQ,         SIGNAL(sliderMoved(int)), this, SLOT(slotUpdateHiEQ()));
    connect(SliderHiEQ,         SIGNAL(sliderReleased()), this, SLOT(slotUpdateHiEQ()));

    connect(SliderLoEQ,         SIGNAL(valueChanged(int)), this, SLOT(slotUpdateLoEQ()));
    connect(SliderLoEQ,         SIGNAL(sliderMoved(int)), this,  SLOT(slotUpdateLoEQ()));
    connect(SliderLoEQ,         SIGNAL(sliderReleased()), this,  SLOT(slotUpdateLoEQ()));

    //Setup Defaults
    SliderHiEQ->setValue( config->getValueString(ConfigKey(CONFIG_KEY, "HiEQFrequency")).toInt());
    TextHiEQ->setText( config->getValueString(ConfigKey(CONFIG_KEY, "HiEQFrequency")));

    SliderLoEQ->setValue( config->getValueString(ConfigKey(CONFIG_KEY, "LoEQFrequency")).toInt());
    TextLoEQ->setText( config->getValueString(ConfigKey(CONFIG_KEY, "LoEQFrequency")));
}

DlgPrefMixer::~DlgPrefMixer()
{
}

void DlgPrefMixer::slotUpdateHiEQ()
{
    if(SliderHiEQ->value() < SliderLoEQ->value())
    {
        SliderHiEQ->setValue( SliderLoEQ->value());
    }
    TextHiEQ->setText( QString("%1 Hz").arg(SliderHiEQ->value()));
    config->set(ConfigKey(CONFIG_KEY, "HiEQFrequency"), ConfigValue(SliderHiEQ->value()));
    setMidEQ();

}

void DlgPrefMixer::slotUpdateLoEQ()
{
    if(SliderLoEQ->value() > SliderHiEQ->value())
    {
        SliderLoEQ->setValue( SliderHiEQ->value());
    }
    TextLoEQ->setText( QString("%1 Hz").arg(SliderLoEQ->value()));
    config->set(ConfigKey(CONFIG_KEY, "LoEQFrequency"), ConfigValue(SliderLoEQ->value()));
    setMidEQ();
}

void DlgPrefMixer::setMidEQ()
{
    int midLoCorner, midHiCorner;

    midLoCorner = config->getValueString(ConfigKey(CONFIG_KEY, "LoEQFrequency")).toInt();
    midHiCorner = config->getValueString(ConfigKey(CONFIG_KEY, "HiEQFrequency")).toInt();

}

void DlgPrefMixer::slotApply()
{
}

void DlgPrefMixer::slotUpdate()
{
}

