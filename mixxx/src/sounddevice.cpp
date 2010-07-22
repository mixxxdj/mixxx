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

#include <QtDebug>
#include <QtCore>
#include "audiopath.h"
#include "soundmanager.h"
#include "sounddevice.h"

SoundDevice::SoundDevice(ConfigObject<ConfigValue> * config, SoundManager * sm)
{
    m_pConfig = config;
    m_pSoundManager = sm;
    m_strInternalName = "Unknown Soundcard";
    m_strDisplayName = "Unknown Soundcard";
    m_iNumOutputChannels = 2;
    m_iNumInputChannels = 2;
    m_iBufferSize = 3200; //~72 milliseconds at 44100 Hz
    m_dSampleRate = 44100.0f;
    //Add channel 1 and 2 as active channels
    //m_listActiveChannels.push_back(1);
    //m_listActiveChannels.push_back(2);
}

SoundDevice::~SoundDevice()
{

}

QString SoundDevice::getInternalName() const
{
    return m_strInternalName;
}

QString SoundDevice::getDisplayName() const
{
    return m_strDisplayName;
}

QString SoundDevice::getHostAPI() const
{
    return m_hostAPI;
}

int SoundDevice::getNumInputChannels() const
{
    return m_iNumInputChannels;
}

int SoundDevice::getNumOutputChannels() const
{
    return m_iNumOutputChannels;
}

void SoundDevice::setHostAPI(QString api)
{
    m_hostAPI = api;
}

int SoundDevice::addOutput(const AudioOutput &out)
{ 
    //Check if the output channels are already used
    QListIterator<AudioOutput> itr(m_audioOutputs);
    while (itr.hasNext())
    {
        AudioOutput out_internal = itr.next();
        if (out.channelsClash(out_internal)) {
            return MIXXX_ERROR_DUPLICATE_OUTPUT_CHANNEL;
        }
    }
    m_audioOutputs.push_back(out);
    
    return 0;
}

void SoundDevice::clearOutputs()
{
    m_audioOutputs.clear();
}

int SoundDevice::addInput(const AudioInput &in)
{
    //Check if the input channels are already used
    QListIterator<AudioInput> itr(m_audioInputs);
    while (itr.hasNext())
    {
        AudioInput in_internal = itr.next();
        if (in.channelsClash(in_internal)) {
            return MIXXX_ERROR_DUPLICATE_INPUT_CHANNEL;
        }
    }
    m_audioInputs.push_back(in);
    
    return 0;
}

void SoundDevice::clearInputs()
{
    m_audioInputs.clear();
}

bool SoundDevice::operator==(const SoundDevice &other) const
{
    return this->getInternalName() == other.getInternalName();
}

bool SoundDevice::operator==(const QString &other) const
{
    return getInternalName() == other;
}

SoundDeviceInfo SoundDevice::getInfo() const {
    SoundDeviceInfo info;
    info.displayName = getDisplayName();
    info.internalName = getInternalName();
    info.outputChannels = getNumOutputChannels();
    info.inputChannels = getNumInputChannels();
    return info;
}
