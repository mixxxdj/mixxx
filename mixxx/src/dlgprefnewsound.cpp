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
    apiComboBox->setCurrentIndex(0); // this needs to be deleted when the config load method comes
    initializePaths();
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
    // ought to be deleted
    apiComboBox->setCurrentIndex(0);
    apiComboBox->setCurrentIndex(1);
}

void DlgPrefNewSound::slotApply() {
    if (!m_settingsModified) {
        return;
    }
    // set api, srate, latency
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
}

void DlgPrefNewSound::settingChanged() {
    m_settingsModified = true;
    if (!applyButton->isEnabled()) {
        applyButton->setEnabled(true);
    }
}
