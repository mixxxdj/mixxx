/**
 * @file dlgprefnewsound.cpp
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
#include "dlgprefnewsound.h"
#include "dlgprefnewsounditem.h"
#include "soundmanager.h"
#include "sounddevice.h"

DlgPrefNewSound::DlgPrefNewSound(QWidget *parent, SoundManager *soundManager,
        ConfigObject<ConfigValue> *config)
    : QWidget(parent)
    , m_pSoundManager(soundManager)
    , m_pConfig(config)
    , m_settingsModified(false)
    , m_api(soundManager->getHostAPI()) {
    setupUi(this);

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
    foreach (QString srate, m_pSoundManager->getSamplerateList()) {
        sampleRateComboBox->addItem(QString("%1 Hz").arg(srate), srate);
    }
    sampleRateComboBox->setCurrentIndex(0);
    updateLatencies(0); // take this away when the config stuff is implemented
    connect(sampleRateComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateLatencies(int)));
    connect(sampleRateComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(settingChanged()));

    initializePaths();
//    loadSettings();

    connect(apiComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(settingChanged()));
    connect(sampleRateComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(settingChanged()));
    connect(latencyComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(settingChanged()));
}

DlgPrefNewSound::~DlgPrefNewSound() {

}

void DlgPrefNewSound::slotUpdate() {
    // have to do this stupid dance because the old sound sound pane
    // resets stuff every chance it gets and breaks our pointers to
    // sound devices -- bkgood
    // ought to be deleted later
    apiComboBox->setCurrentIndex(0);
    apiComboBox->setCurrentIndex(1);
}

void DlgPrefNewSound::slotApply() {
    if (!m_settingsModified) {
        return;
    }
    float sampleRate = sampleRateComboBox->itemData(sampleRateComboBox->currentIndex()).toFloat();
    unsigned int framesPerBuffer = latencyComboBox->itemData(latencyComboBox->currentIndex()).toUInt();
    m_pSoundManager->setHostAPI(m_api);
    m_pSoundManager->setSampleRate(sampleRate);
    m_pSoundManager->setFramesPerBuffer(framesPerBuffer);
    // add paths to soundmanager
    // commit changes
    m_settingsModified = false;
    applyButton->setEnabled(false);
}

void DlgPrefNewSound::initializePaths() {
    foreach (AudioPath::AudioPathType type, AudioSource::getSupportedTypes()) {
        DlgPrefNewSoundItem *toInsert;
        if (AudioPath::isIndexable(type)) {
            for (unsigned int i = 0; i < NUM_DECKS; ++i) {
                toInsert = new DlgPrefNewSoundItem(outputScrollAreaContents, type,
                        m_outputDevices, false, i);
                connect(this, SIGNAL(refreshOutputDevices(QList<SoundDevice*>&)),
                        toInsert, SLOT(refreshDevices(QList<SoundDevice*>&)));
                connect(toInsert, SIGNAL(settingChanged()),
                        this, SLOT(settingChanged()));
                outputVLayout->insertWidget(outputVLayout->count() - 1, toInsert);
            }
        } else {
            toInsert = new DlgPrefNewSoundItem(outputScrollAreaContents, type,
                m_outputDevices, false);
            connect(this, SIGNAL(refreshOutputDevices(QList<SoundDevice*>&)),
                    toInsert, SLOT(refreshDevices(QList<SoundDevice*>&)));
            connect(toInsert, SIGNAL(settingChanged()),
                    this, SLOT(settingChanged()));
            outputVLayout->insertWidget(outputVLayout->count() - 1, toInsert);
        }
    }
    foreach (AudioPath::AudioPathType type, AudioReceiver::getSupportedTypes()) {
        DlgPrefNewSoundItem *toInsert;
        if (AudioPath::isIndexable(type)) {
            for (unsigned int i = 0; i < NUM_DECKS; ++i) {
                toInsert = new DlgPrefNewSoundItem(inputScrollAreaContents, type,
                        m_inputDevices, true, i);
                connect(this, SIGNAL(refreshInputDevices(QList<SoundDevice*>&)),
                        toInsert, SLOT(refreshDevices(QList<SoundDevice*>&)));
                connect(toInsert, SIGNAL(settingChanged()),
                        this, SLOT(settingChanged()));
                inputVLayout->insertWidget(inputVLayout->count() - 1, toInsert);
            }
        } else {
            toInsert = new DlgPrefNewSoundItem(inputScrollAreaContents, type,
                m_inputDevices, true);
            connect(this, SIGNAL(refreshInputDevices(QList<SoundDevice*>&)),
                    toInsert, SLOT(refreshDevices(QList<SoundDevice*>&)));
            connect(toInsert, SIGNAL(settingChanged()),
                    this, SLOT(settingChanged()));
            inputVLayout->insertWidget(inputVLayout->count() - 1, toInsert);
        }
    }
}

void DlgPrefNewSound::apiChanged(int index) {
    QString selected = apiComboBox->itemData(index).toString();
    if (selected == "None") {
        m_outputDevices.clear();
        m_inputDevices.clear();
    } else {
        m_api = selected;
        m_outputDevices =
            m_pSoundManager->getDeviceList(m_api, true, false);
        m_inputDevices =
            m_pSoundManager->getDeviceList(m_api, false, true);
    }
    emit(refreshOutputDevices(m_outputDevices));
    emit(refreshInputDevices(m_inputDevices));
    // JACK sets its own latency
    if (m_api == MIXXX_PORTAUDIO_JACK_STRING) {
        latencyLabel->setEnabled(false);
        latencyComboBox->setEnabled(false);
    } else {
        latencyLabel->setEnabled(true);
        latencyComboBox->setEnabled(true);
    }
}

void DlgPrefNewSound::updateLatencies(int sampleRateIndex) {
    float sampleRate = sampleRateComboBox->itemData(sampleRateIndex).toFloat();
    if (sampleRate == 0.0f) {
        sampleRateComboBox->setCurrentIndex(0); // hope this doesn't recurse!
        return;
    }
    unsigned int framesPerBuffer = 1; // start this at 0 and inf loop happens
    // we don't want to display any sub-1ms latencies (well maybe we do but I
    // don't know if current PC could handle it), so we iterate over all the
    // buffer sizes until we find the first that gives us a latency >= 1 ms
    // -- bkgood
    for (; framesPerBuffer / sampleRate * 1000 < 1.0f; framesPerBuffer *= 2);
    latencyComboBox->clear();
    for (unsigned int i = 0; i < LATENCY_COUNT; ++i) {
        unsigned int latency = framesPerBuffer / sampleRate * 1000;
        latencyComboBox->addItem(QString("%1 ms").arg(latency), framesPerBuffer);
        framesPerBuffer *= 2;
    }
    // set it to the max, let the user dig if they need better latency. better
    // than having a user get the pops on first use and thinking poorly of mixxx
    // because of it -- bkgood
    latencyComboBox->setCurrentIndex(latencyComboBox->count() - 1);
}

void DlgPrefNewSound::settingChanged() {
    m_settingsModified = true;
    if (!applyButton->isEnabled()) {
        applyButton->setEnabled(true);
    }
}
