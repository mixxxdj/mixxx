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

    //Get access to the timecode strength ControlObjects
    m_timecodeQuality1 = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "VinylControlQuality")));
    m_timecodeQuality2 = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]", "VinylControlQuality")));

    m_vinylControlInput1L = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "VinylControlInputL")));
    m_vinylControlInput1R = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "VinylControlInputR")));
    m_vinylControlInput2L = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]", "VinylControlInputL")));
    m_vinylControlInput2R = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]", "VinylControlInputR")));



    m_signalWidget1.setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    m_signalWidget2.setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    const unsigned signalWidgetWidth = 75;
    const unsigned signalWidgetHeight = 75;
    m_signalWidget1.setMinimumSize(signalWidgetWidth, signalWidgetHeight);
    m_signalWidget2.setMinimumSize(signalWidgetWidth, signalWidgetHeight);
    m_signalWidget1.setMaximumSize(signalWidgetWidth, signalWidgetHeight);
    m_signalWidget2.setMaximumSize(signalWidgetWidth, signalWidgetHeight);
    m_signalWidget1.setupWidget();
    m_signalWidget2.setupWidget();

    delete groupBoxSignalQuality->layout();
    QHBoxLayout *layout = new QHBoxLayout;
    layout->layout()->addWidget(&m_signalWidget1);
    layout->layout()->addWidget(&m_signalWidget2);
    groupBoxSignalQuality->setLayout(layout);

    // Add vinyl types
    ComboBoxVinylType->addItem(MIXXX_VINYL_SERATOCV02VINYLSIDEA);
    ComboBoxVinylType->addItem(MIXXX_VINYL_SERATOCV02VINYLSIDEB);
    ComboBoxVinylType->addItem(MIXXX_VINYL_SERATOCD);
    ComboBoxVinylType->addItem(MIXXX_VINYL_TRAKTORSCRATCHSIDEA);
    ComboBoxVinylType->addItem(MIXXX_VINYL_TRAKTORSCRATCHSIDEB);
    ComboBoxVinylType->addItem(MIXXX_VINYL_FINALSCRATCH);
    ComboBoxVinylType->addItem(MIXXX_VINYL_MIXVIBESDVSCD);

    connect(VinylGain, SIGNAL(sliderReleased()), this, SLOT(VinylGainSlotApply()));
    //connect(ComboBoxDeviceDeck1, SIGNAL(currentIndexChanged()), this, SLOT(()));

    connect(VinylGain, SIGNAL(sliderReleased()), this, SLOT(settingsChanged()));
    connect(ComboBoxVinylType, SIGNAL(currentIndexChanged(int)), this, SLOT(settingsChanged()));
    connect(LeadinTime, SIGNAL(textChanged(const QString&)), this, SLOT(settingsChanged()));
    connect(NeedleSkipEnable, SIGNAL(stateChanged(int)), this, SLOT(settingsChanged()));
    connect(AbsoluteMode, SIGNAL(toggled(bool)), this, SLOT(settingsChanged()));
    connect(RelativeMode, SIGNAL(toggled(bool)), this, SLOT(settingsChanged()));
    connect(ScratchMode, SIGNAL(toggled(bool)), this, SLOT(settingsChanged()));
}

DlgPrefVinyl::~DlgPrefVinyl()
{
}

/** @brief Performs any necessary actions that need to happen when the prefs dialog is opened */
void DlgPrefVinyl::slotShow()
{
    //Connect the signal quality ControlObjects to this dialog, so they start updating
    connect(m_timecodeQuality1, SIGNAL(valueChanged(double)), this, SLOT(updateSignalQuality1(double)));
    connect(m_timecodeQuality2, SIGNAL(valueChanged(double)), this, SLOT(updateSignalQuality2(double)));

    connect(m_vinylControlInput1L, SIGNAL(valueChanged(double)), this, SLOT(updateInputLevelLeft1(double)));
    connect(m_vinylControlInput1R, SIGNAL(valueChanged(double)), this, SLOT(updateInputLevelRight1(double)));
    connect(m_vinylControlInput2L, SIGNAL(valueChanged(double)), this, SLOT(updateInputLevelLeft2(double)));
    connect(m_vinylControlInput2R, SIGNAL(valueChanged(double)), this, SLOT(updateInputLevelRight2(double)));

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
    m_timecodeQuality1->disconnect(this);
    m_timecodeQuality2->disconnect(this);
    m_vinylControlInput1L->disconnect(this);
    m_vinylControlInput1R->disconnect(this);
    m_vinylControlInput2L->disconnect(this);
    m_vinylControlInput2R->disconnect(this);
}

void DlgPrefVinyl::slotUpdate()
{
    m_dontForce = true; // otherwise all the signals fired in here will cause
                        // DlgPrefSound to call setupDevices needlessly :) -- bkgood
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
    AutoCalibrationSlotApply();

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

    //qDebug() << "Setting Gain Text";
    //gain->setText(config->getValueString(ConfigKey("[VinylControl]","VinylControlGain")));        //this is probably ineffecient...
}

/** @brief Wraps updateSignalQuality to work nicely with slots
  * @param value The new signal quality level for channel 1 (0.0f-1.0f)
  */
void DlgPrefVinyl::updateSignalQuality1(double value)
{
    m_signalWidget1.updateSignalQuality(VINYLCONTROL_SIGQUALITY, value);
}

/** @brief Wraps updateSignalQuality to work nicely with slots
  * @param value The new signal quality level for channel 2 (0.0f-1.0f)
  */
void DlgPrefVinyl::updateSignalQuality2(double value)
{
    m_signalWidget2.updateSignalQuality(VINYLCONTROL_SIGQUALITY, value);
}

/** @brief Wraps updateSignalQuality to work nicely with slots
  * @param value The new input level for the left channel of the first deck (0.0f-1.0f)
  */
void DlgPrefVinyl::updateInputLevelLeft1(double value)
{
    m_signalWidget1.updateSignalQuality(VINYLCONTROL_SIGLEFTCHANNEL, value);
}

/** @brief Wraps updateSignalQuality to work nicely with slots
  * @param value The new input level for the right channel of the first deck (0.0f-1.0f)
  */
void DlgPrefVinyl::updateInputLevelRight1(double value)
{
    m_signalWidget1.updateSignalQuality(VINYLCONTROL_SIGRIGHTCHANNEL, value);
}

/** @brief Wraps updateSignalQuality to work nicely with slots
  * @param value The new input level for the left channel of the second deck (0.0f-1.0f)
  */
void DlgPrefVinyl::updateInputLevelLeft2(double value)
{
    m_signalWidget2.updateSignalQuality(VINYLCONTROL_SIGLEFTCHANNEL, value);
}

/** @brief Wraps updateSignalQuality to work nicely with slots
  * @param value The new input level for the right channel of the second deck (0.0f-1.0f)
  */
void DlgPrefVinyl::updateInputLevelRight2(double value)
{
    m_signalWidget2.updateSignalQuality(VINYLCONTROL_SIGRIGHTCHANNEL, value);
}

void DlgPrefVinyl::settingsChanged() {
    if (!m_dontForce) {
        emit(refreshVCProxies());
    }
}
