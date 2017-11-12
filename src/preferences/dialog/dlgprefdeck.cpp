/***************************************************************************
                           dlgprefcontrols.cpp  -  description
                          -------------------
    begin                : Sat Jul 5 2003
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

#include <QList>
#include <QDir>
#include <QToolTip>
#include <QDoubleSpinBox>
#include <QWidget>
#include <QLocale>
#include <QDesktopWidget>

#include "mixer/basetrackplayer.h"
#include "preferences/dialog/dlgprefdeck.h"
#include "preferences/usersettings.h"
#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "widget/wnumberpos.h"
#include "engine/enginebuffer.h"
#include "engine/ratecontrol.h"
#include "mixer/playermanager.h"
#include "mixer/playerinfo.h"
#include "control/controlobject.h"
#include "mixxx.h"
#include "defs_urls.h"

const int kDefaultRateRangePercent = 8;
const double kRateDirectionInverted = -1;

DlgPrefDeck::DlgPrefDeck(QWidget * parent, MixxxMainWindow * mixxx,
                         PlayerManager* pPlayerManager,
                         UserSettingsPointer  pConfig)
        :  DlgPreferencePage(parent),
           m_pConfig(pConfig),
           m_mixxx(mixxx),
           m_pPlayerManager(pPlayerManager),
           m_iNumConfiguredDecks(0),
           m_iNumConfiguredSamplers(0) {
    setupUi(this);

    m_pNumDecks = new ControlProxy("[Master]", "num_decks", this);
    m_pNumDecks->connectValueChanged(SLOT(slotNumDecksChanged(double)));
    slotNumDecksChanged(m_pNumDecks->get(), true);

    m_pNumSamplers = new ControlProxy("[Master]", "num_samplers", this);
    m_pNumSamplers->connectValueChanged(SLOT(slotNumSamplersChanged(double)));
    slotNumSamplersChanged(m_pNumSamplers->get(), true);

    // Track time display configuration
    m_pControlTrackTimeDisplay = new ControlObject(
            ConfigKey("[Controls]", "ShowDurationRemaining"));
    connect(m_pControlTrackTimeDisplay, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetTrackTimeDisplay(double)));

    // If not present in the config, set the default value
    if (!m_pConfig->exists(ConfigKey("[Controls]","PositionDisplay"))) {
        m_pConfig->set(ConfigKey("[Controls]","PositionDisplay"),
          QString::number(static_cast<int>(TrackTime::DisplayMode::Remaining)));
    }

    double positionDisplayType = m_pConfig->getValue(
            ConfigKey("[Controls]", "PositionDisplay"),
            static_cast<double>(TrackTime::DisplayMode::Elapsed));
    if (positionDisplayType ==
            static_cast<double>(TrackTime::DisplayMode::Remaining)) {
        radioButtonRemaining->setChecked(true);
        m_pControlTrackTimeDisplay->set(
            static_cast<double>(TrackTime::DisplayMode::Remaining));
    } else if (positionDisplayType ==
                   static_cast<double>(TrackTime::DisplayMode::ElapsedAndRemaining)) {
        radioButtonElapsedAndRemaining->setChecked(true);
        m_pControlTrackTimeDisplay->set(
            static_cast<double>(TrackTime::DisplayMode::ElapsedAndRemaining));
    } else {
        radioButtonElapsed->setChecked(true);
        m_pControlTrackTimeDisplay->set(
            static_cast<double>(TrackTime::DisplayMode::Elapsed));
    }
    connect(buttonGroupTrackTime, SIGNAL(buttonClicked(QAbstractButton*)),
            this, SLOT(slotSetTrackTimeDisplay(QAbstractButton *)));

    // Override Playing Track on Track Load
    // The check box reflects the opposite of the config value
    m_bDisallowTrackLoadToPlayingDeck = !m_pConfig->getValue(
            ConfigKey("[Controls]", "AllowTrackLoadToPlayingDeck"), false);
    checkBoxDisallowLoadToPlayingDeck->setChecked(m_bDisallowTrackLoadToPlayingDeck);
    connect(checkBoxDisallowLoadToPlayingDeck, SIGNAL(toggled(bool)),
            this, SLOT(slotDisallowTrackLoadToPlayingDeckCheckbox(bool)));

    // Jump to cue on track load
    // The check box reflects the opposite of the config value
    m_bJumpToCueOnTrackLoad = !m_pConfig->getValue(ConfigKey("[Controls]", "CueRecall"), false);
    checkBoxSeekToCue->setChecked(m_bJumpToCueOnTrackLoad);
    connect(checkBoxSeekToCue, SIGNAL(toggled(bool)),
            this, SLOT(slotJumpToCueOnTrackLoadCheckbox(bool)));

    m_bRateInverted = m_pConfig->getValue(ConfigKey("[Controls]", "RateDir"), false);
    setRateDirectionForAllDecks(m_bRateInverted);
    checkBoxInvertSpeedSlider->setChecked(m_bRateInverted);
    connect(checkBoxInvertSpeedSlider, SIGNAL(toggled(bool)),
            this, SLOT(slotRateInversionCheckbox(bool)));

    ComboBoxRateRange->clear();
    ComboBoxRateRange->addItem(tr("4%"), 4);
    ComboBoxRateRange->addItem(tr("6% (semitone)"), 6);
    ComboBoxRateRange->addItem(tr("8% (Technics SL-1210)"), 8);
    ComboBoxRateRange->addItem(tr("10%"), 10);
    ComboBoxRateRange->addItem(tr("16%"), 16);
    ComboBoxRateRange->addItem(tr("24%"), 24);
    ComboBoxRateRange->addItem(tr("50%"), 50);
    ComboBoxRateRange->addItem(tr("90%"), 90);
    connect(ComboBoxRateRange, SIGNAL(activated(int)),
            this, SLOT(slotRateRangeComboBox(int)));

    // RateRange is the legacy ConfigKey. RateRangePercent is used now.
    if (m_pConfig->exists(ConfigKey("[Controls]", "RateRange")) &&
        !m_pConfig->exists(ConfigKey("[Controls]", "RateRangePercent"))) {
        int legacyIndex = m_pConfig->getValueString(ConfigKey("[Controls]", "RateRange")).toInt();
        if (legacyIndex == 0) {
            m_iRateRangePercent = 6;
        } else if (legacyIndex == 1) {
            m_iRateRangePercent = 8;
        } else {
            m_iRateRangePercent = (legacyIndex-1) * 10;
        }
    } else {
        m_iRateRangePercent = m_pConfig->getValue(ConfigKey("[Controls]", "RateRangePercent"),
                                                  kDefaultRateRangePercent);
    }
    if (!(m_iRateRangePercent > 0 && m_iRateRangePercent <= 90)) {
        m_iRateRangePercent = kDefaultRateRangePercent;
    }
    setRateRangeForAllDecks(m_iRateRangePercent);

    //
    // Key lock mode
    //
    connect(buttonGroupKeyLockMode, SIGNAL(buttonClicked(QAbstractButton*)),
            this, SLOT(slotKeyLockModeSelected(QAbstractButton *)));

    m_keylockMode = static_cast<KeylockMode>(
        m_pConfig->getValue(ConfigKey("[Controls]", "keylockMode"),
                            static_cast<int>(KeylockMode::LockOriginalKey)));
    for (ControlProxy* pControl : m_keylockModeControls) {
        pControl->set(static_cast<double>(m_keylockMode));
    }

    //
    // Key unlock mode
    //
    connect(buttonGroupKeyUnlockMode, SIGNAL(buttonClicked(QAbstractButton*)),
            this, SLOT(slotKeyUnlockModeSelected(QAbstractButton *)));

    m_keyunlockMode = static_cast<KeyunlockMode>(
        m_pConfig->getValue(ConfigKey("[Controls]", "keyunlockMode"),
        static_cast<int>(KeyunlockMode::ResetLockedKey)));
    for (ControlProxy* pControl : m_keyunlockModeControls) {
        pControl->set(static_cast<int>(m_keyunlockMode));
    }

    //
    // Rate buttons configuration
    //
    //NOTE: THESE DEFAULTS ARE A LIE! You'll need to hack the same values into the static variables
    //      at the top of enginebuffer.cpp
    if (m_pConfig->getValueString(ConfigKey("[Controls]", "RateTempLeft")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]", "RateTempLeft"), ConfigValue(QString("4.0")));
    if (m_pConfig->getValueString(ConfigKey("[Controls]", "RateTempRight")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]", "RateTempRight"), ConfigValue(QString("2.0")));
    if (m_pConfig->getValueString(ConfigKey("[Controls]", "RatePermLeft")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]", "RatePermLeft"), ConfigValue(QString("0.50")));
    if (m_pConfig->getValueString(ConfigKey("[Controls]", "RatePermRight")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]", "RatePermRight"), ConfigValue(QString("0.05")));

    connect(spinBoxTempRateLeft, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetRateTempLeft(double)));
    connect(spinBoxTempRateRight, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetRateTempRight(double)));
    connect(spinBoxPermRateLeft, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetRatePermLeft(double)));
    connect(spinBoxPermRateRight, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetRatePermRight(double)));

    spinBoxTempRateLeft->setValue(m_pConfig->getValueString(
            ConfigKey("[Controls]", "RateTempLeft")).toDouble());
    spinBoxTempRateRight->setValue(m_pConfig->getValueString(
            ConfigKey("[Controls]", "RateTempRight")).toDouble());
    spinBoxPermRateLeft->setValue(m_pConfig->getValueString(
            ConfigKey("[Controls]", "RatePermLeft")).toDouble());
    spinBoxPermRateRight->setValue(m_pConfig->getValueString(
            ConfigKey("[Controls]", "RatePermRight")).toDouble());

    //
    // Cue Mode
    //

    // Add "(?)" with a manual link to the label
    labelCueMode->setText(
            labelCueMode->text() +
            " <a href=\"" +
            MIXXX_MANUAL_URL +
            "/chapters/user_interface.html#using-cue-modes\">(?)</a>");

    // Set default value in config file and control objects, if not present
    // Default is "0" = Mixxx Mode
    int cueDefaultValue = m_pConfig->getValue(
            ConfigKey("[Controls]", "CueDefault"), 0);

    // Update combo box
    // The itemData values are out of order to avoid breaking configurations
    // when Mixxx mode (no blinking) was introduced.
    // TODO: replace magic numbers with an enum class
    ComboBoxCueDefault->addItem(tr("Mixxx mode"), 0);
    ComboBoxCueDefault->addItem(tr("Mixxx mode (no blinking)"), 4);
    ComboBoxCueDefault->addItem(tr("Pioneer mode"), 1);
    ComboBoxCueDefault->addItem(tr("Denon mode"), 2);
    ComboBoxCueDefault->addItem(tr("Numark mode"), 3);
    ComboBoxCueDefault->addItem(tr("CUP mode"), 5);
    const int cueDefaultIndex = cueDefaultIndexByData(cueDefaultValue);
    ComboBoxCueDefault->setCurrentIndex(cueDefaultIndex);
    slotCueModeCombobox(cueDefaultIndex);
    connect(ComboBoxCueDefault, SIGNAL(activated(int)), this, SLOT(slotCueModeCombobox(int)));

    //
    // Ramping Temporary Rate Change configuration
    //

    // Set Ramp Rate On or Off
    connect(radioButtonSpeedBendRamping, SIGNAL(toggled(bool)),
            this, SLOT(slotSetRateRamp(bool)));
    if ((bool)
        m_pConfig->getValueString(ConfigKey("[Controls]", "RateRamp")).toInt()) {
        radioButtonSpeedBendRamping->setChecked(true);
    } else {
        radioButtonSpeedBendStatic->setChecked(true);
    }

    // Update Ramp Rate Sensitivity
    connect(SliderRateRampSensitivity, SIGNAL(valueChanged(int)),
            this, SLOT(slotSetRateRampSensitivity(int)));
    SliderRateRampSensitivity->setValue(m_pConfig->getValueString(
            ConfigKey("[Controls]", "RateRampSensitivity")).toInt());


    // Update "reset speed" and "reset pitch" check boxes
    // TODO: All defaults should only be set in slotResetToDefaults.
    int configSPAutoReset = m_pConfig->getValue<int>(
                    ConfigKey("[Controls]", "SpeedAutoReset"),
                    BaseTrackPlayer::RESET_PITCH);

    m_speedAutoReset = (configSPAutoReset==BaseTrackPlayer::RESET_SPEED ||
                        configSPAutoReset==BaseTrackPlayer::RESET_PITCH_AND_SPEED);
    m_pitchAutoReset = (configSPAutoReset==BaseTrackPlayer::RESET_PITCH ||
                        configSPAutoReset==BaseTrackPlayer::RESET_PITCH_AND_SPEED);

    // Do these need to be here when slotUpdate() has them as well?
    checkBoxResetSpeed->setChecked(m_speedAutoReset);
    checkBoxResetPitch->setChecked(m_pitchAutoReset);

    connect(checkBoxResetSpeed, SIGNAL(toggled(bool)),
            this, SLOT(slotUpdateSpeedAutoReset(bool)));
    connect(checkBoxResetPitch, SIGNAL(toggled(bool)),
            this, SLOT(slotUpdatePitchAutoReset(bool)));

    slotUpdate();
}

DlgPrefDeck::~DlgPrefDeck() {
    delete m_pControlTrackTimeDisplay;
    qDeleteAll(m_rateControls);
    qDeleteAll(m_rateDirectionControls);
    qDeleteAll(m_cueControls);
    qDeleteAll(m_rateRangeControls);
    qDeleteAll(m_keylockModeControls);
    qDeleteAll(m_keyunlockModeControls);
}

void DlgPrefDeck::slotUpdate() {
    double deck1RateRange = m_rateRangeControls[0]->get();
    int idx = ComboBoxRateRange->findData(static_cast<int>(deck1RateRange * 100));
    if (idx == -1) {
        ComboBoxRateRange->addItem(QString::number(deck1RateRange * 100.).append("%"),
                                   deck1RateRange * 100.);
    }
    ComboBoxRateRange->setCurrentIndex(idx);

    double deck1RateDirection = m_rateDirectionControls[0]->get();
    checkBoxInvertSpeedSlider->setChecked(deck1RateDirection == kRateDirectionInverted);

    if (m_keylockMode == KeylockMode::LockCurrentKey) {
        radioButtonCurrentKey->setChecked(true);
    } else {
        radioButtonOriginalKey->setChecked(true);
    }

    if (m_keyunlockMode == KeyunlockMode::KeepLockedKey) {
        radioButtonKeepUnlockedKey->setChecked(true);
    } else {
        radioButtonResetUnlockedKey->setChecked(true);
    }

    checkBoxResetSpeed->setChecked(m_speedAutoReset);
    checkBoxResetPitch->setChecked(m_pitchAutoReset);
}

void DlgPrefDeck::slotResetToDefaults() {
    // Track time display mode
    radioButtonRemaining->setChecked(true);

    // Up increases speed.
    checkBoxInvertSpeedSlider->setChecked(false);

    // 8% Rate Range
    ComboBoxRateRange->setCurrentIndex(ComboBoxRateRange->findData(8));

    // Don't load tracks into playing decks.
    checkBoxDisallowLoadToPlayingDeck->setChecked(true);

    // Mixxx cue mode
    ComboBoxCueDefault->setCurrentIndex(0);

    // Cue recall on.
    checkBoxSeekToCue->setChecked(true);

    // Rate-ramping default off.
    radioButtonSpeedBendStatic->setChecked(true);

    // 0 rate-ramp sensitivity
    SliderRateRampSensitivity->setValue(0);

    // Permanent and temporary pitch adjust fine/coarse.
    spinBoxTempRateLeft->setValue(4.0);
    spinBoxTempRateRight->setValue(2.0);
    spinBoxPermRateLeft->setValue(0.50);
    spinBoxPermRateRight->setValue(0.05);

    // Automatically reset the pitch/key but not speed/tempo slider on track load
    m_speedAutoReset = false;
    m_pitchAutoReset = true;

    checkBoxResetSpeed->setChecked(m_speedAutoReset);
    checkBoxResetPitch->setChecked(m_pitchAutoReset);

    m_keylockMode = KeylockMode::LockOriginalKey;
    radioButtonOriginalKey->setChecked(true);

    m_keyunlockMode = KeyunlockMode::ResetLockedKey;
    radioButtonResetUnlockedKey->setChecked(true);
}

void DlgPrefDeck::slotRateRangeComboBox(int index) {
    m_iRateRangePercent = ComboBoxRateRange->itemData(index).toInt();
}

void DlgPrefDeck::setRateRangeForAllDecks(int rangePercent) {
    for (ControlProxy* pControl : m_rateRangeControls) {
        pControl->set(rangePercent / 100.0);
    }
}

void DlgPrefDeck::slotRateInversionCheckbox(bool inverted) {
    m_bRateInverted = inverted;
}

void DlgPrefDeck::setRateDirectionForAllDecks(bool inverted) {
    double oldRateDirectionMultiplier = m_rateDirectionControls[0]->get();
    double rateDirectionMultiplier = 1.0;
    if (inverted) {
        rateDirectionMultiplier = kRateDirectionInverted;
    }
    for (ControlProxy* pControl : m_rateDirectionControls) {
        pControl->set(rateDirectionMultiplier);
    }

    // If the rate slider direction setting has changed,
    // multiply the rate by -1 so the current sound does not change.
    if (rateDirectionMultiplier != oldRateDirectionMultiplier) {
        for (ControlProxy* pControl : m_rateControls) {
            pControl->set(-1 * pControl->get());
        }
    }
}

void DlgPrefDeck::slotKeyLockModeSelected(QAbstractButton* pressedButton) {
    if (pressedButton == radioButtonCurrentKey) {
        m_keylockMode = KeylockMode::LockCurrentKey;
    } else {
        m_keylockMode = KeylockMode::LockOriginalKey;
    }
}

void DlgPrefDeck::slotKeyUnlockModeSelected(QAbstractButton* pressedButton) {
    if (pressedButton == radioButtonResetUnlockedKey) {
        m_keyunlockMode = KeyunlockMode::ResetLockedKey;
    } else {
        m_keyunlockMode = KeyunlockMode::KeepLockedKey;
    }
}

void DlgPrefDeck::slotDisallowTrackLoadToPlayingDeckCheckbox(bool checked) {
    m_bDisallowTrackLoadToPlayingDeck = checked;
}

void DlgPrefDeck::slotCueModeCombobox(int index) {
    m_iCueMode = ComboBoxCueDefault->itemData(index).toInt();
}

void DlgPrefDeck::slotJumpToCueOnTrackLoadCheckbox(bool checked) {
    m_bJumpToCueOnTrackLoad = checked;
}

void DlgPrefDeck::slotSetTrackTimeDisplay(QAbstractButton* b) {
    double timeDisplay;
    if (b == radioButtonRemaining) {
        timeDisplay = static_cast<double>(TrackTime::DisplayMode::Remaining);
    } else if (b == radioButtonElapsedAndRemaining) {
        timeDisplay = static_cast<double>(TrackTime::DisplayMode::ElapsedAndRemaining);
    } else {
        timeDisplay = static_cast<double>(TrackTime::DisplayMode::Elapsed);
    }
    m_pConfig->set(ConfigKey("[Controls]","PositionDisplay"), ConfigValue(timeDisplay));
    m_pControlTrackTimeDisplay->set(timeDisplay);
}

void DlgPrefDeck::slotSetTrackTimeDisplay(double v) {
    if (v == 1.0) {
        // Remaining
        radioButtonRemaining->setChecked(true);
        m_pConfig->set(ConfigKey("[Controls]", "PositionDisplay"), ConfigValue(1));
    } else if (v == 2.0) {
        // Elapsed and remaining
        radioButtonElapsedAndRemaining->setChecked(true);
        m_pConfig->set(ConfigKey("[Controls]", "PositionDisplay"), ConfigValue(2));
    } else {
        // Elapsed
        radioButtonElapsed->setChecked(true);
        m_pConfig->set(ConfigKey("[Controls]", "PositionDisplay"), ConfigValue(0));
    }
}

void DlgPrefDeck::slotSetRateTempLeft(double v) {
    QString str;
    str = str.setNum(v, 'f');
    m_pConfig->set(ConfigKey("[Controls]", "RateTempLeft"),ConfigValue(str));
    RateControl::setTemp(v);
}

void DlgPrefDeck::slotSetRateTempRight(double v) {
    QString str;
    str = str.setNum(v, 'f');
    m_pConfig->set(ConfigKey("[Controls]", "RateTempRight"),ConfigValue(str));
    RateControl::setTempSmall(v);
}

void DlgPrefDeck::slotSetRatePermLeft(double v) {
    QString str;
    str = str.setNum(v, 'f');
    m_pConfig->set(ConfigKey("[Controls]", "RatePermLeft"),ConfigValue(str));
    RateControl::setPerm(v);
}

void DlgPrefDeck::slotSetRatePermRight(double v) {
    QString str;
    str = str.setNum(v, 'f');
    m_pConfig->set(ConfigKey("[Controls]", "RatePermRight"),ConfigValue(str));
    RateControl::setPermSmall(v);
}

void DlgPrefDeck::slotSetRateRampSensitivity(int sense) {
    m_pConfig->set(ConfigKey("[Controls]", "RateRampSensitivity"),
                   ConfigValue(SliderRateRampSensitivity->value()));
    RateControl::setRateRampSensitivity(sense);
}

void DlgPrefDeck::slotSetRateRamp(bool mode) {
    m_pConfig->set(ConfigKey("[Controls]", "RateRamp"),
                   ConfigValue(radioButtonSpeedBendRamping->isChecked()));
    RateControl::setRateRamp(mode);
}

void DlgPrefDeck::slotApply() {
    // Set cue mode for every deck
    for (ControlProxy* pControl : m_cueControls) {
        pControl->set(m_iCueMode);
    }
    m_pConfig->setValue(ConfigKey("[Controls]", "CueDefault"), m_iCueMode);

    m_pConfig->setValue(ConfigKey("[Controls]", "AllowTrackLoadToPlayingDeck"),
                        !m_bDisallowTrackLoadToPlayingDeck);

    m_pConfig->setValue(ConfigKey("[Controls]", "CueRecall"), !m_bJumpToCueOnTrackLoad);

    // Set rate range
    setRateRangeForAllDecks(m_iRateRangePercent);
    m_pConfig->setValue(ConfigKey("[Controls]", "RateRangePercent"),
                        m_iRateRangePercent);

    setRateDirectionForAllDecks(m_bRateInverted);
    m_pConfig->setValue(ConfigKey("[Controls]", "RateDir"),
                        static_cast<int>(m_bRateInverted));

    int configSPAutoReset = BaseTrackPlayer::RESET_NONE;

    if (m_speedAutoReset && m_pitchAutoReset) {
        configSPAutoReset = BaseTrackPlayer::RESET_PITCH_AND_SPEED;
    } else if (m_speedAutoReset) {
        configSPAutoReset = BaseTrackPlayer::RESET_SPEED;
    } else if (m_pitchAutoReset) {
        configSPAutoReset = BaseTrackPlayer::RESET_PITCH;
    }

    m_pConfig->set(ConfigKey("[Controls]", "SpeedAutoReset"),
                   ConfigValue(configSPAutoReset));

    m_pConfig->setValue(ConfigKey("[Controls]", "keylockMode"),
                        static_cast<int>(m_keylockMode));
    // Set key lock behavior for every group
    for (ControlProxy* pControl : m_keylockModeControls) {
        pControl->set(static_cast<double>(m_keylockMode));
    }

    m_pConfig->setValue(ConfigKey("[Controls]", "keyunlockMode"),
                        static_cast<int>(m_keyunlockMode));
    // Set key un-lock behavior for every group
    for (ControlProxy* pControl : m_keyunlockModeControls) {
        pControl->set(static_cast<double>(m_keyunlockMode));
    }
}

void DlgPrefDeck::slotNumDecksChanged(double new_count, bool initializing) {
    int numdecks = static_cast<int>(new_count);
    if (numdecks <= m_iNumConfiguredDecks) {
        // TODO(owilliams): If we implement deck deletion, shrink the size of configured decks.
        return;
    }

    for (int i = m_iNumConfiguredDecks; i < numdecks; ++i) {
        QString group = PlayerManager::groupForDeck(i);
        m_rateControls.push_back(new ControlProxy(
                group, "rate"));
        m_rateRangeControls.push_back(new ControlProxy(
                group, "rateRange"));
        m_rateDirectionControls.push_back(new ControlProxy(
                group, "rate_dir"));
        m_cueControls.push_back(new ControlProxy(
                group, "cue_mode"));
        m_keylockModeControls.push_back(new ControlProxy(
                group, "keylockMode"));
        m_keylockModeControls.last()->set(static_cast<double>(m_keylockMode));
        m_keyunlockModeControls.push_back(new ControlProxy(
                group, "keyunlockMode"));
        m_keyunlockModeControls.last()->set(static_cast<double>(m_keyunlockMode));
    }

    // The rate range hasn't been read from the config file when this is first called.
    if (!initializing) {
        setRateDirectionForAllDecks(m_rateDirectionControls[0]->get() == kRateDirectionInverted);
        setRateRangeForAllDecks(m_rateRangeControls[0]->get() * 100);
    }
}

void DlgPrefDeck::slotNumSamplersChanged(double new_count, bool initializing) {
    int numsamplers = static_cast<int>(new_count);
    if (numsamplers <= m_iNumConfiguredSamplers) {
        return;
    }

    for (int i = m_iNumConfiguredSamplers; i < numsamplers; ++i) {
        QString group = PlayerManager::groupForSampler(i);
        m_rateControls.push_back(new ControlProxy(
                group, "rate"));
        m_rateRangeControls.push_back(new ControlProxy(
                group, "rateRange"));
        m_rateDirectionControls.push_back(new ControlProxy(
                group, "rate_dir"));
        m_cueControls.push_back(new ControlProxy(
                group, "cue_mode"));
        m_keylockModeControls.push_back(new ControlProxy(
                group, "keylockMode"));
        m_keylockModeControls.last()->set(static_cast<double>(m_keylockMode));
        m_keyunlockModeControls.push_back(new ControlProxy(
                group, "keyunlockMode"));
        m_keyunlockModeControls.last()->set(static_cast<double>(m_keyunlockMode));
    }

    // The rate range hasn't been read from the config file when this is first called.
    if (!initializing) {
        setRateDirectionForAllDecks(m_rateDirectionControls[0]->get() == kRateDirectionInverted);
        setRateRangeForAllDecks(m_rateRangeControls[0]->get() * 100);
    }
}

void DlgPrefDeck::slotUpdateSpeedAutoReset(bool b) {
    m_speedAutoReset = b;
}

void DlgPrefDeck::slotUpdatePitchAutoReset(bool b) {
    m_pitchAutoReset = b;
}

int DlgPrefDeck::cueDefaultIndexByData(int userData) const {
    for (int i = 0; i < ComboBoxCueDefault->count(); ++i) {
        if (ComboBoxCueDefault->itemData(i).toInt() == userData) {
            return i;
        }
    }
    qWarning() << "No default cue behavior found for value" << userData
               << "returning default";
    return 0;
}
