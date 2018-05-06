/***************************************************************************
                          sounddeviceportaudio.cpp
                             -------------------
    begin                : Sun Aug 15, 2007 (Stardate -315378.5417935057)
    copyright            : (C) 2007 Albert Santoni
    email                : gamegod \a\t users.sf.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SOUNDDEVICEPORTAUDIO_H
#define SOUNDDEVICEPORTAUDIO_H

#include <portaudio.h>

#include <QString>
#include "util/performancetimer.h"

#include "soundio/sounddevice.h"
#include "util/duration.h"


#define CPU_USAGE_UPDATE_RATE 30 // in 1/s, fits to display frame rate
#define CPU_OVERLOAD_DURATION 500 // in ms

class SoundManager;
class ControlProxy;

/** Dynamically resolved function which allows us to enable a realtime-priority callback
    thread from ALSA/PortAudio. This must be dynamically resolved because PortAudio can't
    tell us if ALSA is compiled into it or not. */
typedef int (*EnableAlsaRT)(PaStream* s, int enable);

class SoundDevicePortAudio : public SoundDevice {
  public:
    SoundDevicePortAudio(UserSettingsPointer config,
                         SoundManager *sm, const PaDeviceInfo *deviceInfo,
                         unsigned int devIndex);
    virtual ~SoundDevicePortAudio();

    virtual SoundDeviceError open(bool isClkRefDevice, int syncBuffers);
    virtual bool isOpen() const;
    virtual SoundDeviceError close();
    virtual void readProcess();
    virtual void writeProcess();
    virtual QString getError() const;

    // This callback function gets called everytime the sound device runs out of
    // samples (ie. when it needs more sound to play)
    int callbackProcess(const unsigned int framesPerBuffer,
                        CSAMPLE *output, const CSAMPLE* in,
                        const PaStreamCallbackTimeInfo *timeInfo,
                        PaStreamCallbackFlags statusFlags);
    // Same as above but with drift correction
    int callbackProcessDrift(const unsigned int framesPerBuffer,
                        CSAMPLE *output, const CSAMPLE* in,
                        const PaStreamCallbackTimeInfo *timeInfo,
                        PaStreamCallbackFlags statusFlags);
    // The same as above but drives the MixxEngine
    int callbackProcessClkRef(const unsigned int framesPerBuffer,
                        CSAMPLE *output, const CSAMPLE* in,
                        const PaStreamCallbackTimeInfo *timeInfo,
                        PaStreamCallbackFlags statusFlags);

    virtual unsigned int getDefaultSampleRate() const {
        return m_deviceInfo ? static_cast<unsigned int>(
            m_deviceInfo->defaultSampleRate) : 44100;
    }

  private:
    void updateCallbackEntryToDacTime(const PaStreamCallbackTimeInfo* timeInfo);
    void updateAudioLatencyUsage(const unsigned int framesPerBuffer);

    // PortAudio stream for this device.
    PaStream* volatile m_pStream;
    // PortAudio device index for this device.
    PaDeviceIndex m_devId;
    // Struct containing information about this device. Don't free() it, it
    // belongs to PortAudio.
    const PaDeviceInfo* m_deviceInfo;
    // Description of the output stream going to the soundcard.
    PaStreamParameters m_outputParams;
    // Description of the input stream coming from the soundcard.
    PaStreamParameters m_inputParams;
    FIFO<CSAMPLE>* m_outputFifo;
    FIFO<CSAMPLE>* m_inputFifo;
    bool m_outputDrift;
    bool m_inputDrift;

    // A string describing the last PortAudio error to occur.
    QString m_lastError;
    // Whether we have set the thread priority to realtime or not.
    bool m_bSetThreadPriority;
    ControlProxy* m_pMasterAudioLatencyOverloadCount;
    ControlProxy* m_pMasterAudioLatencyUsage;
    ControlProxy* m_pMasterAudioLatencyOverload;
    int m_underflowUpdateCount;
    static volatile int m_underflowHappened;
    mixxx::Duration m_timeInAudioCallback;
    int m_framesSinceAudioLatencyUsageUpdate;
    int m_syncBuffers;
    int m_invalidTimeInfoCount;
    PerformanceTimer m_clkRefTimer;
    PaTime m_lastCallbackEntrytoDacSecs;

};

#endif
