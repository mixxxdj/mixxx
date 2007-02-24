/***************************************************************************
                          dlgprefsound.cpp  -  description
                             -------------------
    begin                : Thu Apr 17 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dlgprefsound.h"
#include "playerproxy.h"
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qthread.h>
#include "controlobject.h"
#include <qwidget.h>

DlgPrefSound::DlgPrefSound(QWidget *parent, PlayerProxy *_player,
                           ConfigObject<ConfigValue> *_config) : DlgPrefSoundDlg(parent,"")
{
    m_bLatencySliderDrag = false;
    player = _player;
    config = _config;
    
    // Update of latency label, when latency slider is updated
    connect(SliderLatency,                SIGNAL(sliderMoved(int)),  this, SLOT(slotLatency()));
    connect(SliderLatency,                SIGNAL(sliderReleased()),  this, SLOT(slotLatency()));
    connect(SliderLatency,                SIGNAL(valueChanged(int)), this, SLOT(slotLatency()));

    // Pitch-indp. time stretch disabled on mac
#ifdef __MACX__
    checkBoxPitchIndp->setChecked(false);
    checkBoxPitchIndp->setEnabled(false);
    textLabelSoundScaling->setEnabled(false);
#endif

    // Set default value for scale mode check box
    int iPitchIndpTimeStretch = config->getValueString(ConfigKey("[Soundcard]","PitchIndpTimeStretch")).toInt();
    if (iPitchIndpTimeStretch)
        checkBoxPitchIndp->setChecked(true);
    else
        checkBoxPitchIndp->setChecked(false);

    // Apply changes whenever apply signal is emitted
    connect(ComboBoxSoundcardMasterLeft,  SIGNAL(activated(int)),    this, SLOT(slotApply()));
    connect(ComboBoxSoundcardMasterRight, SIGNAL(activated(int)),    this, SLOT(slotApply()));
    connect(ComboBoxSoundcardHeadLeft,    SIGNAL(activated(int)),    this, SLOT(slotApply()));
    connect(ComboBoxSoundcardHeadRight,   SIGNAL(activated(int)),    this, SLOT(slotApply()));
    connect(ComboBoxSamplerates,          SIGNAL(activated(int)),    this, SLOT(slotApply()));
    connect(ComboBoxSoundApi,             SIGNAL(activated(int)),    this, SLOT(slotApplyApi()));
    connect(checkBoxPitchIndp,            SIGNAL(stateChanged(int)), this, SLOT(slotApply()));
    connect(SliderLatency,                SIGNAL(sliderPressed()),   this, SLOT(slotLatencySliderClick()));
    connect(SliderLatency,                SIGNAL(sliderReleased()),  this, SLOT(slotLatencySliderRelease()));
    connect(SliderLatency,                SIGNAL(valueChanged(int)), this, SLOT(slotLatencySliderChange(int)));
}

DlgPrefSound::~DlgPrefSound()
{
}

void DlgPrefSound::slotUpdate()
{
    QStringList interfaces = player->getInterfaces();
    QStringList::iterator it;
    int j;

    // Master left sound card info
    ComboBoxSoundcardMasterLeft->clear();
    ComboBoxSoundcardMasterLeft->insertItem("None");
    it = interfaces.begin();
    j = 1;
    while (it!=interfaces.end())
    {
        ComboBoxSoundcardMasterLeft->insertItem((*it));
        if ((*it)==config->getValueString(ConfigKey("[Soundcard]","DeviceMasterLeft")))
            ComboBoxSoundcardMasterLeft->setCurrentItem(j);
        ++j;
        ++it;
    }

    // Master right sound card info
    ComboBoxSoundcardMasterRight->clear();
    ComboBoxSoundcardMasterRight->insertItem("None");
    it = interfaces.begin();
    j = 1;
    while (it!=interfaces.end())
    {
        ComboBoxSoundcardMasterRight->insertItem((*it));
        if ((*it)==config->getValueString(ConfigKey("[Soundcard]","DeviceMasterRight")))
            ComboBoxSoundcardMasterRight->setCurrentItem(j);
        ++j;
        ++it;
    }

    // Head left sound card info
    ComboBoxSoundcardHeadLeft->clear();
    ComboBoxSoundcardHeadLeft->insertItem("None");
    it = interfaces.begin();
    j = 1;
    while (it!=interfaces.end())
    {
        ComboBoxSoundcardHeadLeft->insertItem((*it));
        if ((*it)==config->getValueString(ConfigKey("[Soundcard]","DeviceHeadLeft")))
            ComboBoxSoundcardHeadLeft->setCurrentItem(j);
        ++j;
        ++it;
    }

    // Head right sound card info
    ComboBoxSoundcardHeadRight->clear();
    ComboBoxSoundcardHeadRight->insertItem("None");
    it = interfaces.begin();
    j = 1;
    while (it!=interfaces.end())
    {
        ComboBoxSoundcardHeadRight->insertItem((*it));
        if ((*it)==config->getValueString(ConfigKey("[Soundcard]","DeviceHeadRight")))
            ComboBoxSoundcardHeadRight->setCurrentItem(j);
        ++j;
        ++it;
    }

    // Sample rate
    ComboBoxSamplerates->clear();
    QStringList srates = player->getSampleRates();
    it = srates.begin();
    j = 0;
    while (it!=srates.end())
    {
        ComboBoxSamplerates->insertItem((*it));
        if ((*it)==config->getValueString(ConfigKey("[Soundcard]","Samplerate")))
            ComboBoxSamplerates->setCurrentItem(j);
        ++j;
        ++it;
    }

    // Latency. Disconnect slider slot when updating...
    disconnect(SliderLatency,                SIGNAL(valueChanged(int)), this, SLOT(slotLatencySliderChange(int)));
    SliderLatency->setValue(getSliderLatencyVal(config->getValueString(ConfigKey("[Soundcard]","Latency")).toInt()));
    connect(SliderLatency,                SIGNAL(valueChanged(int)), this, SLOT(slotLatencySliderChange(int)));

    // API's
    ComboBoxSoundApi->clear();
    ComboBoxSoundApi->insertItem("None");
    QStringList api = player->getSoundApiList();
    it = api.begin();
    j = 1;
    while (it!=api.end())
    {
        ComboBoxSoundApi->insertItem((*it));
        if ((*it)==config->getValueString(ConfigKey("[Soundcard]","SoundApi")))
            ComboBoxSoundApi->setCurrentItem(j);
        ++j;
        ++it;
    }
}

void DlgPrefSound::slotLatency()
{
    TextLabelLatency->setText(QString("%1 ms").arg(getSliderLatencyMsec(SliderLatency->value())));
}

int DlgPrefSound::getSliderLatencyMsec(int val)
{
    if (val>16)
        val = (val-12)*(val-12);
    return val;
}

int DlgPrefSound::getSliderLatencyVal(int val)
{
    if (val<=16)
        return val;

    int i=5;
    while (i*i<val)
        i++;
    return 12+i;
}

void DlgPrefSound::slotApply()
{
    qDebug("Apply");

    // Update the config object with parameters from dialog
    config->set(ConfigKey("[Soundcard]","DeviceMasterLeft"), ConfigValue(ComboBoxSoundcardMasterLeft->currentText()));
    config->set(ConfigKey("[Soundcard]","DeviceMasterRight"), ConfigValue(ComboBoxSoundcardMasterRight->currentText()));
    config->set(ConfigKey("[Soundcard]","DeviceHeadLeft"), ConfigValue(ComboBoxSoundcardHeadLeft->currentText()));
    config->set(ConfigKey("[Soundcard]","DeviceHeadRight"), ConfigValue(ComboBoxSoundcardHeadRight->currentText()));
    config->set(ConfigKey("[Soundcard]","Samplerate"), ConfigValue(ComboBoxSamplerates->currentText()));
    config->set(ConfigKey("[Soundcard]","Latency"), ConfigValue(getSliderLatencyMsec(SliderLatency->value())));
    
    if (checkBoxPitchIndp->isChecked())
        config->set(ConfigKey("[Soundcard]","PitchIndpTimeStretch"), ConfigValue(1));
    else
        config->set(ConfigKey("[Soundcard]","PitchIndpTimeStretch"), ConfigValue(0));
     
    qDebug("request msec %i", getSliderLatencyMsec(SliderLatency->value()));
    
    // Close devices, and open using config data
    player->close();

	// Not much to do if the API is None...
	if (config->getValueString(ConfigKey("[Soundcard]","SoundApi"))!="None") {
		if (!player->open())
	        QMessageBox::warning(0, "Configuration error","Audio device could not be opened");
	    else
			slotUpdate();
	}
}

void DlgPrefSound::slotApplyApi()
{
    config->set(ConfigKey("[Soundcard]","SoundApi"), ConfigValue(ComboBoxSoundApi->currentText()));

	if (!player->setSoundApi(ComboBoxSoundApi->currentText()))
	{
		// Did they select the null api?
		if (ComboBoxSoundApi->currentText() != "None") {
			QMessageBox::warning(0, "Configuration problem","Sound API could not be initialized");
			config->set(ConfigKey("[Soundcard]","SoundApi"), ConfigValue("None"));
		}
	} else {
		player->setDefaults();
		if (!player->open()) {
			QMessageBox::warning(0, "Configuration error","Audio device could not be opened");
		}
	}

    slotUpdate();
}

void DlgPrefSound::slotLatencySliderClick()
{
    m_bLatencySliderDrag = true;
}

void DlgPrefSound::slotLatencySliderRelease()
{
    m_bLatencySliderDrag = false;
    slotApply();
}

void DlgPrefSound::slotLatencySliderChange(int)
{
    if (!m_bLatencySliderDrag)
        slotApply();
}
