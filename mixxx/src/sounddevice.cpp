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
#include "soundmanagerutil.h"
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
    m_dSampleRate = 44100.0f;
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

void SoundDevice::setSampleRate(double sampleRate) {
    if (sampleRate <= 0.0) {
        // this is the default value used elsewhere in this file
        sampleRate = 44100.0;
    }
    m_dSampleRate = sampleRate;
}

void SoundDevice::setFramesPerBuffer(unsigned int framesPerBuffer) {
    if (framesPerBuffer * 2 > (unsigned int) MAX_BUFFER_LEN) {
        // framesPerBuffer * 2 because a frame will generally end up
        // being 2 samples and MAX_BUFFER_LEN is a number of samples
        // this isn't checked elsewhere, so...
        qFatal("framesPerBuffer too big in "
                "SoundDevice::setFramesPerBuffer(uint)");
    }
    m_framesPerBuffer = framesPerBuffer;
}

SoundDeviceError SoundDevice::addOutput(const AudioOutput &out)
{ 
    //Check if the output channels are already used
    foreach (AudioOutput myOut, m_audioOutputs) {
        if (out.channelsClash(myOut)) {
            return SOUNDDEVICE_ERROR_DUPLICATE_OUTPUT_CHANNEL;
        }
    }
    if (out.getChannelGroup().getChannelBase()
            + out.getChannelGroup().getChannelCount() > getNumOutputChannels()) {
        return SOUNDDEVICE_ERROR_EXCESSIVE_OUTPUT_CHANNEL;
    }
    m_audioOutputs.append(out);
    return SOUNDDEVICE_ERROR_OK;
}

void SoundDevice::clearOutputs()
{
    m_audioOutputs.clear();
}

SoundDeviceError SoundDevice::addInput(const AudioInput &in)
{
    // DON'T check if the input channels are already used, there's no reason
    // we can't send the same inputted samples to different places in mixxx.
    // -- bkgood 20101108
    if (in.getChannelGroup().getChannelBase()
            + in.getChannelGroup().getChannelCount() > getNumInputChannels()) {
        return SOUNDDEVICE_ERROR_EXCESSIVE_INPUT_CHANNEL;
    }
    m_audioInputs.append(in);
    return SOUNDDEVICE_ERROR_OK;
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
