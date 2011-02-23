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
#include <QtGui>
#include "dlgprefvinyl.h"
#include "controlobject.h"
#include "soundmanager.h"
#include "vinylcontrol.h" //For vinyl type string constants
#include "controlobjectthreadmain.h"
#include "vinylcontrolsignalwidget.h"
#include "dlgprefvinyl.h"

DlgPrefVinyl::DlgPrefVinyl(QWidget * parent, SoundManager * soundman,
                           ConfigObject<ConfigValue> * _config) : QWidget(parent), Ui::DlgPrefVinylDlg()
{
    m_pSoundManager = soundman;
    config = _config;
    m_dontForce = false;

    setupUi(this);

    //Set up a button group for the vinyl control behaviour options
    QButtonGroup vinylControlMode;
    vinylControlMode.addButton(AbsoluteMode);
    vinylControlMode.addButton(RelativeMode);

	m_signalWidget1.setSize(MIXXX_VINYL_SCOPE_SIZE);
	m_signalWidget2.setSize(MIXXX_VINYL_SCOPE_SIZE);

    delete groupBoxSignalQuality->layout();
    QHBoxLayout *layout = new QHBoxLayout;
    layout->layout()->addWidget(&m_signalWidget1);
    layout->layout()->addWidget(&m_signalWidget2);
    groupBoxSignalQuality->setLayout(layout);

    // Add vinyl types
    ComboBoxVinylType1->addItem(MIXXX_VINYL_SERATOCV02VINYLSIDEA);
    ComboBoxVinylType1->addItem(MIXXX_VINYL_SERATOCV02VINYLSIDEB);
    ComboBoxVinylType1->addItem(MIXXX_VINYL_SERATOCD);
    ComboBoxVinylType1->addItem(MIXXX_VINYL_TRAKTORSCRATCHSIDEA);
    ComboBoxVinylType1->addItem(MIXXX_VINYL_TRAKTORSCRATCHSIDEB);
    ComboBoxVinylType1->addItem(MIXXX_VINYL_FINALSCRATCH);
    ComboBoxVinylType1->addItem(MIXXX_VINYL_MIXVIBESDVSCD);
    
    ComboBoxVinylType2->addItem(MIXXX_VINYL_SERATOCV02VINYLSIDEA);
    ComboBoxVinylType2->addItem(MIXXX_VINYL_SERATOCV02VINYLSIDEB);
    ComboBoxVinylType2->addItem(MIXXX_VINYL_SERATOCD);
    ComboBoxVinylType2->addItem(MIXXX_VINYL_TRAKTORSCRATCHSIDEA);
    ComboBoxVinylType2->addItem(MIXXX_VINYL_TRAKTORSCRATCHSIDEB);
    ComboBoxVinylType2->addItem(MIXXX_VINYL_FINALSCRATCH);
    ComboBoxVinylType2->addItem(MIXXX_VINYL_MIXVIBESDVSCD);
    
    ComboBoxVinylSpeed1->addItem(MIXXX_VINYL_SPEED_33);
    ComboBoxVinylSpeed1->addItem(MIXXX_VINYL_SPEED_45);
    
    ComboBoxVinylSpeed2->addItem(MIXXX_VINYL_SPEED_33);
    ComboBoxVinylSpeed2->addItem(MIXXX_VINYL_SPEED_45);

    connect(VinylGain, SIGNAL(sliderReleased()), this, SLOT(VinylGainSlotApply()));
    //connect(ComboBoxDeviceDeck1, SIGNAL(currentIndexChanged()), this, SLOT(()));

    connect(VinylGain, SIGNAL(sliderReleased()), this, SLOT(settingsChanged()));
    connect(ComboBoxVinylType1, SIGNAL(currentIndexChanged(int)), this, SLOT(settingsChanged()));
    connect(ComboBoxVinylType2, SIGNAL(currentIndexChanged(int)), this, SLOT(settingsChanged()));
    connect(ComboBoxVinylSpeed1, SIGNAL(currentIndexChanged(int)), this, SLOT(settingsChanged()));
    connect(ComboBoxVinylSpeed2, SIGNAL(currentIndexChanged(int)), this, SLOT(settingsChanged()));
    connect(LeadinTime, SIGNAL(textChanged(const QString&)), this, SLOT(settingsChanged()));
    connect(NeedleSkipEnable, SIGNAL(stateChanged(int)), this, SLOT(settingsChanged()));
    connect(AbsoluteMode, SIGNAL(toggled(bool)), this, SLOT(settingsChanged()));
    connect(RelativeMode, SIGNAL(toggled(bool)), this, SLOT(settingsChanged()));
    connect(m_pSoundManager, SIGNAL(devicesSetup()), this, SLOT(settingsChanged()));
}

DlgPrefVinyl::~DlgPrefVinyl()
{
}

/** @brief Performs any necessary actions that need to happen when the prefs dialog is opened */
void DlgPrefVinyl::slotShow()
{
    QList<VinylControlProxy*> VCProxiesList = m_pSoundManager->getVinylControlProxies();
    if (VCProxiesList.value(0) != NULL)
        m_signalWidget1.setVinylControlProxy(VCProxiesList.value(0));
    if (VCProxiesList.value(1) != NULL)
        m_signalWidget2.setVinylControlProxy(VCProxiesList.value(1));

    //(Re)Initialize the signal quality indicators
    m_signalWidget1.resetWidget();
    m_signalWidget1.startDrawing();
    m_signalWidget2.resetWidget();
    m_signalWidget2.startDrawing();

}

/** @brief Performs any necessary actions that need to happen when the prefs dialog is closed */
void DlgPrefVinyl::slotClose()
{
    //Stop updating the vinyl control signal indicators when the prefs dialog is closed.
    m_signalWidget1.stopDrawing();
    m_signalWidget2.stopDrawing();
}

void DlgPrefVinyl::slotUpdate()
{
    m_dontForce = true; // otherwise all the signals fired in here will cause
                        // DlgPrefSound to call setupDevices needlessly :) -- bkgood
    // Set vinyl control types in the comboboxes
    int combo_index = ComboBoxVinylType1->findText(config->getValueString(ConfigKey("[Channel1]","strVinylType")));
    if (combo_index != -1)
        ComboBoxVinylType1->setCurrentIndex(combo_index);
        
    combo_index = ComboBoxVinylType2->findText(config->getValueString(ConfigKey("[Channel2]","strVinylType")));
    if (combo_index != -1)
        ComboBoxVinylType2->setCurrentIndex(combo_index);
        
    combo_index = ComboBoxVinylSpeed1->findText(config->getValueString(ConfigKey("[Channel1]","strVinylSpeed")));
    if (combo_index != -1)
        ComboBoxVinylSpeed1->setCurrentIndex(combo_index);
        
    combo_index = ComboBoxVinylSpeed2->findText(config->getValueString(ConfigKey("[Channel2]","strVinylSpeed")));
    if (combo_index != -1)
        ComboBoxVinylSpeed2->setCurrentIndex(combo_index);

    // set lead-in time
    LeadinTime->setText (config->getValueString(ConfigKey("[VinylControl]","LeadInTime")) );

    // set Relative mode
    int iMode = config->getValueString(ConfigKey("[VinylControl]","Mode")).toInt();
    if (iMode == MIXXX_VCMODE_ABSOLUTE)
        AbsoluteMode->setChecked(true);
    else if (iMode == MIXXX_VCMODE_RELATIVE)
        RelativeMode->setChecked(true);

    // Honour the Needle Skip Prevention setting.
    NeedleSkipEnable->setChecked( (bool)config->getValueString( ConfigKey("[VinylControl]", "NeedleSkipPrevention") ).toInt() );

    //set vinyl control gain
    VinylGain->setValue( config->getValueString(ConfigKey("[VinylControl]","VinylControlGain")).toInt());
    m_dontForce = false;
}

// Update the config object with parameters from dialog
void DlgPrefVinyl::slotApply()
{
	QString device2;
    qDebug() << "DlgPrefVinyl::Apply";

    // Lead-in time
    QString strLeadIn      = LeadinTime->text();
    bool isInteger;
    strLeadIn.toInt(&isInteger);
    if (isInteger)
        config->set(ConfigKey("[VinylControl]","LeadInTime"), strLeadIn);
    else
        config->set(ConfigKey("[VinylControl]","LeadInTime"), MIXXX_VC_DEFAULT_LEADINTIME);

    //Apply updates for everything else...
    VinylTypeSlotApply();
    VinylGainSlotApply();

    int iMode = 0;
    if (AbsoluteMode->isChecked())
        iMode = MIXXX_VCMODE_ABSOLUTE;
    if (RelativeMode->isChecked())
        iMode = MIXXX_VCMODE_RELATIVE;

    ControlObject::getControl(ConfigKey("[VinylControl]", "Mode"))->set(iMode);
    ControlObject::getControl(ConfigKey("[Channel1]", "VinylMode"))->set(iMode);
    ControlObject::getControl(ConfigKey("[Channel2]", "VinylMode"))->set(iMode);
    config->set(ConfigKey("[VinylControl]","Mode"), ConfigValue(iMode));
    config->set(ConfigKey("[VinylControl]","NeedleSkipPrevention" ), ConfigValue( (int)(NeedleSkipEnable->isChecked( )) ) );

    slotUpdate();
}

void DlgPrefVinyl::EnableRelativeModeSlotApply()
{

}

void DlgPrefVinyl::VinylTypeSlotApply()
{
    config->set(ConfigKey("[Channel1]","strVinylType"), ConfigValue(ComboBoxVinylType1->currentText()));
    config->set(ConfigKey("[Channel2]","strVinylType"), ConfigValue(ComboBoxVinylType2->currentText()));
    config->set(ConfigKey("[Channel1]","strVinylSpeed"), ConfigValue(ComboBoxVinylSpeed1->currentText()));
    config->set(ConfigKey("[Channel2]","strVinylSpeed"), ConfigValue(ComboBoxVinylSpeed2->currentText()));
}

void DlgPrefVinyl::VinylGainSlotApply()
{
    qDebug() << "in VinylGainSlotApply()" << "with gain:" << VinylGain->value();
    //Update the config key...
    config->set(ConfigKey("[VinylControl]","VinylControlGain"), ConfigValue(VinylGain->value()));

    //Update the ControlObject...
    ControlObject* pControlObjectVinylControlGain = ControlObject::getControl(ConfigKey("[VinylControl]", "VinylControlGain"));
    pControlObjectVinylControlGain->set(VinylGain->value());

    //qDebug() << "Setting Gain Text";
    //gain->setText(config->getValueString(ConfigKey("[VinylControl]","VinylControlGain")));        //this is probably ineffecient...
}

void DlgPrefVinyl::settingsChanged() {
    if (!m_dontForce) {
        emit(refreshVCProxies());
    }

    QList<VinylControlProxy*> VCProxiesList = m_pSoundManager->getVinylControlProxies();
    if (VCProxiesList.value(0) != NULL) {
        m_signalWidget1.setVinylControlProxy(VCProxiesList.value(0));
    }
    if (VCProxiesList.value(1) != NULL) {
        m_signalWidget2.setVinylControlProxy(VCProxiesList.value(1));
    }
    
    m_signalWidget1.setVinylActive(m_pSoundManager->hasVinylInput(0));
    m_signalWidget2.setVinylActive(m_pSoundManager->hasVinylInput(1));
}
