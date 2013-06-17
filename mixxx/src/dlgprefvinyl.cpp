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
#include "vinylcontrol/vinylcontrolmanager.h"
#include "vinylcontrol/defs_vinylcontrol.h"
#include "controlobjectthreadmain.h"

DlgPrefVinyl::DlgPrefVinyl(QWidget * parent, VinylControlManager *pVCMan,
                           ConfigObject<ConfigValue> * _config)
        : QWidget(parent),
          m_COSpeed1(ControlObject::getControl(ConfigKey("[Channel1]", "vinylcontrol_speed_type"))),
          m_COSpeed2(ControlObject::getControl(ConfigKey("[Channel2]", "vinylcontrol_speed_type"))) {
    m_pVCManager = pVCMan;
    config = _config;

    setupUi(this);

    //Set up a button group for the vinyl control behaviour options
    QButtonGroup vinylControlMode;
    vinylControlMode.addButton(AbsoluteMode);
    vinylControlMode.addButton(RelativeMode);

    m_signalWidget1.setVinylInput(0);
    m_signalWidget2.setVinylInput(1);
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
    ComboBoxVinylType1->addItem(MIXXX_VINYL_MIXVIBESDVS);

    ComboBoxVinylType2->addItem(MIXXX_VINYL_SERATOCV02VINYLSIDEA);
    ComboBoxVinylType2->addItem(MIXXX_VINYL_SERATOCV02VINYLSIDEB);
    ComboBoxVinylType2->addItem(MIXXX_VINYL_SERATOCD);
    ComboBoxVinylType2->addItem(MIXXX_VINYL_TRAKTORSCRATCHSIDEA);
    ComboBoxVinylType2->addItem(MIXXX_VINYL_TRAKTORSCRATCHSIDEB);
    ComboBoxVinylType2->addItem(MIXXX_VINYL_MIXVIBESDVS);

    ComboBoxVinylSpeed1->addItem(MIXXX_VINYL_SPEED_33);
    ComboBoxVinylSpeed1->addItem(MIXXX_VINYL_SPEED_45);

    ComboBoxVinylSpeed2->addItem(MIXXX_VINYL_SPEED_33);
    ComboBoxVinylSpeed2->addItem(MIXXX_VINYL_SPEED_45);

    connect(applyButton, SIGNAL(clicked()), this, SLOT(slotApply()));
    connect(VinylGain, SIGNAL(sliderReleased()), this, SLOT(VinylGainSlotApply()));
    //connect(ComboBoxDeviceDeck1, SIGNAL(currentIndexChanged()), this, SLOT(()));
}

DlgPrefVinyl::~DlgPrefVinyl()
{
}

/** @brief Performs any necessary actions that need to happen when the prefs dialog is opened */
void DlgPrefVinyl::slotShow() {
    if (m_pVCManager) {
        m_pVCManager->addSignalQualityListener(&m_signalWidget1);
        m_pVCManager->addSignalQualityListener(&m_signalWidget2);
    }

    //(Re)Initialize the signal quality indicators
    m_signalWidget1.resetWidget();
    m_signalWidget2.resetWidget();

}

/** @brief Performs any necessary actions that need to happen when the prefs dialog is closed */
void DlgPrefVinyl::slotClose() {
    if (m_pVCManager) {
        m_pVCManager->removeSignalQualityListener(&m_signalWidget1);
        m_pVCManager->removeSignalQualityListener(&m_signalWidget2);
    }
}

void DlgPrefVinyl::slotUpdate()
{
    // Set vinyl control types in the comboboxes
    int combo_index = ComboBoxVinylType1->findText(config->getValueString(ConfigKey("[Channel1]","vinylcontrol_vinyl_type")));
    if (combo_index != -1)
        ComboBoxVinylType1->setCurrentIndex(combo_index);

    combo_index = ComboBoxVinylType2->findText(config->getValueString(ConfigKey("[Channel2]","vinylcontrol_vinyl_type")));
    if (combo_index != -1)
        ComboBoxVinylType2->setCurrentIndex(combo_index);

    combo_index = ComboBoxVinylSpeed1->findText(config->getValueString(ConfigKey("[Channel1]","vinylcontrol_speed_type")));
    if (combo_index != -1)
        ComboBoxVinylSpeed1->setCurrentIndex(combo_index);

    combo_index = ComboBoxVinylSpeed2->findText(config->getValueString(ConfigKey("[Channel2]","vinylcontrol_speed_type")));
    if (combo_index != -1)
        ComboBoxVinylSpeed2->setCurrentIndex(combo_index);

    // set lead-in time
    LeadinTime->setText (config->getValueString(ConfigKey(VINYL_PREF_KEY,"lead_in_time")) );

    // set Relative mode
    int iMode = config->getValueString(ConfigKey(VINYL_PREF_KEY,"mode")).toInt();
    if (iMode == MIXXX_VCMODE_ABSOLUTE)
        AbsoluteMode->setChecked(true);
    else if (iMode == MIXXX_VCMODE_RELATIVE)
        RelativeMode->setChecked(true);

    // Honour the Needle Skip Prevention setting.
    NeedleSkipEnable->setChecked( (bool)config->getValueString( ConfigKey(VINYL_PREF_KEY, "needle_skip_prevention") ).toInt() );

    SignalQualityEnable->setChecked((bool)config->getValueString(ConfigKey(VINYL_PREF_KEY, "show_signal_quality") ).toInt() );

    //set vinyl control gain
    VinylGain->setValue( config->getValueString(ConfigKey(VINYL_PREF_KEY,"gain")).toInt());

    m_signalWidget1.setVinylActive(m_pVCManager->vinylInputEnabled(0));
    m_signalWidget2.setVinylActive(m_pVCManager->vinylInputEnabled(1));
}

// Update the config object with parameters from dialog
void DlgPrefVinyl::slotApply()
{
    qDebug() << "DlgPrefVinyl::Apply";

    // Lead-in time
    QString strLeadIn = LeadinTime->text();
    bool isInteger;
    strLeadIn.toInt(&isInteger);
    if (isInteger)
        config->set(ConfigKey(VINYL_PREF_KEY,"lead_in_time"), strLeadIn);
    else
        config->set(ConfigKey(VINYL_PREF_KEY,"lead_in_time"), MIXXX_VC_DEFAULT_LEADINTIME);

    //Apply updates for everything else...
    VinylTypeSlotApply();
    VinylGainSlotApply();

    int iMode = 0;
    if (AbsoluteMode->isChecked())
        iMode = MIXXX_VCMODE_ABSOLUTE;
    if (RelativeMode->isChecked())
        iMode = MIXXX_VCMODE_RELATIVE;

    config->set(ConfigKey(VINYL_PREF_KEY,"mode"), ConfigValue(iMode));
    config->set(ConfigKey(VINYL_PREF_KEY,"needle_skip_prevention" ), ConfigValue( (int)(NeedleSkipEnable->isChecked( )) ) );
    config->set(ConfigKey(VINYL_PREF_KEY,"show_signal_quality" ), ConfigValue( (int)(SignalQualityEnable->isChecked( )) ) );

    m_pVCManager->requestReloadConfig();
    slotUpdate();
}

void DlgPrefVinyl::VinylTypeSlotApply()
{
    config->set(ConfigKey("[Channel1]","vinylcontrol_vinyl_type"), ConfigValue(ComboBoxVinylType1->currentText()));
    config->set(ConfigKey("[Channel2]","vinylcontrol_vinyl_type"), ConfigValue(ComboBoxVinylType2->currentText()));
    config->set(ConfigKey("[Channel1]","vinylcontrol_speed_type"), ConfigValue(ComboBoxVinylSpeed1->currentText()));
    config->set(ConfigKey("[Channel2]","vinylcontrol_speed_type"), ConfigValue(ComboBoxVinylSpeed2->currentText()));

    //Save the vinylcontrol_speed_type in ControlObjects as well so it can be retrieved quickly
    //on the fly. (eg. WSpinny needs to know how fast to spin)
    if (ComboBoxVinylSpeed1->currentText() == MIXXX_VINYL_SPEED_33) {
        m_COSpeed1.slotSet(MIXXX_VINYL_SPEED_33_NUM);
    } else if (ComboBoxVinylSpeed1->currentText() == MIXXX_VINYL_SPEED_45) {
        m_COSpeed1.slotSet(MIXXX_VINYL_SPEED_45_NUM);
    }
    if (ComboBoxVinylSpeed2->currentText() == MIXXX_VINYL_SPEED_33) {
        m_COSpeed2.slotSet(MIXXX_VINYL_SPEED_33_NUM);
    } else if (ComboBoxVinylSpeed2->currentText() == MIXXX_VINYL_SPEED_45) {
        m_COSpeed2.slotSet(MIXXX_VINYL_SPEED_45_NUM);
    }
}

void DlgPrefVinyl::VinylGainSlotApply()
{
    qDebug() << "in VinylGainSlotApply()" << "with gain:" << VinylGain->value();
    //Update the config key...
    config->set(ConfigKey(VINYL_PREF_KEY,"gain"), ConfigValue(VinylGain->value()));

    //Update the ControlObject...
    ControlObject* pControlObjectVinylControlGain = ControlObject::getControl(ConfigKey(VINYL_PREF_KEY, "gain"));
    pControlObjectVinylControlGain->set(VinylGain->value());
}
