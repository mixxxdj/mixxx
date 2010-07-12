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

SoundManagerConfig::SoundManagerConfig() {
    m_configFile = QFileInfo(QString("%1/%2/%3")
            .arg(QDir::homePath())
            .arg(SETTINGS_PATH)
            .arg(SOUNDMANAGERCONFIG_FILENAME));
}

SoundManagerConfig::~SoundManagerConfig() {
    // don't write to disk here, it's SoundManager's responsibility
    // to save its own configuration -- bkgood
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
    // SoundManagerConfig doesn't necessarily have access to a SoundManager
    // instance, so I can't check for input validity here -- bkgood
    return m_api;
}

void SoundManagerConfig::setAPI(QString api) {
    m_api = api;
}

unsigned int SoundManagerConfig::getSampleRate() const {
    return m_sampleRate;
}

void SoundManagerConfig::setSampleRate(unsigned int sampleRate) {
    // SoundManagerConfig doesn't necessarily have access to a SoundManager
    // instance, so I can't check for input validity here -- bkgood
    m_sampleRate = sampleRate;
}

unsigned int SoundManagerConfig::getLatency() const {
    return m_latency;
}

/**
 * Set the latency value.
 * @warning This IS NOT a value in milliseconds, or a number of frames per
 * buffer. It is an index, where 1 is the first power-of-two buffer size (in
 * frames) which corresponds to a latency greater than or equal to 1 ms, 2 is
 * the second, etc. This is so that latency values are roughly equivalent
 * between different sample rates.
 */
void SoundManagerConfig::setLatency(unsigned int latency) {
    if (latency == 0) {
        // this shouldn't be zero, set it to one for fun
        ++latency;
    } else if (latency > MAX_LATENCY) {
        latency = MAX_LATENCY;
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

QList<QPair<SoundDevice*, AudioSource> > SoundManagerConfig::getSources() const {
    return m_sources;
}

QList<QPair<SoundDevice*, AudioReceiver> > SoundManagerConfig::getReceivers() const {
    return m_receivers;
}

void SoundManagerConfig::clearSources() {
    m_sources.clear();
}

void SoundManagerConfig::clearReceivers() {
    m_receivers.clear();
}

/**
 * Loads default values for API, master output, sample rate and/or latency.
 * @param soundManager pointer to SoundManager instance to load data from
 * @param flags Bitfield to determine which defaults to load, use something
 *              like SoundManagerConfig::API | SoundManagerConfig::DEVICES to
 *              load default API and master device.
 */
void SoundManagerConfig::loadDefaults(SoundManager *soundManager, int flags) {
    if (flags & SoundManagerConfig::API) {
        QList<QString> apiList = soundManager->getHostAPIList();
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
    }
    if (flags & SoundManagerConfig::DEVICES) {
        m_sources.clear();
        m_receivers.clear();
        QList<SoundDevice*> outputDevices = soundManager->getDeviceList(m_api, true, false);
        if (!outputDevices.isEmpty() && outputDevices.first()->getNumOutputChannels() > 1) {
            SoundDevice *masterDevice = outputDevices.first();
            AudioSource masterSource(AudioPath::MASTER, 0);
            addSource(masterDevice, masterSource);
        }
    }
    if (flags & SoundManagerConfig::OTHER) {
        QList<unsigned int> sampleRates = soundManager->getSampleRates();
        if (sampleRates.contains(DEFAULT_SAMPLE_RATE)) {
            m_sampleRate = DEFAULT_SAMPLE_RATE;
        } else if (!sampleRates.isEmpty()) {
            m_sampleRate = sampleRates.first();
        } else {
            qDebug() << "got empty sample rate list from SoundManager, this is a bug";
            Q_ASSERT(false);
        }
        m_latency = DEFAULT_LATENCY;
    }
}
