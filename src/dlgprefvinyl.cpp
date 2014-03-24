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

#include <QtDebug>

#include "dlgprefvinyl.h"

#include "controlobject.h"
#include "controlobjectslave.h"
#include "playermanager.h"
#include "vinylcontrol/defs_vinylcontrol.h"
#include "vinylcontrol/vinylcontrolmanager.h"

DlgPrefVinyl::DlgPrefVinyl(QWidget * parent, VinylControlManager *pVCMan,
                           ConfigObject<ConfigValue> * _config)
        : DlgPreferencePage(parent),
          m_pVCManager(pVCMan),
          config(_config),
          m_iConfiguredDecks(0) {
    m_pNumDecks = new ControlObjectSlave("[Master]", "num_decks", this);
    m_pNumDecks->connectValueChanged(SLOT(slotNumDecksChanged(double)));

    setupUi(this);

    //Set up a button group for the vinyl control behavior options
    QButtonGroup vinylControlMode;
    vinylControlMode.addButton(AbsoluteMode);
    vinylControlMode.addButton(RelativeMode);

    delete groupBoxSignalQuality->layout();
    QHBoxLayout *layout = new QHBoxLayout;

    for (int i = 0; i < kMaximumVinylControlInputs; ++i) {
        m_signalWidgets.push_back(new VinylControlSignalWidget());
        VinylControlSignalWidget* widget = m_signalWidgets.back();
        widget->setVinylInput(i);
        widget->setSize(MIXXX_VINYL_SCOPE_SIZE);
        layout->layout()->addWidget(widget);
    }

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

    ComboBoxVinylType3->addItem(MIXXX_VINYL_SERATOCV02VINYLSIDEA);
    ComboBoxVinylType3->addItem(MIXXX_VINYL_SERATOCV02VINYLSIDEB);
    ComboBoxVinylType3->addItem(MIXXX_VINYL_SERATOCD);
    ComboBoxVinylType3->addItem(MIXXX_VINYL_TRAKTORSCRATCHSIDEA);
    ComboBoxVinylType3->addItem(MIXXX_VINYL_TRAKTORSCRATCHSIDEB);
    ComboBoxVinylType3->addItem(MIXXX_VINYL_MIXVIBESDVS);

    ComboBoxVinylType4->addItem(MIXXX_VINYL_SERATOCV02VINYLSIDEA);
    ComboBoxVinylType4->addItem(MIXXX_VINYL_SERATOCV02VINYLSIDEB);
    ComboBoxVinylType4->addItem(MIXXX_VINYL_SERATOCD);
    ComboBoxVinylType4->addItem(MIXXX_VINYL_TRAKTORSCRATCHSIDEA);
    ComboBoxVinylType4->addItem(MIXXX_VINYL_TRAKTORSCRATCHSIDEB);
    ComboBoxVinylType4->addItem(MIXXX_VINYL_MIXVIBESDVS);

    ComboBoxVinylSpeed1->addItem(MIXXX_VINYL_SPEED_33);
    ComboBoxVinylSpeed1->addItem(MIXXX_VINYL_SPEED_45);
    ComboBoxVinylSpeed2->addItem(MIXXX_VINYL_SPEED_33);
    ComboBoxVinylSpeed2->addItem(MIXXX_VINYL_SPEED_45);
    ComboBoxVinylSpeed3->addItem(MIXXX_VINYL_SPEED_33);
    ComboBoxVinylSpeed3->addItem(MIXXX_VINYL_SPEED_45);
    ComboBoxVinylSpeed4->addItem(MIXXX_VINYL_SPEED_33);
    ComboBoxVinylSpeed4->addItem(MIXXX_VINYL_SPEED_45);

    connect(applyButton, SIGNAL(clicked()), this, SLOT(slotApply()));
    connect(VinylGain, SIGNAL(sliderReleased()), this, SLOT(VinylGainSlotApply()));
    //connect(ComboBoxDeviceDeck1, SIGNAL(currentIndexChanged()), this, SLOT(()));

    for (int i = 0; i < kMaxNumberOfDecks; ++i) {
        setDeckWidgetsVisible(i, false);
    }

    slotNumDecksChanged(m_pNumDecks->get());
}

DlgPrefVinyl::~DlgPrefVinyl()
{
    qDeleteAll(m_COSpeeds);
    qDeleteAll(m_signalWidgets);
}

void DlgPrefVinyl::slotNumDecksChanged(double dNumDecks) {
    int num_decks = static_cast<int>(dNumDecks);

    if (num_decks < 0 || num_decks > kMaxNumberOfDecks) {
        return;
    }

    for (int i = m_iConfiguredDecks; i < num_decks; ++i) {
        QString group = PlayerManager::groupForDeck(i);
        qDebug() << "creating speed CO " << group;
        m_COSpeeds.push_back(new ControlObjectSlave(group, "vinylcontrol_speed_type"));
        setDeckWidgetsVisible(i, true);
    }
}

/** @brief Performs any necessary actions that need to happen when the prefs dialog is opened */
void DlgPrefVinyl::slotShow() {
    if (m_pVCManager) {
        for (int i = 0; i < kMaximumVinylControlInputs; ++i) {
            m_pVCManager->addSignalQualityListener(m_signalWidgets[i]);
        }
    }

    //(Re)Initialize the signal quality indicators
    for (int i = 0; i < kMaximumVinylControlInputs; ++i) {
        m_signalWidgets[i]->resetWidget();
    }
}

/** @brief Performs any necessary actions that need to happen when the prefs dialog is closed */
void DlgPrefVinyl::slotHide() {
    if (m_pVCManager) {
        for (int i = 0; i < kMaximumVinylControlInputs; ++i) {
            m_pVCManager->removeSignalQualityListener(m_signalWidgets[i]);
        }
    }
}

void DlgPrefVinyl::slotUpdate()
{
    // Set vinyl control types in the comboboxes
    int combo_index =
            ComboBoxVinylType1->findText(config->getValueString(
                    ConfigKey("[Channel1]", "vinylcontrol_vinyl_type")));
    if (combo_index != -1)
        ComboBoxVinylType1->setCurrentIndex(combo_index);

    combo_index =
            ComboBoxVinylType2->findText(config->getValueString(
                    ConfigKey("[Channel2]", "vinylcontrol_vinyl_type")));
    if (combo_index != -1)
        ComboBoxVinylType2->setCurrentIndex(combo_index);

    combo_index =
            ComboBoxVinylType3->findText(config->getValueString(
                    ConfigKey("[Channel3]", "vinylcontrol_vinyl_type")));
    if (combo_index != -1)
        ComboBoxVinylType3->setCurrentIndex(combo_index);

    combo_index =
            ComboBoxVinylType4->findText(config->getValueString(
                    ConfigKey("[Channel4]", "vinylcontrol_vinyl_type")));
    if (combo_index != -1)
        ComboBoxVinylType4->setCurrentIndex(combo_index);


    combo_index =
            ComboBoxVinylSpeed1->findText(config->getValueString(
                    ConfigKey("[Channel1]", "vinylcontrol_speed_type")));
    if (combo_index != -1)
        ComboBoxVinylSpeed1->setCurrentIndex(combo_index);

    combo_index =
            ComboBoxVinylSpeed2->findText(config->getValueString(
                    ConfigKey("[Channel2]", "vinylcontrol_speed_type")));
    if (combo_index != -1)
        ComboBoxVinylSpeed2->setCurrentIndex(combo_index);

    combo_index =
            ComboBoxVinylSpeed3->findText(config->getValueString(
                    ConfigKey("[Channel3]", "vinylcontrol_speed_type")));
    if (combo_index != -1)
        ComboBoxVinylSpeed3->setCurrentIndex(combo_index);

    combo_index =
            ComboBoxVinylSpeed4->findText(config->getValueString(
                    ConfigKey("[Channel4]", "vinylcontrol_speed_type")));
    if (combo_index != -1)
        ComboBoxVinylSpeed4->setCurrentIndex(combo_index);

    // set lead-in time
    LeadinTime->setText (config->getValueString(ConfigKey(VINYL_PREF_KEY,"lead_in_time")) );

    // set Relative mode
    int iMode = config->getValueString(ConfigKey(VINYL_PREF_KEY,"mode")).toInt();
    if (iMode == MIXXX_VCMODE_ABSOLUTE)
        AbsoluteMode->setChecked(true);
    else if (iMode == MIXXX_VCMODE_RELATIVE)
        RelativeMode->setChecked(true);

    SignalQualityEnable->setChecked(
            (bool)config->getValueString(ConfigKey(VINYL_PREF_KEY, "show_signal_quality")).toInt());

    //set vinyl control gain
    VinylGain->setValue( config->getValueString(ConfigKey(VINYL_PREF_KEY,"gain")).toInt());

    for (int i = 0; i < kMaximumVinylControlInputs; ++i) {
        m_signalWidgets[i]->setVinylActive(m_pVCManager->vinylInputConnected(i));
    }
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

    config->set(ConfigKey(VINYL_PREF_KEY, "mode"), ConfigValue(iMode));
    config->set(ConfigKey(VINYL_PREF_KEY,"show_signal_quality"),
                ConfigValue((int)(SignalQualityEnable->isChecked())));

    m_pVCManager->requestReloadConfig();
    slotUpdate();
}

void DlgPrefVinyl::VinylTypeSlotApply()
{
    config->set(ConfigKey("[Channel1]","vinylcontrol_vinyl_type"),
                ConfigValue(ComboBoxVinylType1->currentText()));
    config->set(ConfigKey("[Channel2]","vinylcontrol_vinyl_type"),
                ConfigValue(ComboBoxVinylType2->currentText()));
    config->set(ConfigKey("[Channel3]","vinylcontrol_vinyl_type"),
                ConfigValue(ComboBoxVinylType3->currentText()));
    config->set(ConfigKey("[Channel4]","vinylcontrol_vinyl_type"),
                ConfigValue(ComboBoxVinylType4->currentText()));
    config->set(ConfigKey("[Channel1]","vinylcontrol_speed_type"),
                ConfigValue(ComboBoxVinylSpeed1->currentText()));
    config->set(ConfigKey("[Channel2]","vinylcontrol_speed_type"),
                ConfigValue(ComboBoxVinylSpeed2->currentText()));
    config->set(ConfigKey("[Channel3]","vinylcontrol_speed_type"),
                ConfigValue(ComboBoxVinylSpeed3->currentText()));
    config->set(ConfigKey("[Channel4]","vinylcontrol_speed_type"),
                ConfigValue(ComboBoxVinylSpeed4->currentText()));

    //Save the vinylcontrol_speed_type in ControlObjects as well so it can be retrieved quickly
    //on the fly. (eg. WSpinny needs to know how fast to spin)

    switch (m_COSpeeds.length()) {
    case 4:
        if (ComboBoxVinylSpeed4->currentText() == MIXXX_VINYL_SPEED_33) {
            m_COSpeeds[3]->slotSet(MIXXX_VINYL_SPEED_33_NUM);
        } else if (ComboBoxVinylSpeed4->currentText() == MIXXX_VINYL_SPEED_45) {
            m_COSpeeds[3]->slotSet(MIXXX_VINYL_SPEED_45_NUM);
        }
        // fallthrough intended
    case 3:
        if (ComboBoxVinylSpeed3->currentText() == MIXXX_VINYL_SPEED_33) {
            m_COSpeeds[2]->slotSet(MIXXX_VINYL_SPEED_33_NUM);
        } else if (ComboBoxVinylSpeed3->currentText() == MIXXX_VINYL_SPEED_45) {
            m_COSpeeds[2]->slotSet(MIXXX_VINYL_SPEED_45_NUM);
        }
        // fallthrough intended
    case 2:
        if (ComboBoxVinylSpeed2->currentText() == MIXXX_VINYL_SPEED_33) {
            m_COSpeeds[1]->slotSet(MIXXX_VINYL_SPEED_33_NUM);
        } else if (ComboBoxVinylSpeed2->currentText() == MIXXX_VINYL_SPEED_45) {
            m_COSpeeds[1]->slotSet(MIXXX_VINYL_SPEED_45_NUM);
        }
        // fallthrough intended
    case 1:
        if (ComboBoxVinylSpeed1->currentText() == MIXXX_VINYL_SPEED_33) {
            m_COSpeeds[0]->slotSet(MIXXX_VINYL_SPEED_33_NUM);
        } else if (ComboBoxVinylSpeed1->currentText() == MIXXX_VINYL_SPEED_45) {
            m_COSpeeds[0]->slotSet(MIXXX_VINYL_SPEED_45_NUM);
        }
    default:
        qWarning() << "Unexpected number of vinyl speed preference items";
    }
}

void DlgPrefVinyl::VinylGainSlotApply() {
    qDebug() << "in VinylGainSlotApply()" << "with gain:" << VinylGain->value();
    // Update the config key...
    config->set(ConfigKey(VINYL_PREF_KEY, "gain"), ConfigValue(VinylGain->value()));

    // Update the ControlObject...
    ControlObject::set(ConfigKey(VINYL_PREF_KEY, "gain"), VinylGain->value());
}

void DlgPrefVinyl::setDeckWidgetsVisible(int deck, bool visible) {
    switch(deck) {
    case 0:
        setDeck1WidgetsVisible(visible);
        break;
    case 1:
        setDeck2WidgetsVisible(visible);
        break;
    case 2:
        setDeck3WidgetsVisible(visible);
        break;
    case 3:
        setDeck4WidgetsVisible(visible);
        break;
    default:
        qWarning() << "Tried to set a vinyl preference widget visible that doesn't exist: " << deck;
    }
}

void DlgPrefVinyl::setDeck1WidgetsVisible(bool visible) {
    if (visible) {
        VinylLabel1->show();
        ComboBoxVinylType1->show();
        ComboBoxVinylSpeed1->show();
        if (m_signalWidgets.length() > 0) {
            m_signalWidgets[0]->show();
        }
    } else {
        VinylLabel1->hide();
        ComboBoxVinylType1->hide();
        ComboBoxVinylSpeed1->hide();
        if (m_signalWidgets.length() > 0) {
            m_signalWidgets[0]->hide();
        }
    }
}

void DlgPrefVinyl::setDeck2WidgetsVisible(bool visible) {
    if (visible) {
        VinylLabel2->show();
        ComboBoxVinylType2->show();
        ComboBoxVinylSpeed2->show();
        if (m_signalWidgets.length() > 1) {
            m_signalWidgets[1]->show();
        }
    } else {
        VinylLabel2->hide();
        ComboBoxVinylType2->hide();
        ComboBoxVinylSpeed2->hide();
        if (m_signalWidgets.length() > 1) {
            m_signalWidgets[1]->hide();
        }
    }
}

void DlgPrefVinyl::setDeck3WidgetsVisible(bool visible) {
    if (visible) {
        VinylLabel3->show();
        ComboBoxVinylType3->show();
        ComboBoxVinylSpeed3->show();
        if (m_signalWidgets.length() > 2) {
            m_signalWidgets[2]->show();
        }
    } else {
        VinylLabel3->hide();
        ComboBoxVinylType3->hide();
        ComboBoxVinylSpeed3->hide();
        if (m_signalWidgets.length() > 2) {
            m_signalWidgets[2]->hide();
        }
    }
}

void DlgPrefVinyl::setDeck4WidgetsVisible(bool visible) {
    if (visible) {
        VinylLabel4->show();
        ComboBoxVinylType4->show();
        ComboBoxVinylSpeed4->show();
        if (m_signalWidgets.length() > 3) {
            m_signalWidgets[3]->show();
        }
    } else {
        VinylLabel4->hide();
        ComboBoxVinylType4->hide();
        ComboBoxVinylSpeed4->hide();
        if (m_signalWidgets.length() > 3) {
            m_signalWidgets[3]->hide();
        }
    }
}
