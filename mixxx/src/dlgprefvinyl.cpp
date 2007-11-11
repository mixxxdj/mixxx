/***************************************************************************
                          dlgprefvinyl.cpp  -  description
                             -------------------
    begin                : Thu Oct 23 2006
    copyright            : (C) 2006 by Stefan Langhammer
                           (C) 2007 by Albert Santoni
    email                : stefan.langhammer@9elements.com
                           gamegod \a\t users.sf.net
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/


#include <QtCore>
#include <QtDebug>
#include <QList>
#include <QButtonGroup>
#include "dlgprefvinyl.h"
#include "playerproxy.h"
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qthread.h>
#include "controlobject.h"
#include "soundmanager.h"
#include "sounddevice.h"
#include <qwidget.h>
#include "vinylcontrol.h" //For vinyl type string constants


DlgPrefVinyl::DlgPrefVinyl(QWidget * parent, SoundManager * soundman,
                           ConfigObject<ConfigValue> * _config) : QWidget(parent), Ui::DlgPrefVinylDlg()
{
    //player = _player;
    m_pSoundManager = soundman;
    config = _config;

    setupUi(this);

    //Set up a button group for the vinyl control behaviour options
    QButtonGroup vinylControlMode;
    vinylControlMode.addButton(AbsoluteMode);
    vinylControlMode.addButton(RelativeMode);
    vinylControlMode.addButton(ScratchMode);

    //Device input.
    ComboBoxDeviceDeck1->setEnabled( TRUE );
    ComboBoxDeviceDeck2->setEnabled( TRUE );

    // Disabled Input Device selection (see note at the top)

    //connect(ComboBoxDeviceDeck1,  SIGNAL(activated(int)),    this, SLOT(slotApply()));
    //connect(ComboBoxDeviceDeck2,  SIGNAL(activated(int)),    this, SLOT(slotApply()));

    //connect(RelativeMode,              SIGNAL(stateChanged(int)), this, SLOT(EnableRelativeModeSlotApply()));
    //connect(ScratchMode,               SIGNAL(stateChanged(int)), this, SLOT(EnableScratchModeSlotApply()));
    //connect(VinylGain,              SIGNAL(valueChanged(int)), this, SLOT(VinylGainSlotApply()));

    // Connect event handler
    /*
       connect(ComboBoxChannelsDeck1, SIGNAL(activated(int)), this, SLOT(ChannelsSlotApply()));
       connect(ComboBoxChannelsDeck2, SIGNAL(activated(int)), this, SLOT(ChannelsSlotApply()));
       connect(EnableRIAA,            SIGNAL(stateChanged(int)), this, SLOT(EnableRIAASlotApply()));
       connect(AutoCalibration,	   SIGNAL(clicked()),		  this, SLOT(AutoCalibrationSlotApply()));
       connect(LeadinTime,			   SIGNAL( lostFocus ()),	  this, SLOT(slotApply()));
       connect(ComboBoxVinylType,	   SIGNAL(activated(int)),	  this, SLOT(VinylTypeSlotApply()));

     */
    // TODO: Enable this button; add two more text boxes for gain&precision; run the calibration test of Scratchlib
    AutoCalibration->setEnabled( FALSE );

    // Disable output text box
    precision->setEnabled( FALSE );
    gain->setEnabled( FALSE );

    // Add vinyl types
    ComboBoxVinylType->insertItem(MIXXX_VINYL_SERATOCV02VINYLSIDEA);
    ComboBoxVinylType->insertItem(MIXXX_VINYL_SERATOCV02VINYLSIDEB);
    ComboBoxVinylType->insertItem(MIXXX_VINYL_SERATOCD);
    ComboBoxVinylType->insertItem(MIXXX_VINYL_TRAKTORSCRATCHSIDEA);
    ComboBoxVinylType->insertItem(MIXXX_VINYL_TRAKTORSCRATCHSIDEB);
    ComboBoxVinylType->insertItem(MIXXX_VINYL_FINALSCRATCH);
    ComboBoxVinylType->insertItem(MIXXX_VINYL_MIXVIBESDVSCD);
}

DlgPrefVinyl::~DlgPrefVinyl()
{
}

void DlgPrefVinyl::slotUpdate()
{
    qDebug() << "DlgPrefVinyl::slotUpdate()";

    // Enable/Disable RIAA Correction checkbox
    int iEnableRIAA = config->getValueString(ConfigKey("[VinylControl]","bInputRIAACorrection")).toInt();
    if (iEnableRIAA)
        EnableRIAA->setChecked(true);
    else
        EnableRIAA->setChecked(false);

    // Get list of input devices, filtering by the current API.
    QList<SoundDevice *> devices = m_pSoundManager->getDeviceList(config->getValueString(ConfigKey("[Soundcard]","SoundApi")), false, true);
    QListIterator<SoundDevice *> device_it(devices);
    SoundDevice * device;
    int j;

    // Decks input devices
    ComboBoxDeviceDeck1->clear();
    ComboBoxDeviceDeck1->insertItem("None");
    ComboBoxDeviceDeck2->clear();
    ComboBoxDeviceDeck2->insertItem("None");
    j = 1;
    while (device_it.hasNext())
    {
        device = device_it.next();
        ComboBoxDeviceDeck1->insertItem(device->getName());
        ComboBoxDeviceDeck2->insertItem(device->getName());
        if (device->getName() == config->getValueString(ConfigKey("[VinylControl]","DeviceInputDeck1")))
        {
            ComboBoxDeviceDeck1->setCurrentItem(j);
        }
        if (device->getName() == config->getValueString(ConfigKey("[VinylControl]","DeviceInputDeck2")))
        {
            ComboBoxDeviceDeck2->setCurrentItem(j);
        }
        ++j;
    }

    // Get input channels of the current device - FIXME
    int channels;  
    channels = 0;
    QString channelname = "";

    // Set vinyl control types in the comboboxes
    int combo_index = ComboBoxVinylType->findText(config->getValueString(ConfigKey("[VinylControl]","strVinylType")));
    if (combo_index != -1)
        ComboBoxVinylType->setCurrentIndex(combo_index);

    // set lead-in time
    LeadinTime->setText (config->getValueString(ConfigKey("[VinylControl]","LeadInTime")) );

    // set Relative mode
    int iMode = config->getValueString(ConfigKey("[VinylControl]","Mode")).toInt();
    if (iMode == MIXXX_VCMODE_ABSOLUTE)
        AbsoluteMode->setChecked(true);
    else if (iMode == MIXXX_VCMODE_RELATIVE)
        RelativeMode->setChecked(true);
    else if (iMode == MIXXX_VCMODE_SCRATCH)
        ScratchMode->setChecked(true);

    //set vinyl control gain
    VinylGain->setValue( config->getValueString(ConfigKey("[VinylControl]","VinylControlGain")).toInt());

}


// Update the config object with parameters from dialog
void DlgPrefVinyl::slotApply()
{
    qDebug("DlgPrefVinyl::Apply");

    config->set(ConfigKey("[VinylControl]","DeviceInputDeck1"), ConfigValue(ComboBoxDeviceDeck1->currentText()));
    config->set(ConfigKey("[VinylControl]","DeviceInputDeck2"), ConfigValue(ComboBoxDeviceDeck2->currentText()));

    // Lead-in time
    QString strLeadIn      = LeadinTime->text();
    bool isInteger;
    int iLeadIn        = strLeadIn.toInt(&isInteger);
    if (isInteger)
        config->set(ConfigKey("[VinylControl]","LeadInTime"), strLeadIn);
    else
        config->set(ConfigKey("[VinylControl]","LeadInTime"), 35);

    // Apply Soundcard options

    //player->close();
    //m_pSoundManager->closeDevices();

    //NOTE: Soundcard options (input device selection) is applied by DlgPrefSound...

    //Apply updates for everything else...
    EnableRIAASlotApply();
    VinylTypeSlotApply();
    VinylGainSlotApply();
    AutoCalibrationSlotApply();

    int iMode = 0;
    if (AbsoluteMode->isChecked())
        iMode = MIXXX_VCMODE_ABSOLUTE;
    if (RelativeMode->isChecked())
        iMode = MIXXX_VCMODE_RELATIVE;        
    if (ScratchMode->isChecked())
        iMode = MIXXX_VCMODE_SCRATCH;    

    ControlObject::getControl(ConfigKey("[VinylControl]", "Mode"))->set(iMode);
    config->set(ConfigKey("[VinylControl]","Mode"), ConfigValue(iMode));

    //if (config->getValueString(ConfigKey("[Soundcard]","SoundApi"))=="None" || !m_pSoundManager->setupDevices())
    //if (config->getValueString(ConfigKey("[Soundcard]","SoundApi"))=="None"|| (m_pSoundManager->setupDevices() != 0))
    //    QMessageBox::warning(0, "Configuration error","Audio device could not be opened");
    //else
    slotUpdate();
}

void DlgPrefVinyl::ChannelsSlotApply()
{
    // Channels
    qDebug("DlgPrefVinyl::ChannelsSlotApply()");
    //config->set(ConfigKey("[VinylControl]","DeviceInputChannelsDeck1"), ConfigValue(ComboBoxChannelsDeck1->currentText()));
    //config->set(ConfigKey("[VinylControl]","DeviceInputChannelsDeck2"), ConfigValue(ComboBoxChannelsDeck2->currentText()));
}

void DlgPrefVinyl::EnableRIAASlotApply()
{
    // Input RIAA correction
    if (EnableRIAA->isChecked())
        config->set(ConfigKey("[VinylControl]","InputRIAACorrection"), ConfigValue(1));
    else
        config->set(ConfigKey("[VinylControl]","InputRIAACorrection"), ConfigValue(0));
}

void DlgPrefVinyl::EnableRelativeModeSlotApply()
{

}

void DlgPrefVinyl::EnableScratchModeSlotApply()
{

}

void DlgPrefVinyl::VinylTypeSlotApply()
{
    config->set(ConfigKey("[VinylControl]","strVinylType"), ConfigValue(ComboBoxVinylType->currentText()));
}

void DlgPrefVinyl::AutoCalibrationSlotApply()
{
    // Do the scratchlib calibration steps.
}

void DlgPrefVinyl::VinylGainSlotApply()
{
    qDebug() << "in VinylGainSlotApply()" << "with gain:" << VinylGain->value();
    //Update the config key...
    config->set(ConfigKey("[VinylControl]","VinylControlGain"), ConfigValue(VinylGain->value()));
   
    //Update the ControlObject...
    ControlObject* pControlObjectVinylControlGain = ControlObject::getControl(ConfigKey("[VinylControl]", "VinylControlGain"));
    pControlObjectVinylControlGain->set(VinylGain->value());
    
    qDebug("Setting Gain Text");
    gain->setText(config->getValueString(ConfigKey("[VinylControl]","VinylControlGain")));        //this is probably ineffecient...
}
