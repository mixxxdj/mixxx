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

#include "soundio/soundmanagerconfig.h"

#include "soundio/soundmanagerutil.h"
#include "soundio/sounddevice.h"
#include "soundio/soundmanager.h"
#include "util/cmdlineargs.h"
#include "util/math.h"

// this (7) represents latency values from 1 ms to about 80 ms -- bkgood
const unsigned int SoundManagerConfig::kMaxAudioBufferSizeIndex = 7;

const QString SoundManagerConfig::kDefaultAPI = QString("None");
// Sample Rate even the cheap sound Devices will support most likely
const unsigned int SoundManagerConfig::kFallbackSampleRate = 48000;
const unsigned int SoundManagerConfig::kDefaultDeckCount = 2;
// audioBufferSizeIndex=5 means about 21 ms of latency which is default in trunk r2453 -- bkgood
const int SoundManagerConfig::kDefaultAudioBufferSizeIndex = 5;

const int SoundManagerConfig::kDefaultSyncBuffers = 2;

SoundManagerConfig::SoundManagerConfig()
    : m_api("None"),
      m_sampleRate(kFallbackSampleRate),
      m_deckCount(kDefaultDeckCount),
      m_audioBufferSizeIndex(kDefaultAudioBufferSizeIndex),
      m_syncBuffers(2),
      m_forceNetworkClock(false),
      m_iNumMicInputs(0),
      m_bExternalRecordBroadcastConnected(false) {
    m_configFile = QFileInfo(QDir(CmdlineArgs::Instance().getSettingsPath()).filePath(SOUNDMANAGERCONFIG_FILENAME));
}

SoundManagerConfig::~SoundManagerConfig() {
    // don't write to disk here, it's SoundManager's responsibility
    // to save its own configuration -- bkgood
}

/**
 * Read the SoundManagerConfig xml serialization at the predetermined
 * path
 * @returns false if the file can't be read or is invalid XML, true otherwise
 */
bool SoundManagerConfig::readFromDisk() {
    QFile file(m_configFile.absoluteFilePath());
    QDomDocument doc;
    QDomElement rootElement;
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }
    file.close();
    rootElement = doc.documentElement();
    setAPI(rootElement.attribute("api"));
    setSampleRate(rootElement.attribute("samplerate", "0").toUInt());
    // audioBufferSizeIndex is refereed as "latency" in the config file
    setAudioBufferSizeIndex(rootElement.attribute("latency", "0").toUInt());
    setSyncBuffers(rootElement.attribute("sync_buffers", "2").toUInt());
    setForceNetworkClock(rootElement.attribute("force_network_clock",
            "0").toUInt() != 0);
    setDeckCount(rootElement.attribute("deck_count",
            QString(kDefaultDeckCount)).toUInt());
    clearOutputs();
    clearInputs();
    QDomNodeList devElements(rootElement.elementsByTagName("SoundDevice"));
    for (int i = 0; i < devElements.count(); ++i) {
        QDomElement devElement(devElements.at(i).toElement());
        if (devElement.isNull()) continue;
        QString device(devElement.attribute("name"));
        if (device.isEmpty()) continue;
        QDomNodeList outElements(devElement.elementsByTagName("output"));
        QDomNodeList inElements(devElement.elementsByTagName("input"));
        for (int j = 0; j < outElements.count(); ++j) {
            QDomElement outElement(outElements.at(j).toElement());
            if (outElement.isNull()) continue;
            AudioOutput out(AudioOutput::fromXML(outElement));
            if (out.getType() == AudioPath::INVALID) continue;
            bool dupe(false);
            foreach (AudioOutput otherOut, m_outputs) {
                if (out == otherOut
                        && out.getChannelGroup() == otherOut.getChannelGroup()) {
                    dupe = true;
                    break;
                }
            }
            if (dupe) continue;
            addOutput(device, out);
        }
        for (int j = 0; j < inElements.count(); ++j) {
            QDomElement inElement(inElements.at(j).toElement());
            if (inElement.isNull()) continue;
            AudioInput in(AudioInput::fromXML(inElement));
            if (in.getType() == AudioPath::INVALID) continue;
            bool dupe(false);
            foreach (AudioInput otherIn, m_inputs) {
                if (in == otherIn
                        && in.getChannelGroup() == otherIn.getChannelGroup()) {
                    dupe = true;
                    break;
                }
            }
            if (dupe) continue;
            addInput(device, in);
        }
    }
    return true;
}

bool SoundManagerConfig::writeToDisk() const {
    QDomDocument doc("SoundManagerConfig");
    QDomElement docElement(doc.createElement("SoundManagerConfig"));
    docElement.setAttribute("api", m_api);
    docElement.setAttribute("samplerate", m_sampleRate);
    // audioBufferSizeIndex is refereed as "latency" in the config file
    docElement.setAttribute("latency", m_audioBufferSizeIndex);
    docElement.setAttribute("sync_buffers", m_syncBuffers);
    docElement.setAttribute("force_network_clock", m_forceNetworkClock);
    docElement.setAttribute("deck_count", m_deckCount);
    doc.appendChild(docElement);

    for (const auto& device: getDevices()) {
        QDomElement devElement(doc.createElement("SoundDevice"));
        devElement.setAttribute("name", device);
        foreach (AudioInput in, m_inputs.values(device)) {
            QDomElement inElement(doc.createElement("input"));
            in.toXML(&inElement);
            devElement.appendChild(inElement);
        }
        foreach (AudioOutput out, m_outputs.values(device)) {
            QDomElement outElement(doc.createElement("output"));
            out.toXML(&outElement);
            devElement.appendChild(outElement);
        }
        docElement.appendChild(devElement);
    }

    QFile file(m_configFile.absoluteFilePath());
    if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        return false;
    }
    file.write(doc.toString().toUtf8());
    file.close();
    return true;
}

QString SoundManagerConfig::getAPI() const {
    return m_api;
}

void SoundManagerConfig::setAPI(const QString &api) {
    // SoundManagerConfig doesn't necessarily have access to a SoundManager
    // instance, so I can't check for input validity here -- bkgood
    m_api = api;
}

/**
 * Checks that the API in the object is valid according to the list of APIs
 * given by SoundManager.
 * @returns false if the API is not found in SoundManager's list, otherwise
 *          true
 */
bool SoundManagerConfig::checkAPI(const SoundManager &soundManager) {
    if (!soundManager.getHostAPIList().contains(m_api) && m_api != "None") {
        return false;
    }
    return true;
}

unsigned int SoundManagerConfig::getSampleRate() const {
    return m_sampleRate;
}

void SoundManagerConfig::setSampleRate(unsigned int sampleRate) {
    // making sure we don't divide by zero elsewhere
    m_sampleRate = sampleRate != 0 ? sampleRate : kFallbackSampleRate;
}


unsigned int SoundManagerConfig::getSyncBuffers() const {
    return m_syncBuffers;
}

void SoundManagerConfig::setSyncBuffers(unsigned int syncBuffers) {
    // making sure we don't divide by zero elsewhere
    m_syncBuffers = qMin(syncBuffers, (unsigned int)2);
}

bool SoundManagerConfig::getForceNetworkClock() const {
    return m_forceNetworkClock;
}

void SoundManagerConfig::setForceNetworkClock(bool force) {
    m_forceNetworkClock = force;
}

/**
 * Checks that the sample rate in the object is valid according to the list of
 * sample rates given by SoundManager.
 * @returns false if the sample rate is not found in SoundManager's list,
 *          otherwise true
 */
bool SoundManagerConfig::checkSampleRate(const SoundManager &soundManager) {
    if (!soundManager.getSampleRates(m_api).contains(m_sampleRate)) {
        return false;
    }
    return true;
}

unsigned int SoundManagerConfig::getDeckCount() const {
    return m_deckCount;
}

void SoundManagerConfig::setDeckCount(unsigned int deckCount) {
    m_deckCount = deckCount;
}

void SoundManagerConfig::setCorrectDeckCount(int configuredDeckCount) {
    int minimum_deck_count = 0;

    for (const auto& device: getDevices()) {
        foreach (AudioInput in, m_inputs.values(device)) {
            if ((in.getType() == AudioInput::DECK ||
                 in.getType() == AudioInput::VINYLCONTROL ||
                 in.getType() == AudioInput::AUXILIARY) &&
                in.getIndex() + 1 > minimum_deck_count) {
                qDebug() << "Found an input connection above current deck count";
                minimum_deck_count = in.getIndex() + 1;
            }
        }
        foreach (AudioOutput out, m_outputs.values(device)) {
            if (out.getType() == AudioOutput::DECK &&
                    out.getIndex() + 1 > minimum_deck_count) {
                qDebug() << "Found an output connection above current deck count";
                minimum_deck_count = out.getIndex() + 1;
            }
        }
    }

    if (minimum_deck_count > configuredDeckCount) {
        m_deckCount = minimum_deck_count;
    } else {
        m_deckCount = configuredDeckCount;
    }
}

unsigned int SoundManagerConfig::getAudioBufferSizeIndex() const {
    return m_audioBufferSizeIndex;
}

// FIXME: This is incorrect when using JACK as the sound API!
// m_audioBufferSizeIndex does not reflect JACK's buffer size.
unsigned int SoundManagerConfig::getFramesPerBuffer() const {
    // endless loop otherwise
    unsigned int audioBufferSizeIndex = m_audioBufferSizeIndex;
    VERIFY_OR_DEBUG_ASSERT(audioBufferSizeIndex > 0) {
        audioBufferSizeIndex = kDefaultAudioBufferSizeIndex;
    }
    unsigned int framesPerBuffer = 1;
    double sampleRate = m_sampleRate; // need this to avoid int division
    // first, get to the framesPerBuffer value corresponding to latency index 1
    for (; framesPerBuffer / sampleRate * 1000 < 1.0; framesPerBuffer *= 2) {
    }
    // then, keep going until we get to our desired latency index (if not 1)
    for (unsigned int latencyIndex = 1; latencyIndex < audioBufferSizeIndex; ++latencyIndex) {
        framesPerBuffer <<= 1; // *= 2
    }
    return framesPerBuffer;
}

// FIXME: This is incorrect when using JACK as the sound API!
// m_audioBufferSizeIndex does not reflect JACK's buffer size.
double SoundManagerConfig::getProcessingLatency() const {
    return static_cast<double>(getFramesPerBuffer()) / m_sampleRate * 1000.0;
}


// Set the audio buffer size
// @warning This IS NOT a value in milliseconds, or a number of frames per
// buffer. It is an index, where 1 is the first power-of-two buffer size (in
// frames) which corresponds to a latency greater than or equal to 1 ms, 2 is
// the second, etc. This is so that latency values are roughly equivalent
// between different sample rates.
void SoundManagerConfig::setAudioBufferSizeIndex(unsigned int sizeIndex) {
    // latency should be either the min of kMaxAudioBufferSizeIndex and the passed value
    // if it's 0, pretend it was 1 -- bkgood
    m_audioBufferSizeIndex = sizeIndex != 0 ? math_min(sizeIndex, kMaxAudioBufferSizeIndex) : 1;
}

void SoundManagerConfig::addOutput(const QString &device, const AudioOutput &out) {
    m_outputs.insert(device, out);
}

void SoundManagerConfig::addInput(const QString &device, const AudioInput &in) {
    m_inputs.insert(device, in);
    if (in.getType() == AudioPath::MICROPHONE) {
        m_iNumMicInputs++;
    } else if (in.getType() == AudioPath::RECORD_BROADCAST) {
        m_bExternalRecordBroadcastConnected = true;
    }
}

QMultiHash<QString, AudioOutput> SoundManagerConfig::getOutputs() const {
    return m_outputs;
}

QMultiHash<QString, AudioInput> SoundManagerConfig::getInputs() const {
    return m_inputs;
}

void SoundManagerConfig::clearOutputs() {
    m_outputs.clear();
}

void SoundManagerConfig::clearInputs() {
    m_inputs.clear();
    m_iNumMicInputs = 0;
    m_bExternalRecordBroadcastConnected = false;
}

bool SoundManagerConfig::hasMicInputs() {
    return m_iNumMicInputs;
}

bool SoundManagerConfig::hasExternalRecordBroadcast() {
    return m_bExternalRecordBroadcastConnected;
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

    unsigned int defaultSampleRate = kFallbackSampleRate;
    if (flags & SoundManagerConfig::DEVICES) {
        clearOutputs();
        clearInputs();
        QList<SoundDevicePointer> outputDevices = soundManager->getDeviceList(m_api, true, false);
        if (!outputDevices.isEmpty()) {
            for (const auto& pDevice: outputDevices) {
                if (pDevice->getNumOutputChannels() < 2) {
                    continue;
                }
                AudioOutput masterOut(AudioPath::MASTER, 0, 2, 0);
                addOutput(pDevice->getInternalName(), masterOut);
                defaultSampleRate = pDevice->getDefaultSampleRate();
                break;
            }
        }
    }
    if (flags & SoundManagerConfig::OTHER) {
        QList<unsigned int> sampleRates = soundManager->getSampleRates(m_api);
        if (sampleRates.contains(defaultSampleRate)) {
            m_sampleRate = defaultSampleRate;
        } else if (sampleRates.contains(kFallbackSampleRate)) {
            m_sampleRate = kFallbackSampleRate;
        } else if (!sampleRates.isEmpty()) {
            m_sampleRate = sampleRates.first();
        } else {
            qWarning() << "got empty sample rate list from SoundManager, this is a bug";
            m_sampleRate = kFallbackSampleRate;
        }
        m_audioBufferSizeIndex = kDefaultAudioBufferSizeIndex;
    }

    m_syncBuffers = kDefaultSyncBuffers;
    m_forceNetworkClock = false;
}

QSet<QString> SoundManagerConfig::getDevices() const {
    QSet<QString> devices;
    devices.reserve(m_outputs.size() + m_inputs.size());
    for (auto it = m_outputs.constBegin(); it != m_outputs.constEnd(); ++it) {
        devices.insert(it.key());
    }
    for (auto it = m_inputs.constBegin(); it != m_inputs.constEnd(); ++it) {
        devices.insert(it.key());
    }
    return devices;
}
