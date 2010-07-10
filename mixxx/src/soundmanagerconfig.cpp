/**
 * @file soundmanagerconfig.cpp
 * @author Bill Good <bkgood at gmail dot com>
 * @date 20100709
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "soundmanagerconfig.h"
#include "sounddevice.h"
#include "soundmanager.h"

SoundManagerConfig::SoundManagerConfig(SoundManager *soundManager, QFileInfo configFile)
    : m_soundManager(soundManager)
    , m_configFile(configFile) {

}

SoundManagerConfig::~SoundManagerConfig() {
    // TODO ??? (do I want to call writeToDisk here?)
}

bool SoundManagerConfig::readFromDisk() {
    // TODO read from XML file
    return false;
}

bool SoundManagerConfig::writeToDisk() const {
    // TODO write to XML file
    return false;
}

QString SoundManagerConfig::getAPI() const {
    return m_api;
}

void SoundManagerConfig::setAPI(QString api) {
    // XXX should I check input here?
    m_api = api;
}

unsigned int SoundManagerConfig::getSampleRate() const {
    return m_sampleRate;
}

void SoundManagerConfig::setSampleRate(unsigned int sampleRate) {
    // XXX should I check input here?
    m_sampleRate = sampleRate;
}

unsigned int SoundManagerConfig::getLatency() const {
    return m_latency;
}

void SoundManagerConfig::setLatency(unsigned int latency) {
    if (latency == 0) {
        // this shouldn't be zero, set it to one for fun
        ++latency;
    }
    m_latency = latency;
}

void SoundManagerConfig::addSource(SoundDevice *device, AudioSource source) {
    QPair<SoundDevice*, AudioSource> toAppend(device, source);
    if (!m_sources.contains(toAppend)) {
        m_sources.append(toAppend);
    }
}

void SoundManagerConfig::addReceiver(SoundDevice *device, AudioReceiver receiver) {
    QPair<SoundDevice*, AudioReceiver> toAppend(device, receiver);
    if (!m_receivers.contains(toAppend)) {
        m_receivers.append(toAppend);
    }
}

void SoundManagerConfig::clearSources() {
    m_sources.clear();
}

void SoundManagerConfig::clearReceivers() {
    m_receivers.clear();
}

void SoundManagerConfig::loadDefaults() {
    QList<QString> apiList = m_soundManager->getHostAPIList();
    if (!apiList.isEmpty()) {
#ifdef __LINUX__
        //Check for JACK and use that if it's available, otherwise use ALSA
        if (apiList.contains(MIXXX_PORTAUDIO_JACK_STRING)) {
            m_api = MIXXX_PORTAUDIO_JACK_STRING;
        } else {
            m_api = MIXXX_PORTAUDIO_ALSA_STRING;
        }
#endif
#ifdef __WINDOWS__
//Existence of ASIO doesn't necessarily mean you've got ASIO devices
//Do something more advanced one day if you like - Adam
        m_api = MIXXX_PORTAUDIO_DIRECTSOUND_STRING;
#endif
#ifdef __APPLE__
        m_api = MIXXX_PORTAUDIO_COREAUDIO_STRING;
#endif
    }
    QList<unsigned int> sampleRates = m_soundManager->getSampleRates();
    if (sampleRates.contains(DEFAULT_SAMPLE_RATE)) {
        m_sampleRate = DEFAULT_SAMPLE_RATE;
    } else if (!sampleRates.isEmpty()) {
        m_sampleRate = sampleRates.first();
    } else {
        qDebug() << "got empty sample rate list from SoundManager, this is a bug";
        Q_ASSERT(false);
    }
    m_latency = DEFAULT_LATENCY;

    m_sources.clear();
    m_receivers.clear();
    QList<SoundDevice*> outputDevices = m_soundManager->getDeviceList(m_api, true, false);
    if (!outputDevices.isEmpty() && outputDevices.first()->getNumOutputChannels() > 1) {
        SoundDevice *masterDevice = outputDevices.first();
        AudioSource masterSource(AudioPath::MASTER, 0);
        addSource(masterDevice, masterSource);
    }
}
