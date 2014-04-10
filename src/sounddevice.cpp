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
#include <cstring> // for memcpy and strcmp

#include "sounddevice.h"

#include "soundmanagerutil.h"
#include "soundmanager.h"
#include "util/debug.h"
#include "sampleutil.h"

SoundDevice::SoundDevice(ConfigObject<ConfigValue> * config, SoundManager * sm)
        : m_pConfig(config),
          m_pSoundManager(sm),
          m_strInternalName("Unknown Soundcard"),
          m_strDisplayName("Unknown Soundcard"),
          m_iNumOutputChannels(2),
          m_iNumInputChannels(2),
          m_dSampleRate(44100.0),
          m_hostAPI("Unknown API"),
          m_framesPerBuffer(0),
          m_pRenderBuffer(SampleUtil::alloc(MAX_BUFFER_LEN)) {
}

SoundDevice::~SoundDevice() {
    SampleUtil::free(m_pRenderBuffer);
}

QString SoundDevice::getDisplayName() const {
    return m_strDisplayName;
}

QString SoundDevice::getHostAPI() const {
    return m_hostAPI;
}

int SoundDevice::getNumInputChannels() const {
    return m_iNumInputChannels;
}

int SoundDevice::getNumOutputChannels() const {
    return m_iNumOutputChannels;
}

void SoundDevice::setHostAPI(QString api) {
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
    if (framesPerBuffer * 2 > MAX_BUFFER_LEN) {
        // framesPerBuffer * 2 because a frame will generally end up
        // being 2 samples and MAX_BUFFER_LEN is a number of samples
        // this isn't checked elsewhere, so...
        reportFatalErrorAndQuit("framesPerBuffer too big in "
                                "SoundDevice::setFramesPerBuffer(uint)");
    }
    m_framesPerBuffer = framesPerBuffer;
}

SoundDeviceError SoundDevice::addOutput(const AudioOutputBuffer &out) {
    //Check if the output channels are already used
    foreach (AudioOutputBuffer myOut, m_audioOutputs) {
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

void SoundDevice::clearOutputs() {
    m_audioOutputs.clear();
}

SoundDeviceError SoundDevice::addInput(const AudioInputBuffer &in) {
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

void SoundDevice::clearInputs() {
    m_audioInputs.clear();
}

bool SoundDevice::operator==(const SoundDevice &other) const {
    return this->getInternalName() == other.getInternalName();
}

bool SoundDevice::operator==(const QString &other) const {
    return getInternalName() == other;
}

void SoundDevice::onOutputBuffersReady(const unsigned long iFramesPerBuffer) {
    // qDebug() << getInternalName() << "onOutputBuffersReady" << iFramesPerBuffer
    //          << "outputs" << m_audioInputs.size();
    for (QList<AudioOutputBuffer>::iterator it = m_audioOutputs.begin();
         it != m_audioOutputs.end(); ++it) {
        AudioOutputBuffer& buffer = *it;
        const CSAMPLE* pBuffer = buffer.getBuffer();
        FIFO<CSAMPLE>* pFifo = buffer.getFifo();

        // All engine buffers are stereo.
        int samplesToWrite = iFramesPerBuffer * 2;
        int written = pFifo->write(pBuffer, samplesToWrite);

        if (written != samplesToWrite) {
            qWarning() << getInternalName()
                       << "AudioOutputBuffer FIFO buffer overflow"
                       << "wanted to write" << samplesToWrite
                       << "but wrote" << written
                       << "This device is lagging behind the clock reference device!";
        }
    }
}

void SoundDevice::composeOutputBuffer(CSAMPLE* outputBuffer,
                                      const unsigned long iFramesPerBuffer,
                                      const unsigned int iFrameSize) {
    //qDebug() << "SoundDevice::composeOutputBuffer()"
    //         << device->getInternalName()
    //         << iFramesPerBuffer << iFrameSize;

    // If we are the clock reference device then we are able to use audio
    // buffers directly (since we know we are not concurrent with the engine
    // processing). Otherwise, we must read from the AudioOutput FIFO.
    bool isReferenceDevice = m_pSoundManager->isDeviceClkRef(this);

    // Reset sample for each open channel
    SampleUtil::clear(outputBuffer, iFramesPerBuffer * iFrameSize);

    // Interlace Audio data onto portaudio buffer.  We iterate through the
    // source list to find out what goes in the buffer data is interlaced in
    // the order of the list

    for (QList<AudioOutputBuffer>::iterator i = m_audioOutputs.begin(),
                 e = m_audioOutputs.end(); i != e; ++i) {
        AudioOutputBuffer& out = *i;

        const ChannelGroup outChans = out.getChannelGroup();
        const int iChannelCount = outChans.getChannelCount();
        const int iChannelBase = outChans.getChannelBase();

        const CSAMPLE* pAudioOutputBuffer = NULL;
        if (isReferenceDevice) {
            // If we are the reference device we can read the buffer directly
            // because we know we are not concurrent with the engine processing.
            pAudioOutputBuffer = out.getBuffer();
        } else {
            FIFO<CSAMPLE>* pFifo = out.getFifo();
            // All buffers from the engine are stereo.
            int samplesToRead = iFramesPerBuffer * 2;
            memset(m_pRenderBuffer, 0, samplesToRead * sizeof(*m_pRenderBuffer));

            int offset = 0;
            int available = pFifo->readAvailable();

            if (available < samplesToRead) {
                qWarning() << getInternalName()
                           << "AudioOutputBuffer FIFO buffer underflow"
                           << "want" << samplesToRead
                           << "available" << available
                           << "This device is running faster than the clock reference device!";
                offset = samplesToRead - available;
                samplesToRead = available;
            }

            int samplesRead = pFifo->read(m_pRenderBuffer + offset, samplesToRead);
            if (samplesRead < samplesToRead) {
                // Should not happen. We already verified samplesToRead samples
                // were available.
            }
            pAudioOutputBuffer = m_pRenderBuffer;
        }

        if (iChannelCount == 1) {
            // All AudioOutputs are stereo as of Mixxx 1.12.0. If we have a mono
            // output then we need to downsample.
            for (unsigned int iFrameNo = 0; iFrameNo < iFramesPerBuffer; ++iFrameNo) {
                // iFrameBase is the "base sample" in a frame (ie. the first
                // sample in a frame)
                const unsigned int iFrameBase = iFrameNo * iFrameSize;
                outputBuffer[iFrameBase + iChannelBase] =
                        (pAudioOutputBuffer[iFrameNo*2] +
                                pAudioOutputBuffer[iFrameNo*2 + 1]) / 2.0f;
            }
        } else {
            for (unsigned int iFrameNo = 0; iFrameNo < iFramesPerBuffer; ++iFrameNo) {
                // iFrameBase is the "base sample" in a frame (ie. the first
                // sample in a frame)
                const unsigned int iFrameBase = iFrameNo * iFrameSize;
                const unsigned int iLocalFrameBase = iFrameNo * iChannelCount;

                // this will make sure a sample from each channel is copied
                for (int iChannel = 0; iChannel < iChannelCount; ++iChannel) {
                    outputBuffer[iFrameBase + iChannelBase + iChannel] =
                            pAudioOutputBuffer[iLocalFrameBase + iChannel];

                    // Input audio pass-through (useful for debugging)
                    //if (in)
                    //    output[iFrameBase + src.channelBase + iChannel] =
                    //    in[iFrameBase + src.channelBase + iChannel];
                }
            }
        }
    }
}

