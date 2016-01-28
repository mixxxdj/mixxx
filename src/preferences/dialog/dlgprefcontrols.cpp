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
#include "preferences/dialog/dlgprefcontrols.h"
#include "preferences/constants.h"
#include "preferences/usersettings.h"
#include "controlobject.h"
#include "controlobjectslave.h"
#include "widget/wnumberpos.h"
#include "engine/enginebuffer.h"
#include "engine/ratecontrol.h"
#include "skin/skinloader.h"
#include "skin/legacyskinparser.h"
#include "mixer/playermanager.h"
#include "controlobject.h"
#include "mixxx.h"
#include "defs_urls.h"

DlgPrefControls::DlgPrefControls(QWidget * parent, MixxxMainWindow * mixxx,
                                 SkinLoader* pSkinLoader,
                                 PlayerManager* pPlayerManager,
                                 UserSettingsPointer  pConfig)
        :  DlgPreferencePage(parent),
           m_pConfig(pConfig),
           m_mixxx(mixxx),
           m_pSkinLoader(pSkinLoader),
           m_pPlayerManager(pPlayerManager),
           m_iNumConfiguredDecks(0),
           m_iNumConfiguredSamplers(0) {
    setupUi(this);

    m_pNumDecks = new ControlObjectSlave("[Master]", "num_decks", this);
    m_pNumDecks->connectValueChanged(SLOT(slotNumDecksChanged(double)));
    slotNumDecksChanged(m_pNumDecks->get());

    m_pNumSamplers = new ControlObjectSlave("[Master]", "num_samplers", this);
    m_pNumSamplers->connectValueChanged(SLOT(slotNumSamplersChanged(double)));
    slotNumSamplersChanged(m_pNumSamplers->get());

    // Track time display configuration
    m_pControlTrackTimeDisplay = new ControlObject(
            ConfigKey("[Controls]", "ShowDurationRemaining"));
    connect(m_pControlTrackTimeDisplay, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetTrackTimeDisplay(double)));

    // If not present in the config, set the default value
    if (!m_pConfig->exists(ConfigKey("[Controls]","PositionDisplay")))
        m_pConfig->set(ConfigKey("[Controls]","PositionDisplay"),ConfigValue(0));

    if (m_pConfig->getValueString(ConfigKey("[Controls]", "PositionDisplay")).toInt() == 1) {
        radioButtonRemaining->setChecked(true);
        m_pControlTrackTimeDisplay->set(1.0);
    } else {
        radioButtonElapsed->setChecked(true);
        m_pControlTrackTimeDisplay->set(0.0);
    }
    connect(buttonGroupTrackTime, SIGNAL(buttonClicked(QAbstractButton*)),
            this, SLOT(slotSetTrackTimeDisplay(QAbstractButton *)));

    // Set default direction as stored in config file
    if (m_pConfig->getValueString(ConfigKey("[Controls]", "RateDir")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]", "RateDir"),ConfigValue(0));

    connect(checkBoxInvertSpeedSlider, SIGNAL(toggled(bool)),
            this, SLOT(slotSetRateDir(bool)));

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
            this, SLOT(slotSetRateRange(int)));

    // Set default range as stored in config file
    if (m_pConfig->getValueString(ConfigKey("[Controls]", "RateRangePercent")).length() == 0) {
        // Fall back to old [Controls]RateRange
        if (m_pConfig->getValueString(ConfigKey("[Controls]", "RateRange")).length() == 0) {
            m_pConfig->set(ConfigKey("[Controls]", "RateRangePercent"), ConfigValue(8));
        } else {
            int oldIdx = m_pConfig->getValueString(ConfigKey("[Controls]", "RateRange")).toInt();
            double oldRange = static_cast<double>(oldIdx-1) / 10.0;
            if (oldIdx == 0) {
                oldRange = 0.06;
            }
            if (oldIdx == 1) {
                oldRange = 0.08;
            }
            m_pConfig->set(ConfigKey("[Controls]", "RateRangePercent"),
                           ConfigValue(static_cast<int>(oldRange * 100.)));
            slotSetRateRangePercent(oldRange * 100.);
        }
    }

    //
    // Key lock mode
    //
    connect(buttonGroupKeyLockMode, SIGNAL(buttonClicked(QAbstractButton*)),
            this, SLOT(slotKeyLockMode(QAbstractButton *)));

    m_keylockMode = m_pConfig->getValueString(
        ConfigKey("[Controls]", "keylockMode"), "0").toInt();
    foreach (ControlObjectSlave* pControl, m_keylockModeControls) {
        pControl->set(m_keylockMode);
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

//     labelSpeedRampSensitivity->setEnabled(true);
//     SliderRateRampSensitivity->setEnabled(true);
//     SpinBoxRateRampSensitivity->setEnabled(true);

    //
    // Override Playing Track on Track Load
    //
    // The check box reflects the opposite of the config value
    checkBoxDisallowLoadToPlayingDeck->setChecked(
        m_pConfig->getValueString(ConfigKey("[Controls]", "AllowTrackLoadToPlayingDeck")).toInt()==0);
    connect(checkBoxDisallowLoadToPlayingDeck, SIGNAL(toggled(bool)),
            this, SLOT(slotSetAllowTrackLoadToPlayingDeck(bool)));

    //
    // Locale setting
    //

    // Iterate through the available locales and add them to the combobox
    // Borrowed following snippet from http://qt-project.org/wiki/How_to_create_a_multi_language_application
    QString translationsFolder = m_pConfig->getResourcePath() + "translations/";
    QString currentLocale = pConfig->getValueString(ConfigKey("[Config]", "Locale"));

    QDir translationsDir(translationsFolder);
    QStringList fileNames = translationsDir.entryList(QStringList("mixxx_*.qm"));
    fileNames.push_back("mixxx_en_US.qm"); // add source language as a fake value

    bool indexFlag = false; // it'll indicate if the selected index changed.
    for (int i = 0; i < fileNames.size(); ++i) {
        // Extract locale from filename
        QString locale = fileNames[i];
        locale.truncate(locale.lastIndexOf('.'));
        locale.remove(0, locale.indexOf('_') + 1);
        QLocale qlocale = QLocale(locale);

        QString lang = QLocale::languageToString(qlocale.language());
        QString country = QLocale::countryToString(qlocale.country());
        if (lang == "C") { // Ugly hack to remove the non-resolving locales
            continue;
        }
        lang = QString("%1 (%2)").arg(lang).arg(country);
        ComboBoxLocale->addItem(lang, locale); // locale as userdata (for storing to config)
        if (locale == currentLocale) { // Set the currently selected locale
            ComboBoxLocale->setCurrentIndex(ComboBoxLocale->count() - 1);
            indexFlag = true;
        }
    }
    ComboBoxLocale->model()->sort(0); // Sort languages list

    ComboBoxLocale->insertItem(0, "System", ""); // System default locale - insert at the top
    if (!indexFlag) { // if selectedIndex didn't change - select system default
        ComboBoxLocale->setCurrentIndex(0);
    }
    connect(ComboBoxLocale, SIGNAL(activated(int)),
            this, SLOT(slotSetLocale(int)));

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
    QString cueDefault = m_pConfig->getValueString(ConfigKey("[Controls]", "CueDefault"), "0");
    int cueDefaultValue = cueDefault.toInt();

    // Update combo box
    // The itemData values are out of order to avoid breaking configurations
    // when Mixxx mode (no blinking) was introduced.
    ComboBoxCueDefault->addItem(tr("Mixxx mode"), 0);
    ComboBoxCueDefault->addItem(tr("Mixxx mode (no blinking)"), 4);
    ComboBoxCueDefault->addItem(tr("Pioneer mode"), 1);
    ComboBoxCueDefault->addItem(tr("Denon mode"), 2);
    ComboBoxCueDefault->addItem(tr("Numark mode"), 3);
    const int cueDefaultIndex = cueDefaultIndexByData(cueDefaultValue);
    ComboBoxCueDefault->setCurrentIndex(cueDefaultIndex);
    slotSetCueDefault(cueDefaultIndex);
    connect(ComboBoxCueDefault, SIGNAL(activated(int)), this, SLOT(slotSetCueDefault(int)));

    // Cue recall
    checkBoxSeekToCue->setChecked(m_pConfig->getValueString(
        ConfigKey("[Controls]", "CueRecall")).toInt()==0);
    //NOTE: for CueRecall, 0 means ON...
    connect(checkBoxSeekToCue, SIGNAL(toggled(bool)),
            this, SLOT(slotSetCueRecall(bool)));

    //
    // Skin configurations
    //
    QString warningString = "<img src=\":/images/preferences/ic_preferences_warning.png\") width=16 height=16 />"
        + tr("The minimum size of the selected skin is bigger than your screen resolution.");
    warningLabel->setText(warningString);

    ComboBoxSkinconf->clear();

    QList<QDir> skinSearchPaths = m_pSkinLoader->getSkinSearchPaths();
    QList<QFileInfo> skins;
    foreach (QDir dir, skinSearchPaths) {
        dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        skins.append(dir.entryInfoList());
    }

    QString configuredSkinPath = m_pSkinLoader->getSkinPath();
    QIcon sizeWarningIcon(":/images/preferences/ic_preferences_warning.png");
    int index = 0;
    foreach (QFileInfo skinInfo, skins) {
        bool size_ok = checkSkinResolution(skinInfo.absoluteFilePath());
        if (size_ok) {
            ComboBoxSkinconf->insertItem(index, skinInfo.fileName());
        } else {
            ComboBoxSkinconf->insertItem(index, sizeWarningIcon, skinInfo.fileName());
        }

        if (skinInfo.absoluteFilePath() == configuredSkinPath) {
            ComboBoxSkinconf->setCurrentIndex(index);
            if (size_ok) {
                warningLabel->hide();
            } else {
                warningLabel->show();
            }
        }
        index++;
    }

    connect(ComboBoxSkinconf, SIGNAL(activated(int)), this, SLOT(slotSetSkin(int)));
    connect(ComboBoxSchemeconf, SIGNAL(activated(int)), this, SLOT(slotSetScheme(int)));

    slotUpdateSchemes();

    //
    // Start in fullscreen mode
    //
    checkBoxStartFullScreen->setChecked(m_pConfig->getValueString(
                       ConfigKey("[Config]", "StartInFullscreen")).toInt()==1);
    connect(checkBoxStartFullScreen, SIGNAL(toggled(bool)),
            this, SLOT(slotSetStartInFullScreen(bool)));
    //
    // Tooltip configuration
    //

    // Initialize checkboxes to match config
    mixxx::TooltipsPreference configTooltips = m_mixxx->getToolTipsCfg();
    switch (configTooltips) {
        case mixxx::TooltipsPreference::TOOLTIPS_OFF:
            radioButtonTooltipsOff->setChecked(true);
            break;
        case mixxx::TooltipsPreference::TOOLTIPS_ON:
            radioButtonTooltipsLibraryAndSkin->setChecked(true);
            break;
        case mixxx::TooltipsPreference::TOOLTIPS_ONLY_IN_LIBRARY:
            radioButtonTooltipsLibrary->setChecked(true);
            break;
    }

    slotSetTooltips();  // Update disabled status of "only library" checkbox
    connect(buttonGroupTooltips, SIGNAL(buttonClicked(QAbstractButton*)),
            this, SLOT(slotSetTooltips()));

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
    int configSPAutoReset = m_pConfig->getValueString(
                    ConfigKey("[Controls]", "SpeedAutoReset"),
                    QString("%1").arg(BaseTrackPlayer::RESET_PITCH)).toInt();

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

DlgPrefControls::~DlgPrefControls() {
    delete m_pControlTrackTimeDisplay;
    qDeleteAll(m_rateControls);
    qDeleteAll(m_rateDirControls);
    qDeleteAll(m_cueControls);
    qDeleteAll(m_rateRangeControls);
    qDeleteAll(m_keylockModeControls);
}

void DlgPrefControls::slotUpdateSchemes() {
    // Since this involves opening a file we won't do this as part of regular slotUpdate
    QList<QString> schlist = LegacySkinParser::getSchemeList(
                m_pSkinLoader->getSkinPath());

    ComboBoxSchemeconf->clear();

    if (schlist.size() == 0) {
        ComboBoxSchemeconf->setEnabled(false);
        ComboBoxSchemeconf->addItem(tr("This skin does not support color schemes", 0));
        ComboBoxSchemeconf->setCurrentIndex(0);
    } else {
        ComboBoxSchemeconf->setEnabled(true);
        QString selectedScheme = m_pConfig->getValueString(ConfigKey("[Config]", "Scheme"));
        for (int i = 0; i < schlist.size(); i++) {
            ComboBoxSchemeconf->addItem(schlist[i]);

            if (schlist[i] == selectedScheme) {
                ComboBoxSchemeconf->setCurrentIndex(i);
            }
        }
    }
}

void DlgPrefControls::slotUpdate() {
    double deck1RateRange = m_rateRangeControls[0]->get();
    double deck1RateDir = m_rateDirControls[0]->get();

    int idx = ComboBoxRateRange->findData(static_cast<int>(deck1RateRange * 100));
    if (idx == -1) {
        ComboBoxRateRange->addItem(QString::number(deck1RateRange * 100.).append("%"),
                                   deck1RateRange * 100.);
    }

    ComboBoxRateRange->setCurrentIndex(idx);

    if (deck1RateDir == 1) {
        checkBoxInvertSpeedSlider->setChecked(false);
    } else {
        checkBoxInvertSpeedSlider->setChecked(true);
    }

    if (m_keylockMode == 1)
        radioButtonCurrentKey->setChecked(true);
    else
        radioButtonOriginalKey->setChecked(true);

    checkBoxResetSpeed->setChecked(m_speedAutoReset);
    checkBoxResetPitch->setChecked(m_pitchAutoReset);
}

void DlgPrefControls::slotResetToDefaults() {
    // Track time display mode
    radioButtonRemaining->setChecked(true);

    // Up increases speed.
    checkBoxInvertSpeedSlider->setChecked(false);

    // 8% Rate Range
    ComboBoxRateRange->setCurrentIndex(ComboBoxRateRange->findData(8));

    // Don't load tracks into playing decks.
    checkBoxDisallowLoadToPlayingDeck->setChecked(true);

    // Use System locale
    ComboBoxLocale->setCurrentIndex(0);

    // Mixxx cue mode
    ComboBoxCueDefault->setCurrentIndex(0);

    // Cue recall on.
    checkBoxSeekToCue->setChecked(true);

    // Don't start in full screen.
    checkBoxStartFullScreen->setChecked(false);

    // Tooltips on everywhere.
    radioButtonTooltipsLibraryAndSkin->setChecked(true);

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

    // Lock to original key
    m_keylockMode = 0;
    radioButtonOriginalKey->setChecked(true);
}

void DlgPrefControls::slotSetLocale(int pos) {
    QString newLocale = ComboBoxLocale->itemData(pos).toString();
    m_pConfig->set(ConfigKey("[Config]", "Locale"), ConfigValue(newLocale));
    notifyRebootNecessary();
}

void DlgPrefControls::slotSetRateRange(int pos) {
    slotSetRateRangePercent(ComboBoxRateRange->itemData(pos).toInt());
}


void DlgPrefControls::slotSetRateRangePercent (int rateRangePercent) {
    double rateRange = rateRangePercent / 100.;

    // Set rate range for every group
    foreach (ControlObjectSlave* pControl, m_rateRangeControls) {
        pControl->set(rateRange);
    }

    // Reset rate for every group
    foreach (ControlObjectSlave* pControl, m_rateControls) {
        pControl->set(0);
    }
}

void DlgPrefControls::slotSetRateDir(bool invert) {
    int index = 0;
    if (invert) index = 1;
    slotSetRateDir(index);
}

void DlgPrefControls::slotSetRateDir(int index) {
    float dir = 1.;
    if (index == 1)
        dir = -1.;
    float oldDir = m_rateDirControls[0]->get();

    // Set rate direction for every group
    foreach (ControlObjectSlave* pControl, m_rateDirControls) {
        pControl->set(dir);
    }

    // If the setting was changed, ie the old direction is not equal to the new one,
    // multiply the rate by -1 so the current sound does not change.
    if(fabs(dir - oldDir) > 0.1) {
        foreach (ControlObjectSlave* pControl, m_rateControls) {
            pControl->set(-1 * pControl->get());
        }
    }

}

void DlgPrefControls::slotKeyLockMode(QAbstractButton* b) {
    if (b == radioButtonCurrentKey) {
        m_keylockMode = 1;
    }
    else { m_keylockMode = 0; }
}

void DlgPrefControls::slotSetAllowTrackLoadToPlayingDeck(bool b) {
    // If b is true, it means NOT to allow track loading
    m_pConfig->set(ConfigKey("[Controls]", "AllowTrackLoadToPlayingDeck"),
                   ConfigValue(b?0:1));
}

void DlgPrefControls::slotSetCueDefault(int index)
{
    int cueMode = ComboBoxCueDefault->itemData(index).toInt();
    m_pConfig->set(ConfigKey("[Controls]", "CueDefault"), ConfigValue(cueMode));

    // Set cue behavior for every group
    foreach (ControlObjectSlave* pControl, m_cueControls) {
        pControl->set(cueMode);
    }
}

void DlgPrefControls::slotSetCueRecall(bool b)
{
    m_pConfig->set(ConfigKey("[Controls]", "CueRecall"), ConfigValue(b?0:1));
}

void DlgPrefControls::slotSetStartInFullScreen(bool b) {
    m_pConfig->set(ConfigKey("[Config]", "StartInFullscreen"), ConfigValue(b?1:0));
}

void DlgPrefControls::slotSetTooltips() {
    //0=OFF, 1=ON, 2=ON (only in Library)
    mixxx::TooltipsPreference valueToSet = mixxx::TooltipsPreference::TOOLTIPS_ON;
    if (radioButtonTooltipsOff->isChecked()) {
        valueToSet = mixxx::TooltipsPreference::TOOLTIPS_OFF;
    } else if (radioButtonTooltipsLibrary->isChecked()) {
        valueToSet = mixxx::TooltipsPreference::TOOLTIPS_ONLY_IN_LIBRARY;
    }
    m_mixxx->setToolTipsCfg(valueToSet);
}

void DlgPrefControls::notifyRebootNecessary() {
    // make the fact that you have to restart mixxx more obvious
    QMessageBox::information(
        this, tr("Information"),
        tr("Mixxx must be restarted before the changes will take effect."));
}

void DlgPrefControls::slotSetScheme(int) {
    m_pConfig->set(ConfigKey("[Config]", "Scheme"), ComboBoxSchemeconf->currentText());
    m_mixxx->rebootMixxxView();
}

void DlgPrefControls::slotSetSkin(int) {
    ComboBoxSkinconf->repaint(); // without it the combobox sticks to the old value until
                                 // the new Skin is fully loaded
    m_pConfig->set(ConfigKey("[Config]", "ResizableSkin"), ComboBoxSkinconf->currentText());
    m_mixxx->rebootMixxxView();
    checkSkinResolution(ComboBoxSkinconf->currentText())
            ? warningLabel->hide() : warningLabel->show();
    slotUpdateSchemes();
}

void DlgPrefControls::slotSetTrackTimeDisplay(QAbstractButton* b) {
    int timeDisplay = 0;
    if (b == radioButtonRemaining) {
        timeDisplay = 1;
        m_pConfig->set(ConfigKey("[Controls]","PositionDisplay"), ConfigValue(1));
    }
    else {
        m_pConfig->set(ConfigKey("[Controls]","PositionDisplay"), ConfigValue(0));
    }
    m_pControlTrackTimeDisplay->set(timeDisplay);
}

void DlgPrefControls::slotSetTrackTimeDisplay(double v) {
    if (v > 0) {
        // Remaining
        radioButtonRemaining->setChecked(true);
        m_pConfig->set(ConfigKey("[Controls]", "PositionDisplay"), ConfigValue(1));
    } else {
        // Elapsed
        radioButtonElapsed->setChecked(true);
        m_pConfig->set(ConfigKey("[Controls]", "PositionDisplay"), ConfigValue(0));
    }
}

void DlgPrefControls::slotSetRateTempLeft(double v) {
    QString str;
    str = str.setNum(v, 'f');
    m_pConfig->set(ConfigKey("[Controls]", "RateTempLeft"),ConfigValue(str));
    RateControl::setTemp(v);
}

void DlgPrefControls::slotSetRateTempRight(double v) {
    QString str;
    str = str.setNum(v, 'f');
    m_pConfig->set(ConfigKey("[Controls]", "RateTempRight"),ConfigValue(str));
    RateControl::setTempSmall(v);
}

void DlgPrefControls::slotSetRatePermLeft(double v) {
    QString str;
    str = str.setNum(v, 'f');
    m_pConfig->set(ConfigKey("[Controls]", "RatePermLeft"),ConfigValue(str));
    RateControl::setPerm(v);
}

void DlgPrefControls::slotSetRatePermRight(double v) {
    QString str;
    str = str.setNum(v, 'f');
    m_pConfig->set(ConfigKey("[Controls]", "RatePermRight"),ConfigValue(str));
    RateControl::setPermSmall(v);
}

void DlgPrefControls::slotSetRateRampSensitivity(int sense) {
    m_pConfig->set(ConfigKey("[Controls]", "RateRampSensitivity"),
                   ConfigValue(SliderRateRampSensitivity->value()));
    RateControl::setRateRampSensitivity(sense);
}

void DlgPrefControls::slotSetRateRamp(bool mode) {
    m_pConfig->set(ConfigKey("[Controls]", "RateRamp"),
                   ConfigValue(radioButtonSpeedBendRamping->isChecked()));
    RateControl::setRateRamp(mode);
}

void DlgPrefControls::slotApply() {
    double deck1RateRange = m_rateRangeControls[0]->get();
    double deck1RateDir = m_rateDirControls[0]->get();

    m_pConfig->set(ConfigKey("[Controls]", "RateRangePercent"),
                   ConfigValue(static_cast<int>(deck1RateRange * 100)));

    // Write rate direction to config file
    if (deck1RateDir == 1) {
        m_pConfig->set(ConfigKey("[Controls]", "RateDir"), ConfigValue(0));
    } else {
        m_pConfig->set(ConfigKey("[Controls]", "RateDir"), ConfigValue(1));
    }

    int configSPAutoReset = BaseTrackPlayer::RESET_NONE;

    if (m_speedAutoReset && m_pitchAutoReset) {
        configSPAutoReset = BaseTrackPlayer::RESET_PITCH_AND_SPEED;
    }
    else if (m_speedAutoReset) configSPAutoReset = BaseTrackPlayer::RESET_SPEED;
    else if (m_pitchAutoReset) configSPAutoReset = BaseTrackPlayer::RESET_PITCH;

    m_pConfig->set(ConfigKey("[Controls]", "SpeedAutoReset"),
                   ConfigValue(configSPAutoReset));

    m_pConfig->set(ConfigKey("[Controls]", "keylockMode"),
            ConfigValue(m_keylockMode));
    // Set key lock behavior for every group
    foreach (ControlObjectSlave* pControl, m_keylockModeControls) {
        pControl->set(m_keylockMode);
    }
}

//Returns TRUE if skin fits to screen resolution, FALSE otherwise
bool DlgPrefControls::checkSkinResolution(QString skin)
{
    int screenWidth = QApplication::desktop()->width();
    int screenHeight = QApplication::desktop()->height();

    const QRegExp min_size_regex("<MinimumSize>(\\d+), *(\\d+)<");
    QFile skinfile(skin + "/skin.xml");
    if (skinfile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream in(&skinfile);
        bool found_size = false;
        while (!in.atEnd()) {
            if (min_size_regex.indexIn(in.readLine()) != -1) {
                found_size = true;
                break;
            }
        }
        if (found_size) {
            return !(min_size_regex.cap(1).toInt() > screenWidth ||
                     min_size_regex.cap(2).toInt() > screenHeight);
        }
    }

    // If regex failed, fall back to skin name parsing.
    QString skinName = skin.left(skin.indexOf(QRegExp("\\d")));
    QString resName = skin.right(skin.count()-skinName.count());
    QString res = resName.left(resName.lastIndexOf(QRegExp("\\d"))+1);
    QString skinWidth = res.left(res.indexOf("x"));
    QString skinHeight = res.right(res.count()-skinWidth.count()-1);
    return !(skinWidth.toInt() > screenWidth || skinHeight.toInt() > screenHeight);
}

void DlgPrefControls::slotNumDecksChanged(double new_count) {
    int numdecks = static_cast<int>(new_count);
    if (numdecks <= m_iNumConfiguredDecks) {
        // TODO(owilliams): If we implement deck deletion, shrink the size of configured decks.
        return;
    }

    for (int i = m_iNumConfiguredDecks; i < numdecks; ++i) {
        QString group = PlayerManager::groupForDeck(i);
        m_rateControls.push_back(new ControlObjectSlave(
                group, "rate"));
        m_rateRangeControls.push_back(new ControlObjectSlave(
                group, "rateRange"));
        m_rateDirControls.push_back(new ControlObjectSlave(
                group, "rate_dir"));
        m_cueControls.push_back(new ControlObjectSlave(
                group, "cue_mode"));
        m_keylockModeControls.push_back(new ControlObjectSlave(
                        group, "keylockMode"));
        m_keylockModeControls.last()->set(m_keylockMode);
    }

    m_iNumConfiguredDecks = numdecks;
    slotSetRateDir(m_pConfig->getValueString(ConfigKey("[Controls]", "RateDir")).toInt());
    slotSetRateRangePercent(m_pConfig->getValueString(ConfigKey("[Controls]", "RateRangePercent")).toInt());
}

void DlgPrefControls::slotNumSamplersChanged(double new_count) {
    int numsamplers = static_cast<int>(new_count);
    if (numsamplers <= m_iNumConfiguredSamplers) {
        return;
    }

    for (int i = m_iNumConfiguredSamplers; i < numsamplers; ++i) {
        QString group = PlayerManager::groupForSampler(i);
        m_rateControls.push_back(new ControlObjectSlave(
                group, "rate"));
        m_rateRangeControls.push_back(new ControlObjectSlave(
                group, "rateRange"));
        m_rateDirControls.push_back(new ControlObjectSlave(
                group, "rate_dir"));
        m_cueControls.push_back(new ControlObjectSlave(
                group, "cue_mode"));
        m_keylockModeControls.push_back(new ControlObjectSlave(
                        group, "keylockMode"));
        m_keylockModeControls.last()->set(m_keylockMode);
    }

    m_iNumConfiguredSamplers = numsamplers;
    slotSetRateDir(m_pConfig->getValueString(ConfigKey("[Controls]", "RateDir")).toInt());
    slotSetRateRangePercent(m_pConfig->getValueString(ConfigKey("[Controls]", "RateRangePercent")).toInt());
}

void DlgPrefControls::slotUpdateSpeedAutoReset(bool b) {
    m_speedAutoReset = b;
}

void DlgPrefControls::slotUpdatePitchAutoReset(bool b) {
    m_pitchAutoReset = b;
}

int DlgPrefControls::cueDefaultIndexByData(int userData) const {
    for (int i = 0; i < ComboBoxCueDefault->count(); ++i) {
        if (ComboBoxCueDefault->itemData(i).toInt() == userData) {
            return i;
        }
    }
    qWarning() << "No default cue behavior found for value" << userData
               << "returning default";
    return 0;
}
