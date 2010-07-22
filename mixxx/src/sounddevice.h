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
struct SoundDeviceInfo;

#define MIXXX_ERROR_DUPLICATE_OUTPUT_CHANNEL 0x0f00f00
#define MIXXX_ERROR_DUPLICATE_INPUT_CHANNEL  0x0100100

class SoundDevice
{
    public:
        SoundDevice(ConfigObject<ConfigValue> *config, SoundManager* sm);
        virtual ~SoundDevice();
        QString getInternalName() const;
        QString getDisplayName() const;
        QString getHostAPI() const;
        void setHostAPI(QString api);        
        virtual int open() = 0;
        virtual int close() = 0;
        int getNumOutputChannels() const;     
        int getNumInputChannels() const;
        int addOutput(const AudioOutput &out);
        int addInput(const AudioInput &in);
        void clearOutputs();
        void clearInputs();
        bool operator==(const SoundDevice &other) const;
        bool operator==(const QString &other) const;
        SoundDeviceInfo getInfo() const;
    protected:
    
    //TODO: Cleanup unused members
        ConfigObject<ConfigValue> *m_pConfig;
        SoundManager *m_pSoundManager;      //Pointer to the SoundManager object which we'll request audio from.
        QString m_strInternalName;          //The name of the soundcard, used internally (may include the device ID)
        QString m_strDisplayName;           //The name of the soundcard, as displayed to the user 
        int m_iNumOutputChannels;           //The number of output channels that the soundcard has
        int m_iNumInputChannels;            //The number of input channels that the soundcard has
        int m_iBufferSize;                  //The number of samples in a buffer.
        double m_dSampleRate;               //The current samplerate for the sound device.
        QString m_hostAPI;                  //The name of the audio API used by this device.
        //int m_iLatency;                       //The latency of the soundcard in milliseconds (TODO: Use bufferSize instead?)
        QList<AudioOutput> m_audioOutputs;
        QList<AudioInput> m_audioInputs;
        //QList<int> m_listActiveOutputChannels;    //A list containing the output channels which are currently active on the soundcard.
        //QList<int> m_listActiveInputChannels;     //A list containing the input channels which are currently active on the soundcard.

};

struct SoundDeviceInfo {
    QString displayName;
    QString internalName;
    int outputChannels;
    int inputChannels;
};

#endif
