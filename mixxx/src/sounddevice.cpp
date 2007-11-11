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
#include "soundmanager.h"
#include "sounddevice.h"

SoundDevice::SoundDevice(ConfigObject<ConfigValue> * config, SoundManager * sm)
{
    m_pConfig = config;
    m_pSoundManager = sm;
    m_strName = "Unknown Soundcard";
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

QString SoundDevice::getName()
{
    return m_strName;
}

QString SoundDevice::getHostAPI()
{
    return m_hostAPI;
}

int SoundDevice::getNumInputChannels()
{
    return m_iNumInputChannels;
}

int SoundDevice::getNumOutputChannels()
{
    return m_iNumOutputChannels;
}

void SoundDevice::setHostAPI(QString api)
{
    m_hostAPI = api;
}

void SoundDevice::addSource(const AudioSource src)
{
    m_audioSources.push_back(src);
}

void SoundDevice::clearSources()
{
    while (!m_audioSources.empty())
        m_audioSources.pop_back();
}

void SoundDevice::addReceiver(const AudioReceiver recv)
{
    m_audioReceivers.push_back(recv);
}

void SoundDevice::clearReceivers()
{
    while (!m_audioReceivers.empty())
        m_audioReceivers.pop_back();
}

bool SoundDevice::operator== (SoundDevice * other)
{
    return (this->getName() == other->getName());
}

bool SoundDevice::operator== (QString other)
{
    return (this->getName() == other);
}

