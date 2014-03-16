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

#include <portaudio.h>

#include <QtDebug>
#include <QThread>

#ifdef __LINUX__
#include <QLibrary>
#endif

#include "sounddeviceportaudio.h"

#include "soundmanager.h"
#include "sounddevice.h"
#include "soundmanagerutil.h"
#include "controlobject.h"
#include "visualplayposition.h"
#include "util/timer.h"
#include "util/trace.h"
#include "vinylcontrol/defs_vinylcontrol.h"
#include "sampleutil.h"

SoundDevicePortAudio::SoundDevicePortAudio(ConfigObject<ConfigValue> *config, SoundManager *sm,
                                           const PaDeviceInfo *deviceInfo, unsigned int devIndex)
        : SoundDevice(config, sm),
          m_pStream(NULL),
          m_devId(devIndex),
          m_deviceInfo(deviceInfo),
          m_pOutputBuffer(NULL),
          m_pInputBuffer(NULL),
          m_bSetThreadPriority(false),
          m_pMasterUnderflowCount(ControlObject::getControl(
              ConfigKey("[Master]", "underflow_count"))),
          m_underflowUpdateCount(0) {
    // Setting parent class members:
    m_hostAPI = Pa_GetHostApiInfo(deviceInfo->hostApi)->name;
    m_dSampleRate = deviceInfo->defaultSampleRate;
    m_strInternalName = QString("%1, %2").arg(QString::number(m_devId), deviceInfo->name);
    m_strDisplayName = QString(deviceInfo->name);
    m_iNumInputChannels = m_deviceInfo->maxInputChannels;
    m_iNumOutputChannels = m_deviceInfo->maxOutputChannels;
}

SoundDevicePortAudio::~SoundDevicePortAudio() {
}

int SoundDevicePortAudio::open(bool registerCallback) {
    qDebug() << "SoundDevicePortAudio::open()" << getInternalName();
    PaError err;

    if (m_audioOutputs.empty() && m_audioInputs.empty()) {
        m_lastError = QString::fromAscii("No inputs or outputs in SDPA::open() "
            "(THIS IS A BUG, this should be filtered by SM::setupDevices)");
        return ERR;
    }

    memset(&m_outputParams, 0, sizeof(m_outputParams));
    memset(&m_inputParams, 0, sizeof(m_inputParams));
    PaStreamParameters* pOutputParams = &m_outputParams;
    PaStreamParameters* pInputParams = &m_inputParams;

    // Look at how many audio outputs we have,
    // so we can figure out how many output channels we need to open.
    if (m_audioOutputs.empty()) {
        m_outputParams.channelCount = 0;
        pOutputParams = NULL;
    } else {
        foreach (AudioOutput out, m_audioOutputs) {
            ChannelGroup channelGroup = out.getChannelGroup();
            int highChannel = channelGroup.getChannelBase()
                    + channelGroup.getChannelCount();
            if (m_outputParams.channelCount <= highChannel) {
                m_outputParams.channelCount = highChannel;
            }
        }
    }

    // Look at how many audio inputs we have,
    // so we can figure out how many input channels we need to open.
    if (m_audioInputs.empty()) {
        m_inputParams.channelCount = 0;
        pInputParams = NULL;
    } else {
        foreach (AudioInput in, m_audioInputs) {
            ChannelGroup channelGroup = in.getChannelGroup();
            int highChannel = channelGroup.getChannelBase()
                + channelGroup.getChannelCount();
            if (m_inputParams.channelCount <= highChannel) {
                m_inputParams.channelCount = highChannel;
            }
        }
    }

    // Workaround for Bug #900364. The PortAudio ALSA hostapi opens the minimum
    // number of device channels supported by the device regardless of our
    // channel request. It has no way of notifying us when it does this. The
    // typical case this happens is when we are opening a device in mono when it
    // supports a minimum of stereo. To work around this, simply open the device
    // in stereo and only take the first channel.
    // TODO(rryan): Remove once PortAudio has a solution built in (and
    // released).
    if (m_deviceInfo->hostApi == paALSA) {
        // Only engage workaround if the device has enough input and output
        // channels.
        if (m_deviceInfo->maxInputChannels >= 2 &&
                m_inputParams.channelCount == 1) {
            m_inputParams.channelCount = 2;
        }
        if (m_deviceInfo->maxOutputChannels >= 2 &&
                m_outputParams.channelCount == 1) {
            m_outputParams.channelCount = 2;
        }
    }


    // Sample rate
    if (m_dSampleRate <= 0) {
        m_dSampleRate = 44100.0;
    }

    // Get latency in milleseconds
    qDebug() << "framesPerBuffer:" << m_framesPerBuffer;
    double bufferMSec = m_framesPerBuffer / m_dSampleRate * 1000;
    qDebug() << "Requested sample rate: " << m_dSampleRate << "Hz, latency:" << bufferMSec << "ms";

    qDebug() << "Output channels:" << m_outputParams.channelCount << "| Input channels:"
        << m_inputParams.channelCount;

    // PortAudio's JACK backend also only properly supports
    // paFramesPerBufferUnspecified in non-blocking mode because the latency
    // comes from the JACK daemon. (PA should give an error or something though,
    // but it doesn't.)
    if (m_deviceInfo->hostApi == paJACK) {
        m_framesPerBuffer = paFramesPerBufferUnspecified;
    }

    //Fill out the rest of the info.
    m_outputParams.device = m_devId;
    m_outputParams.sampleFormat = paFloat32;
    m_outputParams.suggestedLatency = bufferMSec / 1000.0;
    m_outputParams.hostApiSpecificStreamInfo = NULL;

    m_inputParams.device  = m_devId;
    m_inputParams.sampleFormat  = paFloat32;
    m_inputParams.suggestedLatency = bufferMSec / 1000.0;
    m_inputParams.hostApiSpecificStreamInfo = NULL;

    qDebug() << "Opening stream with id" << m_devId;

    //Create the callback function pointer.
    PaStreamCallback* callback = NULL;
    if (registerCallback) {
        callback = paV19Callback;
    } else {
        if (m_outputParams.channelCount) {
            // PA allocates the double amount of buffer so we need it too;
            m_pOutputBuffer = SampleUtil::alloc(
                    m_outputParams.channelCount * m_framesPerBuffer * 2);
            SampleUtil::clear(m_pOutputBuffer,
                    m_outputParams.channelCount * m_framesPerBuffer * 2);
            m_pOutputBufferReadFrame = 0;
        }
        if (m_inputParams.channelCount) {
            m_pInputBuffer = SampleUtil::alloc(
                    m_inputParams.channelCount * m_framesPerBuffer);
        }
    }

    PaStream *pStream;
    // Try open device using iChannelMax
    err = Pa_OpenStream(&pStream,
                        pInputParams,
                        pOutputParams,
                        m_dSampleRate,
                        m_framesPerBuffer,
                        paClipOff, // Stream flags
                        callback,
                        (void*) this); // pointer passed to the callback function

    if (err != paNoError) {
        qWarning() << "Error opening stream:" << Pa_GetErrorText(err);
        m_lastError = QString::fromUtf8(Pa_GetErrorText(err));
        return ERR;
    } else {
        qDebug() << "Opened PortAudio stream successfully... starting";
    }


#ifdef __LINUX__
    //Attempt to dynamically load and resolve stuff in the PortAudio library
    //in order to enable RT priority with ALSA.
    QLibrary portaudio("libportaudio.so.2");
    if (!portaudio.load())
       qWarning() << "Failed to dynamically load PortAudio library";
    else
       qDebug() << "Dynamically loaded PortAudio library";

    EnableAlsaRT enableRealtime = (EnableAlsaRT) portaudio.resolve("PaAlsa_EnableRealtimeScheduling");
    if (enableRealtime) {
        enableRealtime(pStream, 1);
    }
    portaudio.unload();
#endif

    // Start stream
    err = Pa_StartStream(pStream);
    if (err != paNoError) {
        qWarning() << "PortAudio: Start stream error:" << Pa_GetErrorText(err);
        m_lastError = QString::fromUtf8(Pa_GetErrorText(err));
        err = Pa_CloseStream(pStream);
        if (err != paNoError) {
            qWarning() << "PortAudio: Close stream error:" << Pa_GetErrorText(err) << getInternalName();
        }
        return ERR;
    } else {
        qDebug() << "PortAudio: Started stream successfully";
    }

    // Get the actual details of the stream & update Mixxx's data
    const PaStreamInfo* streamDetails = Pa_GetStreamInfo(pStream);
    m_dSampleRate = streamDetails->sampleRate;
    double currentLatencyMSec = streamDetails->outputLatency * 1000;
    qDebug() << "   Actual sample rate: " << m_dSampleRate << "Hz, latency:" << currentLatencyMSec << "ms";

    // Update the samplerate and latency ControlObjects, which allow the
    // waveform view to properly correct for the latency.
    ControlObject::set(ConfigKey("[Master]", "latency"), currentLatencyMSec);
    ControlObject::set(ConfigKey("[Master]", "samplerate"), m_dSampleRate);
    ControlObject::set(ConfigKey("[Master]", "audio_buffer_size"), bufferMSec);

    if (m_pMasterUnderflowCount) {
        m_pMasterUnderflowCount->set(0);
    }
    m_pStream = pStream;
    return OK;
}

int SoundDevicePortAudio::close() {
    //qDebug() << "SoundDevicePortAudio::close()" << getInternalName();
    PaStream* pStream = m_pStream;
    m_pStream = NULL;
    if (pStream) {
        // Make sure the stream is not stopped before we try stopping it.
        PaError err = Pa_IsStreamStopped(pStream);
        // 1 means the stream is stopped. 0 means active.
        if (err == 1) {
            qDebug() << "PortAudio: Stream already stopped, but no error.";
            return 1;
        }
        // Real PaErrors are always negative.
        if (err < 0) {
            qWarning() << "PortAudio: Stream already stopped:"
                       << Pa_GetErrorText(err) << getInternalName();
            return 1;
        }

        //Stop the stream.
        err = Pa_StopStream(pStream);
        //PaError err = Pa_AbortStream(m_pStream); //Trying Pa_AbortStream instead, because StopStream seems to wait
                                                   //until all the buffers have been flushed, which can take a
                                                   //few (annoying) seconds when you're doing soundcard input.
                                                   //(it flushes the input buffer, and then some, or something)
                                                   //BIG FAT WARNING: Pa_AbortStream() will kill threads while they're
                                                   //waiting on a mutex, which will leave the mutex in an screwy
                                                   //state. Don't use it!

        if (err != paNoError) {
            qWarning() << "PortAudio: Stop stream error:" << Pa_GetErrorText(err) << getInternalName();
            return 1;
        }

        // Close stream
        err = Pa_CloseStream(pStream);
        if (err != paNoError) {
            qWarning() << "PortAudio: Close stream error:" << Pa_GetErrorText(err) << getInternalName();
            return 1;
        }

        if (m_pOutputBuffer) {
            SampleUtil::free(m_pOutputBuffer);
        }
        if (m_pInputBuffer) {
            SampleUtil::free(m_pInputBuffer);
        }
    }

    m_pOutputBuffer = NULL;
    m_pInputBuffer = NULL;
    m_bSetThreadPriority = false;

    return 0;
}

void SoundDevicePortAudio::readProcess() {
    PaStream* pStream = m_pStream;
    if (pStream) {
        if (m_inputParams.channelCount) {
//          signed int readAvailable = Pa_GetStreamReadAvailable(m_pStream);
//          qDebug() << "SoundDevicePortAudio::readProcess()" << readAvailable;
        }
     }
}

void SoundDevicePortAudio::writeProcess() {
    PaStream* pStream = m_pStream;
    if (pStream) {
        if (m_outputParams.channelCount) {
            int writeAvailable = Pa_GetStreamWriteAvailable(pStream);
            if (m_pOutputBufferReadFrame != m_framesPerBuffer) {
                qDebug() << "SoundDevicePortAudio::writeProcess()" << writeAvailable << m_pOutputBufferReadFrame;
                if (m_pOutputBufferReadFrame < m_framesPerBuffer / 2) {
                    // risk of overflow skip one frame
                    m_pOutputBufferReadFrame++;
                } else if (m_pOutputBufferReadFrame > m_framesPerBuffer / 2 * 3) {
                    // risk of underflow duplicate one frame
                    memcpy(getOutputBufferFrame(m_pOutputBufferReadFrame - 1),
                            getOutputBufferFrame(m_pOutputBufferReadFrame),
                            m_outputParams.channelCount * sizeof(CSAMPLE));
                    m_pOutputBufferReadFrame--;
                }
            }
            if (writeAvailable > 0) {
                if (writeAvailable == (int)(m_framesPerBuffer * 2)) {
                    // Underflow try to catch up with Zeros, clear fist half of buffer
                    SampleUtil::clear(m_pOutputBuffer, m_outputParams.channelCount * m_framesPerBuffer);
                    m_pOutputBufferReadFrame = 0;
                }
                // Fetch fresh samples and write to the second half of the output buffer
                m_pSoundManager->pullBuffer(m_audioOutputs, getOutputBufferFrame(m_framesPerBuffer), m_framesPerBuffer,
                        static_cast<unsigned int>(m_outputParams.channelCount));
                int framesCountWrite = qMin(writeAvailable, (int)(m_framesPerBuffer * 2 - m_pOutputBufferReadFrame));
                PaError err = Pa_WriteStream(pStream, getOutputBufferFrame(m_pOutputBufferReadFrame), framesCountWrite);
                m_pOutputBufferReadFrame += framesCountWrite;
                if (err == paOutputUnderflowed) {
                    qDebug() << "SoundDevicePortAudio::writeProcess() Pa_WriteStream paOutputUnderflowed";
                } else if (err < 0) {
                    qDebug() << "SoundDevicePortAudio::writeProcess() Pa_WriteStream" << err;
                }
                if (m_pOutputBufferReadFrame > m_framesPerBuffer) {
                    if (m_framesPerBuffer * 2 > m_pOutputBufferReadFrame) {
                        // copy remaining frames to the fist half of the buffer
                        memcpy(getOutputBufferFrame(m_pOutputBufferReadFrame - m_framesPerBuffer),
                                getOutputBufferFrame(m_pOutputBufferReadFrame),
                                (m_framesPerBuffer * 2 - m_pOutputBufferReadFrame) * m_outputParams.channelCount * sizeof(CSAMPLE));
                    }
                    m_pOutputBufferReadFrame -= m_framesPerBuffer;
                } else {
                        // Overflow!
                        qDebug() << "SoundDevicePortAudio::writeProcess() Overflow";
                        // copy second half of the buffer to fist half
                        memcpy(getOutputBufferFrame(0),
                                getOutputBufferFrame(m_framesPerBuffer),
                                m_framesPerBuffer * m_outputParams.channelCount * sizeof(CSAMPLE));
                        m_pOutputBufferReadFrame = 0;
                }
            }
        }
    }
}

QString SoundDevicePortAudio::getError() const {
    return m_lastError;
}

int SoundDevicePortAudio::callbackProcess(const unsigned int framesPerBuffer,
                                          CSAMPLE *output, const CSAMPLE *in,
                                          const PaStreamCallbackTimeInfo *timeInfo,
                                          PaStreamCallbackFlags statusFlags) {
    Trace trace("SoundDevicePortAudio::callbackProcess " + getInternalName());

    //qDebug() << "SoundDevicePortAudio::callbackProcess:" << getInternalName();
    // Turn on TimeCritical priority for the callback thread. If we are running
    // in Linux userland, for example, this will have no effect.
    if (!m_bSetThreadPriority) {
        QThread::currentThread()->setPriority(QThread::TimeCriticalPriority);
        m_bSetThreadPriority = true;
    }

    if (m_pSoundManager->isDeviceClkRef(this)) {
        VisualPlayPosition::setTimeInfo(timeInfo);
    }

    if (!m_underflowUpdateCount) {
        if (statusFlags & (paOutputUnderflow | paInputOverflow)) {
            if (m_pMasterUnderflowCount) {
                m_pMasterUnderflowCount->set(
                    m_pMasterUnderflowCount->get() + 1);
            }
            m_underflowUpdateCount = 40;
        }
    } else {
        m_underflowUpdateCount--;
    }

    //Note: Input is processed first so that any ControlObject changes made in
    //      response to input are processed as soon as possible (that is, when
    //      m_pSoundManager->requestBuffer() is called below.)

    // Send audio from the soundcard's input off to the SoundManager...
    if (in) {
        ScopedTimer t("SoundDevicePortAudio::callbackProcess input " + getInternalName());
        m_pSoundManager->pushBuffer(m_audioInputs, in, framesPerBuffer,
                                    m_inputParams.channelCount);
    }

    m_pSoundManager->readProcess();

    if (m_pSoundManager->isDeviceClkRef(this)) {
        ScopedTimer t("SoundDevicePortAudio::callbackProcess prepare " + getInternalName());
        m_pSoundManager->prepareBuffer(framesPerBuffer);
    }

    if (output) {
        ScopedTimer t("SoundDevicePortAudio::callbackProcess output " + getInternalName());

        if (m_outputParams.channelCount <= 0) {
            qWarning() << "SoundDevicePortAudio::callbackProcess m_outputParams channel count is zero or less:" << m_outputParams.channelCount;
            // Bail out.
            return paContinue;
        }

        m_pSoundManager->pullBuffer(m_audioOutputs, output, framesPerBuffer,
                static_cast<unsigned int>(m_outputParams.channelCount));
    }

    m_pSoundManager->writeProcess();

    return paContinue;
}

int paV19Callback(const void *inputBuffer, void *outputBuffer,
                  unsigned long framesPerBuffer,
                  const PaStreamCallbackTimeInfo *timeInfo,
                  PaStreamCallbackFlags statusFlags,
                  void *soundDevice) {
    return ((SoundDevicePortAudio*)soundDevice)->callbackProcess((unsigned int)framesPerBuffer,
            (CSAMPLE*)outputBuffer, (const CSAMPLE*)inputBuffer, timeInfo, statusFlags);
}
