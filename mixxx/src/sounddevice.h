/***************************************************************************
                          sounddevice.cpp
                             -------------------
    begin                : Sun Aug 12, 2007, past my bedtime
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

#ifndef SOUNDDEVICE_H
#define SOUNDDEVICE_H

#include "soundmanager.h"

//Forward declarations
///....
class SoundDevice;
class SoundManager;
class AudioOutput;
class AudioInput;

enum SoundDeviceError {
    SOUNDDEVICE_ERROR_OK = OK,
    SOUNDDEVICE_ERROR_DUPLICATE_OUTPUT_CHANNEL,
    SOUNDDEVICE_ERROR_EXCESSIVE_OUTPUT_CHANNEL,
    SOUNDDEVICE_ERROR_EXCESSIVE_INPUT_CHANNEL,
};

class SoundDevice
{
    public:
        SoundDevice(ConfigObject<ConfigValue> *config, SoundManager* sm);
        virtual ~SoundDevice();
        QString getInternalName() const;
        QString getDisplayName() const;
        QString getHostAPI() const;
        void setHostAPI(QString api);
        void setSampleRate(double sampleRate);
        void setFramesPerBuffer(unsigned int framesPerBuffer);
        virtual int open() = 0;
        virtual int close() = 0;
        virtual QString getError() const = 0;
        int getNumOutputChannels() const;     
        int getNumInputChannels() const;
        SoundDeviceError addOutput(const AudioOutput &out);
        SoundDeviceError addInput(const AudioInput &in);
        void clearOutputs();
        void clearInputs();
        bool operator==(const SoundDevice &other) const;
        bool operator==(const QString &other) const;
    protected:
        ConfigObject<ConfigValue> *m_pConfig;
        SoundManager *m_pSoundManager;      //Pointer to the SoundManager object which we'll request audio from.
        QString m_strInternalName;          //The name of the soundcard, used internally (may include the device ID)
        QString m_strDisplayName;           //The name of the soundcard, as displayed to the user 
        int m_iNumOutputChannels;           //The number of output channels that the soundcard has
        int m_iNumInputChannels;            //The number of input channels that the soundcard has
        double m_dSampleRate;               //The current samplerate for the sound device.
        QString m_hostAPI;                  //The name of the audio API used by this device.
        unsigned int m_framesPerBuffer;
        QList<AudioOutput> m_audioOutputs;
        QList<AudioInput> m_audioInputs;
};

#endif
