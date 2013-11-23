/**
 * @file soundmanagerconfig.h
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

#ifndef SOUNDMANAGERCONFIG_H
#define SOUNDMANAGERCONFIG_H

#ifndef SOUNDMANAGERCONFIG_FILENAME
#define SOUNDMANAGERCONFIG_FILENAME "soundconfig.xml"
#endif

#include <QtCore>

#include "soundmanagerutil.h"

class SoundDevice;
class SoundManager;

class SoundManagerConfig {
public:
    enum Defaults {
        API = (1 << 0),
        DEVICES = (1 << 1),
        OTHER = (1 << 2),
        ALL = (API | DEVICES | OTHER),
    };
    static const unsigned int kMaxAudioBufferSizeIndex;
    static const QString kDefaultAPI;
    static const unsigned int kFallbackSampleRate;
    static const int kDefaultAudioBufferSizeIndex;

    SoundManagerConfig();
    ~SoundManagerConfig();
    bool readFromDisk();
    bool writeToDisk() const;
    QString getAPI() const;
    void setAPI(const QString &api);
    bool checkAPI(const SoundManager &soundManager);
    unsigned int getSampleRate() const;
    void setSampleRate(unsigned int sampleRate);
    bool checkSampleRate(const SoundManager &soundManager);
    unsigned int getAudioBufferSizeIndex() const;
    unsigned int getFramesPerBuffer() const;
    void setAudioBufferSizeIndex(unsigned int latency);
    void addOutput(const QString &device, const AudioOutput &out);
    void addInput(const QString &device, const AudioInput &in);
    QMultiHash<QString, AudioOutput> getOutputs() const;
    QMultiHash<QString, AudioInput> getInputs() const;
    void clearOutputs();
    void clearInputs();
    // I'd prefer for these to be const but SoundManager::getDeviceList isn't
    void filterOutputs(SoundManager *soundManager);
    void filterInputs(SoundManager *soundManager);
    void loadDefaults(SoundManager *soundManager, unsigned int flags);
private:
    QFileInfo m_configFile;
    QString m_api;
    // none of our sample rates are actually decimals, this avoids
    // the weirdness using floating point can introduce
    unsigned int m_sampleRate;
    // m_latency is an index > 0, where 1 is a latency of 1ms and
    // higher indices represent subsequently higher latencies (storing
    // latency as milliseconds or frames per buffer is bad because those
    // values vary with sample rate) -- bkgood
    unsigned int m_audioBufferSizeIndex;
    QMultiHash<QString, AudioOutput> m_outputs;
    QMultiHash<QString, AudioInput> m_inputs;
};
#endif
