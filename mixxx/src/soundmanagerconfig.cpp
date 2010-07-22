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
#include "audiopath.h"
#include "sounddevice.h"
#include "soundmanager.h"

SoundManagerConfig::SoundManagerConfig()
    : m_api("None")
    , m_sampleRate(DEFAULT_SAMPLE_RATE)
    , m_latency(DEFAULT_LATENCY) {
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
    return m_api;
}

void SoundManagerConfig::setAPI(QString api) {
    // SoundManagerConfig doesn't necessarily have access to a SoundManager
    // instance, so I can't check for input validity here -- bkgood
    m_api = api;
}

unsigned int SoundManagerConfig::getSampleRate() const {
    return m_sampleRate;
}

void SoundManagerConfig::setSampleRate(unsigned int sampleRate) {
    // making sure we don't divide by zero elsewhere
    m_sampleRate = sampleRate != 0 ? sampleRate : DEFAULT_SAMPLE_RATE;
}

unsigned int SoundManagerConfig::getLatency() const {
    return m_latency;
}

unsigned int SoundManagerConfig::getFramesPerBuffer() const {
    Q_ASSERT(m_latency > 0); // endless loop otherwise
    unsigned int framesPerBuffer = 1;
    double sampleRate = m_sampleRate; // need this to avoid int division
    // first, get to the framesPerBuffer value corresponding to latency index 1
    for (; framesPerBuffer / sampleRate * 1000 < 1.0; framesPerBuffer *= 2);
    // then, keep going until we get to our desired latency index (if not 1)
    for (unsigned int latencyIndex = 1; latencyIndex < m_latency; ++latencyIndex) {
        framesPerBuffer <<= 1; // *= 2
    }
    return framesPerBuffer;
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
    // latency should be either the min of MAX_LATENCY and the passed value
    // if it's 0, pretend it was 1 -- bkgood
    m_latency = latency != 0 ? math_min(latency, MAX_LATENCY) : 1;
}

void SoundManagerConfig::addOutput(SoundDevice *device, AudioOutput out) {
    m_outputs.insert(device, out);
}

void SoundManagerConfig::addInput(SoundDevice *device, AudioInput in) {
    m_inputs.insert(device, in);
}

QMultiHash<SoundDevice*, AudioOutput> SoundManagerConfig::getOutputs() const {
    return m_outputs;
}

QMultiHash<SoundDevice*, AudioInput> SoundManagerConfig::getInputs() const {
    return m_inputs;
}

void SoundManagerConfig::clearOutputs() {
    m_outputs.clear();
}

void SoundManagerConfig::clearInputs() {
    m_inputs.clear();
}

/**
 * Loads default values for API, master output, sample rate and/or latency.
 * @param soundManager pointer to SoundManager instance to load data from
 * @param flags Bitfield to determine which defaults to load, use something
 *              like SoundManagerConfig::API | SoundManagerConfig::DEVICES to
 *              load default API and master device.
 */
void SoundManagerConfig::loadDefaults(SoundManager *soundManager, unsigned int flags) {
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
            // hoping this counts as more advanced, tests if ASIO is an option
            // and then that we have at least one ASIO output device -- bkgood
            if (apiList.contains(MIXXX_PORTAUDIO_ASIO_STRING)
                   && !soundManager->getDeviceList(
                       MIXXX_PORTAUDIO_ASIO_STRING, true, false).isEmpty()) {
                m_api = MIXXX_PORTAUDIO_ASIO_STRING;
            } else {
                m_api = MIXXX_PORTAUDIO_DIRECTSOUND_STRING;
            }
#endif
#ifdef __APPLE__
            m_api = MIXXX_PORTAUDIO_COREAUDIO_STRING;
#endif
        }
    }
    if (flags & SoundManagerConfig::DEVICES) {
        clearOutputs();
        clearInputs();
        QList<SoundDevice*> outputDevices = soundManager->getDeviceList(m_api, true, false);
        if (!outputDevices.isEmpty() && outputDevices.first()->getNumOutputChannels() > 1) {
            SoundDevice *masterDevice = outputDevices.first();
            AudioOutput masterOut(AudioPath::MASTER, 0);
            addOutput(masterDevice, masterOut);
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
