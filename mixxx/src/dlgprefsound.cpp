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
#include "player.h"
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qlabel.h>
#include <qmessagebox.h>

DlgPrefSound::DlgPrefSound(QWidget *parent, Player *_player,
                           ConfigObject<ConfigValue> *_config) : DlgPrefSoundDlg(parent,"")
{
    player = _player;
    config = _config;

    // Update device options when a new device is selected
    connect(ComboBoxSoundcardMaster,  SIGNAL(activated(int)),    this, SLOT(slotMasterDeviceOptions()));
    connect(ComboBoxSoundcardHead,    SIGNAL(activated(int)),    this, SLOT(slotHeadDeviceOptions()));

    // The latency slider for head might need updating if head device equals master device
    connect(ComboBoxSoundcardHead,    SIGNAL(activated(int)),    this, SLOT(slotLatencyHead()));

    // Latency slider updates
    connect(SliderLatencyMaster,      SIGNAL(sliderMoved(int)),  this, SLOT(slotLatencyMaster()));
    connect(SliderLatencyHead,        SIGNAL(sliderMoved(int)),  this, SLOT(slotLatencyHead()));
    connect(SliderLatencyMaster,      SIGNAL(sliderReleased()),  this, SLOT(slotLatencyMaster()));
    connect(SliderLatencyHead,        SIGNAL(sliderReleased()),  this, SLOT(slotLatencyHead()));
    connect(SliderLatencyMaster,      SIGNAL(valueChanged(int)), this, SLOT(slotLatencyMaster()));
    connect(SliderLatencyHead,        SIGNAL(valueChanged(int)), this, SLOT(slotLatencyHead()));

    // Sound quality slider updates
    SliderSoundQuality->setValue(config->getValueString(ConfigKey("[Soundcard]","SoundQuality")).toInt());

    // Apply changes whenever apply signal is emitted
    connect(ComboBoxSoundcardMaster,  SIGNAL(activated(int)),    this, SLOT(slotApply()));
    connect(ComboBoxSoundcardHead,    SIGNAL(activated(int)),    this, SLOT(slotApply()));
    connect(ComboBoxChannelsMaster,   SIGNAL(activated(int)),    this, SLOT(slotApply()));
    connect(ComboBoxChannelsHead,     SIGNAL(activated(int)),    this, SLOT(slotApply()));
    connect(SliderLatencyMaster,      SIGNAL(valueChanged(int)), this, SLOT(slotApply()));
    connect(SliderLatencyHead,        SIGNAL(valueChanged(int)), this, SLOT(slotApply()));
    connect(SliderSoundQuality,       SIGNAL(valueChanged(int)), this, SLOT(slotApply()));
    connect(SliderSoundQuality,       SIGNAL(valueChanged(int)), this, SLOT(slotApply()));
}

DlgPrefSound::~DlgPrefSound()
{
}

void DlgPrefSound::slotUpdate()
{
    slotMasterDevice();
    slotHeadDevice();
}
    
void DlgPrefSound::slotMasterDevice()
{
    // Master sound card info
    ComboBoxSoundcardMaster->clear();

    QPtrList<Player::Info> *pInfo = player->getInfo();
    for (unsigned int j=0; j<pInfo->count(); j++)
    {
        Player::Info *p = pInfo->at(j);

        // Name of device.
        ComboBoxSoundcardMaster->insertItem(p->name);

        // If it's the first device, it becomes the default, if no device has been
        // selected previously. Thus update its properties
        slotMasterDeviceOptions();

        if (p->name == config->getValueString(ConfigKey("[Soundcard]","DeviceMaster")) &&
            !(p->noChannels==2 && p->name==ComboBoxSoundcardHead->currentText()))
        {
            ComboBoxSoundcardMaster->setCurrentItem(j);
            slotMasterDeviceOptions();
        }
    }

    // Latency
    SliderLatencyMaster->setValue(getSliderLatencyVal(config->getValueString(ConfigKey("[Soundcard]","LatencyMaster")).toInt()));
    slotLatencyMaster();
}

void DlgPrefSound::slotHeadDevice()
{
    // Headphone sound card info
    ComboBoxSoundcardHead->clear();

    // First device, "None"
    ComboBoxSoundcardHead->insertItem("None");
    slotHeadDeviceOptions();

    QPtrList<Player::Info> *pInfo = player->getInfo();
    for (unsigned int j=0; j<pInfo->count(); j++)
    {
        Player::Info *p = pInfo->at(j);

        // Name of device. On macx playback can only happen on one device at a time
#ifdef __MACX__
        if (p->noChannels>=4 && p->name==ComboBoxSoundcardMaster->currentText())
#endif
            ComboBoxSoundcardHead->insertItem(p->name);


        if (p->name == config->getValueString(ConfigKey("[Soundcard]","DeviceHeadphone")) &&
            !(p->noChannels==2 && p->name==ComboBoxSoundcardMaster->currentText()))
        {
            ComboBoxSoundcardHead->setCurrentItem(j+1);
            slotHeadDeviceOptions();
        }
    }

    int latency = config->getValueString(ConfigKey("[Soundcard]","LatencyHeadphone")).toInt();
    SliderLatencyHead->setValue(getSliderLatencyVal(latency));
    slotLatencyHead();
}

void DlgPrefSound::slotMasterDeviceOptions()
{
    QPtrList<Player::Info> *pInfo = player->getInfo();
    Player::Info *p = pInfo->first();

    while (p != 0)
    {
        if (ComboBoxSoundcardMaster->currentText() == p->name)
        {
            // Master channels
            ComboBoxChannelsMaster->clear();
            int j=0;
            for (int i=1; i<=p->noChannels; i+=2)
            {
                QString ch(QString("%1-%2").arg(i).arg(i+1));
                ComboBoxChannelsMaster->insertItem(ch);
                if (i==player->chMaster)
                    ComboBoxChannelsMaster->setCurrentItem(j);
                j++;
            }

            // Sample rates
            ComboBoxSamplerates->clear();
            {for (unsigned int i=0; i<p->sampleRates.size(); i++)
            {
                ComboBoxSamplerates->insertItem(QString("%1 Hz").arg(p->sampleRates[i]));
                if (p->sampleRates[i]==player->getPlaySrate())
                    ComboBoxSamplerates->setCurrentItem(i);
            }}

            // Bits - Allow only 16 bits
            ComboBoxBits->clear();
//            for (unsigned int i=0; i<p->bits.size(); i++)
//            {
//                ComboBoxBits->insertItem(QString("%1").arg(p->bits[i]));
//                if (p->bits[i]==player->BITS)
//                    ComboBoxBits->setCurrentItem(i);
//            }
            ComboBoxBits->insertItem(QString("16"));
        }
        // Get next device
        p = pInfo->next();
    }
}

void DlgPrefSound::slotHeadDeviceOptions()
{
    // Ensure sample rate list is contains all rates the master device supports
    slotMasterDeviceOptions();

    QPtrList<Player::Info> *pInfo = player->getInfo();
    Player::Info *p = pInfo->first();

    // Clear headphone channels
    ComboBoxChannelsHead->clear();

    while (p != 0)
    {
        if (ComboBoxSoundcardHead->currentText() == p->name)
        {
            // Headphone channels
            int j=0;
            for (int i=1; i<=p->noChannels; i+=2)
            {
                QString ch(QString("%1-%2").arg(i).arg(i+1));

                ComboBoxChannelsHead->insertItem(ch);
                if (i==player->chHead)
                    ComboBoxChannelsHead->setCurrentItem(j);
                j++;
            }

            // Limit sample rates to those provided by both cards. If no head card is selected, do nothing
            if (ComboBoxSoundcardHead->currentText() != "None")
            {
                {for (int i=0; i<ComboBoxSamplerates->count(); i++)
                {
                    QString srateStr = ComboBoxSamplerates->text(i);
                    int srate = srateStr.left(srateStr.length()-3).toInt();

                    bool foundSrate = false;
                    for (unsigned int j=0; j<p->sampleRates.size(); j++)
                    {
                        if (p->sampleRates[j] == srate)
                            foundSrate = true;
                    }
                    if (!foundSrate)
                        ComboBoxSamplerates->removeItem(i);
                }}
            }
        }

        // Get next device
        p = pInfo->next();
    }
}

void DlgPrefSound::slotLatencyMaster()
{
    TextLabelLatencyMaster->setText(QString("%1 ms").arg(getSliderLatencyMsec(SliderLatencyMaster->value())));
}

void DlgPrefSound::slotLatencyHead()
{
    // Enable/disable latency slider
    if (ComboBoxSoundcardHead->currentText() == "None" ||
        ComboBoxSoundcardHead->currentText() == ComboBoxSoundcardMaster->currentText())
    {
        SliderLatencyHead->setDisabled(true);
        TextLabelLatency->setDisabled(true);
        TextLabelLatencyHead->setDisabled(true);
        TextLabelLatencyLow->setDisabled(true);
        TextLabelLatencyHigh->setDisabled(true);
    }
    else
    {
        SliderLatencyHead->setEnabled(true);
        TextLabelLatency->setEnabled(true);
        TextLabelLatencyHead->setEnabled(true);
        TextLabelLatencyLow->setEnabled(true);
        TextLabelLatencyHigh->setEnabled(true);
    }

    // Update text label
    TextLabelLatencyHead->setText(QString("%1 ms").arg(getSliderLatencyMsec(SliderLatencyHead->value())));
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
    // Update the config object with parameters from dialog
    config->set(ConfigKey("[Soundcard]","DeviceMaster"), ConfigValue(ComboBoxSoundcardMaster->currentText()));
    config->set(ConfigKey("[Soundcard]","DeviceHeadphone"), ConfigValue(ComboBoxSoundcardHead->currentText()));
    QString temp = ComboBoxSamplerates->currentText();
    temp.truncate(temp.length()-3);
    config->set(ConfigKey("[Soundcard]","Samplerate"), ConfigValue(temp));
    config->set(ConfigKey("[Soundcard]","Bits"), ConfigValue(ComboBoxBits->currentText()));
    config->set(ConfigKey("[Soundcard]","ChannelMaster"), ConfigValue(ComboBoxChannelsMaster->currentText().left(1).toInt()));
    config->set(ConfigKey("[Soundcard]","ChannelHeadphone"), ConfigValue(ComboBoxChannelsHead->currentText().left(1).toInt()));
    config->set(ConfigKey("[Soundcard]","LatencyMaster"), ConfigValue(getSliderLatencyMsec(SliderLatencyMaster->value())));
    config->set(ConfigKey("[Soundcard]","LatencyHeadphone"), ConfigValue(getSliderLatencyMsec(SliderLatencyHead->value())));
    config->set(ConfigKey("[Soundcard]","SoundQuality"), ConfigValue(4-SliderSoundQuality->value()));

    // Close devices, and open using config data
    player->close();
    if (!player->open(false))
        QMessageBox::warning(0, "Configuration error","Problem opening audio device");
    else
        player->start();

    // Save preferences
    config->Save();
}

