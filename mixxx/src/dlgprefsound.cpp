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
//#include "playerproxy.h"
#include <qcombobox.h> 
#include <QtDebug>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qthread.h>
#include "controlobject.h"
#include "sounddevice.h"
#include "soundmanager.h"
#include <qwidget.h>

DlgPrefSound::DlgPrefSound(QWidget *parent, SoundManager *_soundman,
                           ConfigObject<ConfigValue> *_config) : QWidget(parent), Ui::DlgPrefSoundDlg()
{
    m_bLatencySliderDrag = false;
    //player = _player;
    m_pSoundManager = _soundman;
    config = _config;
    
    setupUi(this);
    slotUpdate();
    slotLatency();
    
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
    /*
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
    */
    
    connect(SliderLatency,                SIGNAL(sliderReleased()),  this, SLOT(slotLatencySliderRelease()));
    connect(ComboBoxSoundApi,             SIGNAL(activated(int)),    this, SLOT(slotApplyApi()));
    
}

DlgPrefSound::~DlgPrefSound()
{
}

void DlgPrefSound::slotUpdate()
{
    // API's
    ComboBoxSoundApi->clear();
    ComboBoxSoundApi->insertItem(0, "None");
    //QStringList api = player->getSoundApiList();
    QList<QString> apis = m_pSoundManager->getHostAPIList();
    QListIterator<QString> api_it(apis);
    QString api;
    int j = 1;
    while (api_it.hasNext())
    {
        api = api_it.next();
        ComboBoxSoundApi->insertItem(j, api);
        if (api==config->getValueString(ConfigKey("[Soundcard]","SoundApi")))
            ComboBoxSoundApi->setCurrentIndex(j);
        ++j;
    }

    //Get the currently selected API (just like we did above kinda)
    QString selectedAPI = config->getValueString(ConfigKey("[Soundcard]","SoundApi"));

    //QStringList interfaces = player->getInterfaces();
    //QStringList::iterator it;

    QList<SoundDevice*> devices = m_pSoundManager->getDeviceList(selectedAPI);
    SoundDevice* dev;

    // Master left sound card info
    ComboBoxSoundcardMasterLeft->clear();
    ComboBoxSoundcardMasterLeft->insertItem(0, "None");
    QListIterator<SoundDevice*> it(devices);
    //it = devices.begin();
    j = 1;
    while (it.hasNext())
    {
        dev = it.next();
        ComboBoxSoundcardMasterLeft->insertItem(j, dev->getName());
        if (dev->getName()==config->getValueString(ConfigKey("[Soundcard]","DeviceMasterLeft")))
            ComboBoxSoundcardMasterLeft->setCurrentIndex(j);
        ++j;
    }

    // Master right sound card info
    ComboBoxSoundcardMasterRight->clear();
    ComboBoxSoundcardMasterRight->insertItem(0, "None");
    it.toFront();
    j = 1;
    while (it.hasNext())
    {
        dev = it.next();
        ComboBoxSoundcardMasterRight->insertItem(j, dev->getName());
        if (dev->getName()==config->getValueString(ConfigKey("[Soundcard]","DeviceMasterRight")))
            ComboBoxSoundcardMasterRight->setCurrentIndex(j);
        ++j;
    }

    // Head left sound card info
    ComboBoxSoundcardHeadLeft->clear();
    ComboBoxSoundcardHeadLeft->insertItem(0, "None");
    it.toFront();
    j = 1;
    while (it.hasNext())
    {
        dev = it.next();
        ComboBoxSoundcardHeadLeft->insertItem(j, dev->getName());
        if (dev->getName()==config->getValueString(ConfigKey("[Soundcard]","DeviceHeadLeft")))
            ComboBoxSoundcardHeadLeft->setCurrentIndex(j);
        ++j;
    }

    // Head right sound card info
    ComboBoxSoundcardHeadRight->clear();
    ComboBoxSoundcardHeadRight->insertItem(0, "None");
    it.toFront();
    j = 1;
    while (it.hasNext())
    {
        dev = it.next();
        ComboBoxSoundcardHeadRight->insertItem(j, dev->getName());
        if (dev->getName()==config->getValueString(ConfigKey("[Soundcard]","DeviceHeadRight")))
            ComboBoxSoundcardHeadRight->setCurrentIndex(j);
        ++j;
    }

    // Sample rate
    ComboBoxSamplerates->clear();
    //QStringList srates = player->getSampleRates();
    QList<QString> srates = m_pSoundManager->getSamplerateList();
    QListIterator<QString> srate_it(srates);
    QString srate;
    j = 0;
    while (srate_it.hasNext())
    {
        srate = srate_it.next();
        ComboBoxSamplerates->insertItem(j, srate);
        if (srate==config->getValueString(ConfigKey("[Soundcard]","Samplerate")))
            ComboBoxSamplerates->setCurrentIndex(j);
        ++j;
    }

    // Latency. Disconnect slider slot when updating...
    disconnect(SliderLatency,                SIGNAL(valueChanged(int)), this, SLOT(slotLatencySliderChange(int)));
    SliderLatency->setValue(getSliderLatencyVal(config->getValueString(ConfigKey("[Soundcard]","Latency")).toInt()));
    connect(SliderLatency,                SIGNAL(valueChanged(int)), this, SLOT(slotLatencySliderChange(int)));


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
    qDebug() << "DlgPrefSound::Apply";

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
    //player->close();
    m_pSoundManager->closeDevices();

	// Not much to do if the API is None...
	if (config->getValueString(ConfigKey("[Soundcard]","SoundApi"))!="None") 
	{
		if (m_pSoundManager->setupDevices() != 0)
	        QMessageBox::warning(0, "Configuration error","Audio device could not be opened");
	    else
			slotUpdate();
	}
}

void DlgPrefSound::slotApplyApi()
{
    qDebug() << "DlgPrefSound::slotApplyApi";
    
    config->set(ConfigKey("[Soundcard]","SoundApi"), ConfigValue(ComboBoxSoundApi->currentText()));

	if (m_pSoundManager->setHostAPI(ComboBoxSoundApi->currentText()) != 0)
	{
		// Did they select the null api?
		if (ComboBoxSoundApi->currentText() != "None") {
			QMessageBox::warning(0, "Configuration problem","Sound API could not be initialized");
			config->set(ConfigKey("[Soundcard]","SoundApi"), ConfigValue("None"));
		}
	} else {
		//player->setDefaults();
		m_pSoundManager->setDefaults();
		if (m_pSoundManager->setupDevices() != 0)
		{
			QMessageBox::warning(0, "Configuration error","Audio device could not be opened");
		}
	}
    m_pSoundManager->closeDevices();
	emit(apiUpdated());    
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
    //if (!m_bLatencySliderDrag)
    //    slotApply();
}

