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
#include <qpushbutton.h>
#include <qslider.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qthread.h>

DlgPrefSound::DlgPrefSound(QWidget *parent, PlayerProxy *_player,
                           ConfigObject<ConfigValue> *_config) : DlgPrefSoundDlg(parent,"")
{
    player = _player;
    config = _config;

    // Latency slider updates
    connect(SliderLatency,                SIGNAL(sliderMoved(int)),  this, SLOT(slotLatency()));
    connect(SliderLatency,                SIGNAL(sliderReleased()),  this, SLOT(slotLatency()));
    connect(SliderLatency,                SIGNAL(valueChanged(int)), this, SLOT(slotLatency()));

    // Set default sound quality as stored in config file if not already set
    if (config->getValueString(ConfigKey("[Soundcard]","SoundQuality")).length() == 0)
        config->set(ConfigKey("[Soundcard]","SoundQuality"),ConfigValue(4));

    // Sound quality slider updates
    SliderSoundQuality->setValue(2+4-config->getValueString(ConfigKey("[Soundcard]","SoundQuality")).toInt());

    // Apply changes whenever apply signal is emitted
    connect(ComboBoxSoundcardMasterLeft,  SIGNAL(activated(int)),    this, SLOT(slotApply()));
    connect(ComboBoxSoundcardMasterRight, SIGNAL(activated(int)),    this, SLOT(slotApply()));
    connect(ComboBoxSoundcardHeadLeft,    SIGNAL(activated(int)),    this, SLOT(slotApply()));
    connect(ComboBoxSoundcardHeadRight,   SIGNAL(activated(int)),    this, SLOT(slotApply()));
    connect(ComboBoxSamplerates,          SIGNAL(activated(int)),    this, SLOT(slotApply()));
    connect(ComboBoxSoundApi,             SIGNAL(activated(int)),    this, SLOT(slotApplyApi()));
    connect(SliderLatency,                SIGNAL(sliderMoved(int)),  this, SLOT(slotApply()));
    connect(SliderSoundQuality,           SIGNAL(valueChanged(int)), this, SLOT(slotApply()));
    connect(SliderSoundQuality,           SIGNAL(valueChanged(int)), this, SLOT(slotApply()));
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
    while ((*it))
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
    while ((*it))
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
    while ((*it))
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
    while ((*it))
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
    while ((*it))
    {
        ComboBoxSamplerates->insertItem((*it));
        if ((*it)==config->getValueString(ConfigKey("[Soundcard]","Samplerate")))
            ComboBoxSamplerates->setCurrentItem(j);
        ++j;
        ++it;
    }

    // Latency
    SliderLatency->setValue(getSliderLatencyVal(config->getValueString(ConfigKey("[Soundcard]","Latency")).toInt()));

    // API's
    ComboBoxSoundApi->clear();
    ComboBoxSoundApi->insertItem("None");
    QStringList api = player->getSoundApiList();
    it = api.begin();
    j = 1;
    while ((*it))
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

    QString temp = ComboBoxSamplerates->currentText();
    //temp.truncate(temp.length()-3);
    config->set(ConfigKey("[Soundcard]","Samplerate"), ConfigValue(temp));
    //config->set(ConfigKey("[Soundcard]","Bits"), ConfigValue(ComboBoxBits->currentText()));
    config->set(ConfigKey("[Soundcard]","Latency"), ConfigValue(getSliderLatencyMsec(SliderLatency->value())));
    config->set(ConfigKey("[Soundcard]","SoundQuality"), ConfigValue(2+4-SliderSoundQuality->value()));
    
    // Close devices, and open using config data
    player->close();

    if (!player->open())
        QMessageBox::warning(0, "Configuration error","Audio device could not be opened");

    // Configuration values might have changed after the opening of the player,
    // so ensure the form is updated...
    slotUpdate();
}

void DlgPrefSound::slotApplyApi()
{
    config->set(ConfigKey("[Soundcard]","SoundApi"), ConfigValue(ComboBoxSoundApi->currentText()));
    if (!player->setSoundApi(ComboBoxSoundApi->currentText()))
    {
        QMessageBox::warning(0, "Configuration problem","Sound API could not be initialized");
        config->set(ConfigKey("[Soundcard]","SoundApi"), ConfigValue("None"));
    }
    else if (!player->open())
        QMessageBox::warning(0, "Configuration error","Audio device could not be opened");

    slotUpdate();
}


