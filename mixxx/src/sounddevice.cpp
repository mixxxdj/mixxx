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

QString SoundDevice::getInternalName()
{
    return m_strInternalName;
}

QString SoundDevice::getDisplayName()
{
    return m_strDisplayName;
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

int SoundDevice::addSource(const AudioSource src)
{ 
	//Check if the output channels are already used
	QListIterator<AudioSource> itr(m_audioSources);
	while(itr.hasNext())
	{
		AudioSource src_internal = itr.next();
		AudioSource src_lower = (src_internal.channelBase < src.channelBase) ? src_internal : src;
		AudioSource src_higher = (src_internal.channelBase < src.channelBase) ?  src : src_internal;
		if((src_lower.channelBase + src_lower.channels) > src_higher.channelBase)
			return MIXXX_ERROR_DUPLICATE_OUTPUT_CHANNEL;
	}
    m_audioSources.push_back(src);
    
    return 0;
}

void SoundDevice::clearSources()
{
    while (!m_audioSources.empty())
        m_audioSources.pop_back();
}

int SoundDevice::addReceiver(const AudioReceiver recv)
{
    //Check if the input channels are already used
    QListIterator<AudioReceiver> itr(m_audioReceivers);
	while(itr.hasNext())
	{
		AudioReceiver recv_internal = itr.next();
		AudioReceiver recv_lower  = (recv_internal.channelBase < recv.channelBase) ?  recv_internal : recv;
		AudioReceiver recv_higher = (recv_internal.channelBase < recv.channelBase) ?  recv : recv_internal;
		if((recv_lower.channelBase + recv_lower.channels) > recv_higher.channelBase)
			return MIXXX_ERROR_DUPLICATE_INPUT_CHANNEL;
	}
    m_audioReceivers.push_back(recv);
    
    return 0;
}

void SoundDevice::clearReceivers()
{
    while (!m_audioReceivers.empty())
        m_audioReceivers.pop_back();
}

bool SoundDevice::operator== (SoundDevice * other)
{
    return (this->getInternalName() == other->getInternalName());
}

bool SoundDevice::operator== (QString other)
{
    return (this->getInternalName() == other);
}

