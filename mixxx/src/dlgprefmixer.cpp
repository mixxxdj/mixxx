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
#include <qfiledialog.h>
#include <qwidget.h>
#include <qslider.h>
#include <qlabel.h>
#include <qstring.h>
#include <qpushbutton.h>
#include "../lib/fidlib-0.9.9/fidlib.h"

#define CONFIG_KEY "[Mixer Profile]"

extern "C" {
    double fid_design_coef(double *coef, int n_coef, char *spec, double rate, double freq0, double freq1, int adj);
}

DlgPrefMixer::DlgPrefMixer(QWidget *parent, ConfigObject<ConfigValue> *_config) : DlgPrefMixerDlg(parent,"")
{
    config = _config;

    // Connection
    connect(SliderHiEQ,		SIGNAL(valueChanged(int)), this, SLOT(slotUpdateHiEQ()));
    connect(SliderHiEQ,		SIGNAL(sliderMoved(int)), this, SLOT(slotUpdateHiEQ()));
    connect(SliderHiEQ,		SIGNAL(sliderReleased()), this, SLOT(slotUpdateHiEQ()));
    
    connect(SliderLoEQ,		SIGNAL(valueChanged(int)), this, SLOT(slotUpdateLoEQ()));
    connect(SliderLoEQ,		SIGNAL(sliderMoved(int)), this,	 SLOT(slotUpdateLoEQ()));
    connect(SliderLoEQ,		SIGNAL(sliderReleased()), this,  SLOT(slotUpdateLoEQ()));
    
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

    double coef[MAX_COEF + 1];
    char spec_buf[12];
    sprintf(spec_buf, "HpBe4/%d", SliderHiEQ->value());
    coef[0] = fid_design_coef(coef + 1, 4, spec_buf, 44100., -1., -1., true);

    qDebug("Hi coefs %f:%f:%f:%f:%f:%dHz", (float) coef[0], (float) coef[1], (float) coef[2], (float) coef[3], (float) coef[4], SliderHiEQ->value());
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
