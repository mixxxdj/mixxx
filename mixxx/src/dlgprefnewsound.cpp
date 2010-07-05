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
    , m_settingsModified(false) {
    setupUi(this);
    apiComboBox->clear();
    apiComboBox->addItem("None", "None");
    foreach (QString api, m_pSoundManager->getHostAPIList()) {
        apiComboBox->addItem(api, api);
    }
    connect(apiComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(apiChanged(int)));
    apiComboBox->setCurrentIndex(1);
    initializePaths();
}

DlgPrefNewSound::~DlgPrefNewSound() {

}

void DlgPrefNewSound::slotUpdate() {
    // have to do this stupid chance because the old sound sound pane
    // resets stuff every chance it gets and breaks our pointers to 
    // sound devices -- bkgood 
    apiComboBox->setCurrentIndex(0);
    apiComboBox->setCurrentIndex(1);
}

void DlgPrefNewSound::slotApply() {
    if (!m_settingsModified) {
        return;
    }
}

void DlgPrefNewSound::initializePaths() {
    foreach (AudioPath::AudioPathType type, AudioSource::getSupportedTypes()) {
        DlgPrefNewSoundItem *toInsert;
        if (AudioPath::isIndexable(type)) {
            for (unsigned int i = 1; i <= NUM_DECKS; ++i) {
                QString typeLabel = QString("%1 %2")
                    .arg(AudioPath::getStringFromType(type))
                    .arg(i);
                toInsert = new DlgPrefNewSoundItem(outputScrollAreaContents, typeLabel,
                        m_outputDevices, false);
                connect(this, SIGNAL(refreshOutputDevices(QList<SoundDevice*>&)),
                        toInsert, SLOT(refreshDevices(QList<SoundDevice*>&)));
                outputVLayout->insertWidget(outputVLayout->count() - 1, toInsert);
            }
        } else {
            QString typeLabel = AudioPath::getStringFromType(type);
            toInsert = new DlgPrefNewSoundItem(outputScrollAreaContents, typeLabel,
                m_outputDevices, false);
            connect(this, SIGNAL(refreshOutputDevices(QList<SoundDevice*>&)),
                    toInsert, SLOT(refreshDevices(QList<SoundDevice*>&)));
            outputVLayout->insertWidget(outputVLayout->count() - 1, toInsert);
        }
    }
    foreach (AudioPath::AudioPathType type, AudioReceiver::getSupportedTypes()) {
        DlgPrefNewSoundItem *toInsert;
        if (AudioPath::isIndexable(type)) {
            for (unsigned int i = 1; i <= NUM_DECKS; ++i) {
                QString typeLabel = QString("%1 %2")
                    .arg(AudioPath::getStringFromType(type))
                    .arg(i);
                toInsert = new DlgPrefNewSoundItem(inputScrollAreaContents, typeLabel,
                        m_inputDevices, true);
                connect(this, SIGNAL(refreshInputDevices(QList<SoundDevice*>&)),
                        toInsert, SLOT(refreshDevices(QList<SoundDevice*>&)));
                inputVLayout->insertWidget(inputVLayout->count() - 1, toInsert);
            }
        } else {
            QString typeLabel = AudioPath::getStringFromType(type);
            toInsert = new DlgPrefNewSoundItem(inputScrollAreaContents, typeLabel,
                m_inputDevices, true);
            connect(this, SIGNAL(refreshInputDevices(QList<SoundDevice*>&)),
                    toInsert, SLOT(refreshDevices(QList<SoundDevice*>&)));
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
        m_pSoundManager->setHostAPI(selected);
        m_outputDevices =
            m_pSoundManager->getDeviceList(m_pSoundManager->getHostAPI(), true, false);
        m_inputDevices =
            m_pSoundManager->getDeviceList(m_pSoundManager->getHostAPI(), false, true);
    }
    emit(refreshOutputDevices(m_outputDevices));
    emit(refreshInputDevices(m_inputDevices));
}
