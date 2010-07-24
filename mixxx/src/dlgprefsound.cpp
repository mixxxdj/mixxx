/**
 * @file dlgprefsound.cpp
 * @author Bill Good <bkgood at gmail dot com>
 * @date 20100625
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDebug>
#include "dlgprefsound.h"
#include "dlgprefsounditem.h"
#include "soundmanager.h"
#include "sounddevice.h"

/**
 * Construct a new sound preferences pane. Initializes and populates all the
 * all the controls to the values obtained from SoundManager.
 */
DlgPrefSound::DlgPrefSound(QWidget *parent, SoundManager *soundManager,
        ConfigObject<ConfigValue> *config)
    : QWidget(parent)
    , m_pSoundManager(soundManager)
    , m_pConfig(config)
    , m_settingsModified(false)
    , m_loading(false) {
    setupUi(this);

    connect(m_pSoundManager, SIGNAL(devicesUpdated()),
            this, SLOT(refreshDevices()));

    applyButton->setEnabled(false);
    connect(applyButton, SIGNAL(clicked()),
            this, SLOT(slotApply()));

    apiComboBox->clear();
    apiComboBox->addItem("None", "None");
    foreach (QString api, m_pSoundManager->getHostAPIList()) {
        apiComboBox->addItem(api, api);
    }
    connect(apiComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(apiChanged(int)));

    sampleRateComboBox->clear();
    foreach (unsigned int srate, m_pSoundManager->getSampleRates()) {
        sampleRateComboBox->addItem(QString("%1 Hz").arg(srate), srate);
    }
    connect(sampleRateComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(sampleRateChanged(int)));
    connect(sampleRateComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateLatencies(int)));
    connect(latencyComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(latencyChanged(int)));

    initializePaths();
    loadSettings();

    connect(apiComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(settingChanged()));
    connect(sampleRateComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(settingChanged()));
    connect(latencyComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(settingChanged()));
}

DlgPrefSound::~DlgPrefSound() {

}

/**
 * Slot called when the preferences dialog is opened.
 */
void DlgPrefSound::slotUpdate() {
    loadSettings();
}

/**
 * Slot called when the Apply or OK button is pressed.
 */
void DlgPrefSound::slotApply() {
    if (!m_settingsModified) {
        return;
    }
    m_config.clearInputs();
    m_config.clearOutputs();
    emit(writePaths(&m_config));
    m_pSoundManager->setConfig(m_config); // setConfig will call setupDevices if necessary
    m_settingsModified = false;
    applyButton->setEnabled(false);
}

/**
 * Initializes (and creates) all the path items. Each path item widget allows
 * the user to input a sound device name and channel number given a description
 * of what will be done with that info. Inputs and outputs are grouped by tab,
 * and each path item has an identifier (Master, Headphones, ...) and an index,
 * if necessary.
 */
void DlgPrefSound::initializePaths() {
    QList<DlgPrefSoundItem*> items;
    foreach (AudioPathType type, AudioOutput::getSupportedTypes()) {
        DlgPrefSoundItem *toInsert;
        if (AudioPath::isIndexed(type)) {
            for (unsigned int i = 0; i < NUM_DECKS; ++i) {
                toInsert = new DlgPrefSoundItem(outputScrollAreaContents, type,
                        m_outputDevices, false, i);
                connect(this, SIGNAL(refreshOutputDevices(const QList<SoundDevice*>&)),
                        toInsert, SLOT(refreshDevices(const QList<SoundDevice*>&)));
                outputVLayout->insertWidget(outputVLayout->count() - 1, toInsert);
                items.append(toInsert);
            }
        } else {
            toInsert = new DlgPrefSoundItem(outputScrollAreaContents, type,
                m_outputDevices, false);
            connect(this, SIGNAL(refreshOutputDevices(const QList<SoundDevice*>&)),
                    toInsert, SLOT(refreshDevices(const QList<SoundDevice*>&)));
            outputVLayout->insertWidget(outputVLayout->count() - 1, toInsert);
            items.append(toInsert);
        }
    }
    foreach (AudioPathType type, AudioInput::getSupportedTypes()) {
        DlgPrefSoundItem *toInsert;
        if (AudioPath::isIndexed(type)) {
            for (unsigned int i = 0; i < NUM_DECKS; ++i) {
                toInsert = new DlgPrefSoundItem(inputScrollAreaContents, type,
                        m_inputDevices, true, i);
                connect(this, SIGNAL(refreshInputDevices(const QList<SoundDevice*>&)),
                        toInsert, SLOT(refreshDevices(const QList<SoundDevice*>&)));
                inputVLayout->insertWidget(inputVLayout->count() - 1, toInsert);
                items.append(toInsert);
            }
        } else {
            toInsert = new DlgPrefSoundItem(inputScrollAreaContents, type,
                m_inputDevices, true);
            connect(this, SIGNAL(refreshInputDevices(const QList<SoundDevice*>&)),
                    toInsert, SLOT(refreshDevices(const QList<SoundDevice*>&)));
            inputVLayout->insertWidget(inputVLayout->count() - 1, toInsert);
            items.append(toInsert);
        }
    }
    foreach (DlgPrefSoundItem *item, items) {
        connect(item, SIGNAL(settingChanged()),
                this, SLOT(settingChanged()));
        connect(this, SIGNAL(loadPaths(const SoundManagerConfig&)),
                item, SLOT(loadPath(const SoundManagerConfig&)));
        connect(this, SIGNAL(writePaths(SoundManagerConfig*)),
                item, SLOT(writePath(SoundManagerConfig*)));
    }
}

/**
 *
 */
void DlgPrefSound::loadSettings() {
    m_loading = true; // so settingsChanged ignores all our modifications here
    m_config = m_pSoundManager->getConfig();
    int apiIndex = apiComboBox->findData(m_config.getAPI());
    if (apiIndex != -1) {
        apiComboBox->setCurrentIndex(apiIndex);
    }
    int sampleRateIndex = sampleRateComboBox->findData(m_config.getSampleRate());
    if (sampleRateIndex != -1) {
        sampleRateComboBox->setCurrentIndex(sampleRateIndex);
    }
    int latencyIndex = latencyComboBox->findData(m_config.getLatency());
    if (latencyIndex != -1) {
        latencyComboBox->setCurrentIndex(latencyIndex);
    }
    emit(loadPaths(m_config));
    m_loading = false;
}

/**
 * Slots called when the user selects a different API, or the
 * software changes it programatically (for instance, when it
 * loads a value from SoundManager). Refreshes the device lists
 * for the new API and pushes those to the path items.
 */
void DlgPrefSound::apiChanged(int index) {
    m_config.setAPI(apiComboBox->itemData(index).toString());
    refreshDevices();
    // JACK sets its own latency
    if (m_config.getAPI() == MIXXX_PORTAUDIO_JACK_STRING) {
        latencyLabel->setEnabled(false);
        latencyComboBox->setEnabled(false);
    } else {
        latencyLabel->setEnabled(true);
        latencyComboBox->setEnabled(true);
    }
}

/**
 * Slot called when the sample rate combo box changes to update the
 * sample rate in the config.
 */
void DlgPrefSound::sampleRateChanged(int index) {
    m_config.setSampleRate(
            sampleRateComboBox->itemData(index).toUInt());
}

/**
 * Slot called when the latency combo box is changed to update the
 * latency in the config.
 */
void DlgPrefSound::latencyChanged(int index) {
    m_config.setLatency(
            latencyComboBox->itemData(index).toUInt());
}

/**
 * Slot called whenever the selected sample rate is changed. Populates the
 * latency input box with MAX_LATENCY values, starting at 1ms, representing
 * a number of frames per buffer, which will always be a power of 2 (so the
 * values displayed in ms won't be constant between sample rates, but they'll
 * be close).
 */
void DlgPrefSound::updateLatencies(int sampleRateIndex) {
    double sampleRate = sampleRateComboBox->itemData(sampleRateIndex).toDouble();
    if (sampleRate == 0.0) {
        sampleRateComboBox->setCurrentIndex(0); // hope this doesn't recurse!
        return;
    }
    unsigned int framesPerBuffer = 1; // start this at 0 and inf loop happens
    // we don't want to display any sub-1ms latencies (well maybe we do but I
    // don't right now!), so we iterate over all the buffer sizes until we
    // find the first that gives us a latency >= 1 ms -- bkgood
    for (; framesPerBuffer / sampleRate * 1000 < 1.0; framesPerBuffer *= 2);
    latencyComboBox->clear();
    for (unsigned int i = 0; i < MAX_LATENCY; ++i) {
        unsigned int latency = framesPerBuffer / sampleRate * 1000;
        latencyComboBox->addItem(QString("%1 ms").arg(latency), framesPerBuffer);
        framesPerBuffer <<= 1; // *= 2
    }
    // set it to the max, let the user dig if they need better latency. better
    // than having a user get the pops on first use and thinking poorly of mixxx
    // because of it -- bkgood
    latencyComboBox->setCurrentIndex(latencyComboBox->count() - 1);
}

/**
 * Slot called when device pointers go bad to refresh them all, or the API
 * just changes and we need to display new devices.
 */
void DlgPrefSound::refreshDevices() {
    if (m_config.getAPI() == "None") {
        m_outputDevices.clear();
        m_inputDevices.clear();
    } else {
        m_outputDevices =
            m_pSoundManager->getDeviceList(m_config.getAPI(), true, false);
        m_inputDevices =
            m_pSoundManager->getDeviceList(m_config.getAPI(), false, true);
    }
    emit(refreshOutputDevices(m_outputDevices));
    emit(refreshInputDevices(m_inputDevices));
}

/**
 * Called when any of the combo boxes in this dialog are changed. Enables the
 * apply button and marks that settings have been changed so that slotApply
 * knows to apply them.
 */
void DlgPrefSound::settingChanged() {
    if (m_loading) return; // doesn't count if we're just loading prefs
    m_settingsModified = true;
    if (!applyButton->isEnabled()) {
        applyButton->setEnabled(true);
    }
}
