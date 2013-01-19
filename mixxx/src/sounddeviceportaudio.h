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
#include "sounddevice.h"

class SoundManager;

/** Maximum frame size used with PortAudio. Used to determine no of buffers
  * when setting latency */
#define MIXXXPA_MAX_FRAME_SIZE 1024

/** Dynamically resolved function which allows us to enable a realtime-priority callback
    thread from ALSA/PortAudio. This must be dynamically resolved because PortAudio can't
    tell us if ALSA is compiled into it or not. */
typedef int (*EnableAlsaRT)(PaStream* s, int enable);

class SoundDevicePortAudio;

class SoundDevicePortAudio : public SoundDevice
{
    public:
        SoundDevicePortAudio(ConfigObject<ConfigValue> *config,
                SoundManager *sm, const PaDeviceInfo *deviceInfo,
                unsigned int devIndex);
        ~SoundDevicePortAudio();
        int open();
        int close();
        QString getError() const;
        int callbackProcess(unsigned long framesPerBuffer,
                float *output, short *in,
                const PaStreamCallbackTimeInfo *timeInfo,
                PaStreamCallbackFlags statusFlags);
        virtual unsigned int getDefaultSampleRate() const {
            return (unsigned int)m_deviceInfo->defaultSampleRate;
        }
    private:
        /** PortAudio stream for this device. */
        PaStream *m_pStream;
        /** PortAudio device index for this device.*/
        PaDeviceIndex m_devId;
        /** Struct containing information about this device. Don't free() it, it belongs to PortAudio.*/
        const PaDeviceInfo *m_deviceInfo;
        /** Number of buffers */
        int m_iNumberOfBuffers;
        /** Number of active/open soundcards */
        //int m_iNumActiveDevices;
        /** Description of the output stream going to the soundcard */
        PaStreamParameters m_outputParams;
        /** Description of the input stream coming from the soundcard */
	    PaStreamParameters m_inputParams;
        /** A string describing the last PortAudio error to occur */
        QString m_lastError;

        bool m_bSetThreadPriority;

        ControlObject* m_pMasterUnderflowCount;
        int m_undeflowUpdateCount;
};



int paV19Callback(const void *inputBuffer, void *outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo* timeInfo,
                        PaStreamCallbackFlags statusFlags,
                        void *soundDevice);

#endif
