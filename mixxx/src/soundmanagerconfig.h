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
#include "audiopath.h"

class SoundDevice;
class SoundManager;

const unsigned int MAX_LATENCY = 8; // this represents latency values from 1 ms to about
                                    // 180 ms, should be sufficient -- bkgood
// no DEFAULT_API because we have to check that the api is available -- bkgood
const unsigned int DEFAULT_SAMPLE_RATE = 48000;
// default is max so that we don't set latency too low and make mixxx look bad
// to newbies with a bunch of pops and clicks (maybe this should be more like
// the ~80 ms value instead of the ~180 value though) -- bkgood
const int DEFAULT_LATENCY = MAX_LATENCY;

class SoundManagerConfig {
public:
    enum Defaults {
        API = (1 << 0),
        DEVICES = (1 << 1),
        OTHER = (1 << 2),
    };
    SoundManagerConfig();
    ~SoundManagerConfig();
    bool readFromDisk();
    bool writeToDisk() const;
    QString getAPI() const;
    void setAPI(QString api);
    unsigned int getSampleRate() const;
    void setSampleRate(unsigned int sampleRate);
    unsigned int getLatency() const;
    void setLatency(unsigned int latency);
    void addSource(SoundDevice *device, AudioSource source);
    void addReceiver(SoundDevice *device, AudioReceiver receiver);
    QList<QPair<SoundDevice*, AudioSource> > getSources() const;
    QList<QPair<SoundDevice*, AudioReceiver> > getReceivers() const;
    void clearSources();
    void clearReceivers();
    void loadDefaults(SoundManager *soundManager, int flags);
private:
    QFileInfo m_configFile;
    QString m_api;
    // none of our sample rates are actually decimals, this avoids
    // the weirdness using floating point can introduce
    unsigned int m_sampleRate;
    // m_latency is an index > 0, where 1 is a latency of 1ms and
    // higher indices represent subsequently higher latencies (storing
    // latency as milliseconds or frames per buffer is bad because those
    // values very with sample rate) -- bkgood
    unsigned int m_latency; // this is an index
    QList<QPair<SoundDevice*, AudioSource> > m_sources;
    QList<QPair<SoundDevice*, AudioReceiver> > m_receivers;
};
#endif
