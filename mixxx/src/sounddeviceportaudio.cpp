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

#include <QtDebug>
#include <QtCore>
#include <portaudio.h>
#include <assert.h>
#include "controlobjectthreadmain.h"
#include "soundmanager.h"
#include "sounddevice.h"
#include "sounddeviceportaudio.h"
#include "soundmanagerutil.h"
#include "controlobject.h"
#include "visualplayposition.h"
#include "util/timer.h"

SoundDevicePortAudio::SoundDevicePortAudio(ConfigObject<ConfigValue> *config, SoundManager *sm,
                                           const PaDeviceInfo *deviceInfo, unsigned int devIndex)
        : SoundDevice(config, sm),
          m_bSetThreadPriority(false),
          m_pMasterUnderflowCount(ControlObject::getControl(ConfigKey("[Master]", "underflow_count"))),
          m_undeflowUpdateCount(0) {
    //qDebug() << "SoundDevicePortAudio::SoundDevicePortAudio()";
    m_deviceInfo = deviceInfo;
    m_devId = devIndex;
    m_hostAPI = Pa_GetHostApiInfo(deviceInfo->hostApi)->name;
    m_dSampleRate = deviceInfo->defaultSampleRate;
    m_strInternalName = QString("%1, %2").arg(QString::number(m_devId), deviceInfo->name);
    m_strDisplayName = QString(deviceInfo->name);

    m_pStream = 0;
    //m_devId = -1;
    m_iNumberOfBuffers = 2;
    m_iNumInputChannels = m_deviceInfo->maxInputChannels;
    m_iNumOutputChannels = m_deviceInfo->maxOutputChannels;
}

SoundDevicePortAudio::~SoundDevicePortAudio() {
}

int SoundDevicePortAudio::open()
{
    qDebug() << "SoundDevicePortAudio::open()" << this->getInternalName();
    PaError err;

    if (m_audioOutputs.empty() && m_audioInputs.empty()) {
        m_lastError = QString::fromAscii("No inputs or outputs in SDPA::open() "
            "(THIS IS A BUG, this should be filtered by SM::setupDevices)");
        return ERR;
    }

    memset(&m_outputParams, 0, sizeof(m_outputParams));
    memset(&m_inputParams, 0, sizeof(m_inputParams));
    PaStreamParameters * pOutputParams = &m_outputParams;
    PaStreamParameters * pInputParams = &m_inputParams;

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

    //Sample rate
    if (m_dSampleRate <= 0) {
        m_dSampleRate = 44100.0f;
    }

    //Get latency in milleseconds
    qDebug() << "framesPerBuffer:" << m_framesPerBuffer;
    double bufferMSec = m_framesPerBuffer / m_dSampleRate * 1000;
    qDebug() << "Requested sample rate: " << m_dSampleRate << "Hz, latency:" << bufferMSec << "ms";

    qDebug() << "Output channels:" << m_outputParams.channelCount << "| Input channels:"
        << m_inputParams.channelCount;

    /*
    //Calculate the latency in samples
    //Max channels opened for input or output
    int iMaxChannels = math_max(m_outputParams.channelCount, m_inputParams.channelCount);

    int iLatencySamples = (int)((float)(m_dSampleRate*iMaxChannels)/1000.f*(float)iLatencyMSec);

    //Round to the nearest multiple of 4.
    if (iLatencySamples % 4 != 0) {
        iLatencySamples -= (iLatencySamples % 4);
        iLatencySamples += 4;
    }

    qDebug() << "iLatencySamples:" << iLatencySamples;

    int iNumberOfBuffers = 2;
    //Apply simple rule to determine number of buffers
    if (iLatencySamples / MIXXXPA_MAX_FRAME_SIZE < 2)
        iNumberOfBuffers = 2;
    else
        iNumberOfBuffers = iLatencySamples / MIXXXPA_MAX_FRAME_SIZE;

    //Frame size...
    unsigned int iFramesPerBuffer = iLatencySamples/m_iNumberOfBuffers;
    */

    //PortAudio's JACK backend also only properly supports paFramesPerBufferUnspecified in non-blocking mode
    //because the latency comes from the JACK daemon. (PA should give an error or something though, but it doesn't.)
    if (m_hostAPI == MIXXX_PORTAUDIO_JACK_STRING) {
        m_framesPerBuffer = paFramesPerBufferUnspecified;
    }

    //Fill out the rest of the info.
    m_outputParams.device = m_devId;
    m_outputParams.sampleFormat = paFloat32;
    m_outputParams.suggestedLatency = bufferMSec / 1000.0;
    m_outputParams.hostApiSpecificStreamInfo = NULL;

    m_inputParams.device  = m_devId;
    m_inputParams.sampleFormat  = paInt16; //This is how our vinyl control stuff like samples.
    m_inputParams.suggestedLatency = bufferMSec / 1000.0;
    m_inputParams.hostApiSpecificStreamInfo = NULL;

    qDebug() << "Opening stream with id" << m_devId;

    //Create the callback function pointer.
    PaStreamCallback *callback = paV19Callback;

    // Try open device using iChannelMax
    err = Pa_OpenStream(&m_pStream,
                        pInputParams,
                        pOutputParams,
                        m_dSampleRate,
                        m_framesPerBuffer,
                        paClipOff, // Stream flags
                        callback,    
                        (void*) this); // pointer passed to the callback function

    if (err != paNoError)
    {
        qWarning() << "Error opening stream:" << Pa_GetErrorText(err);
        m_lastError = QString::fromUtf8(Pa_GetErrorText(err));
        m_pStream = 0;
        return ERR;
    }
    else
    {
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
    if (enableRealtime)
    {
        enableRealtime(m_pStream, 1);
    }
    portaudio.unload();
#endif

    // Start stream
    err = Pa_StartStream(m_pStream);
    if (err != paNoError)
    {
        qWarning() << "PortAudio: Start stream error:" << Pa_GetErrorText(err);
        m_lastError = QString::fromUtf8(Pa_GetErrorText(err));
        m_pStream = 0;
        return ERR;
    }
    else
        qDebug() << "PortAudio: Started stream successfully";

    // Get the actual details of the stream & update Mixxx's data
    const PaStreamInfo* streamDetails = Pa_GetStreamInfo(m_pStream);
    m_dSampleRate = streamDetails->sampleRate;
    double currentLatencyMSec = streamDetails->outputLatency * 1000;
    qDebug() << "   Actual sample rate: " << m_dSampleRate << "Hz, latency:" << currentLatencyMSec << "ms";

    //Update the samplerate and latency ControlObjects, which allow the waveform view to properly correct
    //for the latency.

    ControlObjectThreadMain* pControlObjectSampleRate =
        new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Master]","samplerate")));
    ControlObjectThreadMain* pControlObjectLatency =
        new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Master]","latency")));
    ControlObjectThreadMain* pControlObjectAudioBufferSize =
        new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Master]","audio_buffer_size")));

    pControlObjectLatency->slotSet(currentLatencyMSec);
    pControlObjectSampleRate->slotSet(m_dSampleRate);
    pControlObjectAudioBufferSize->slotSet(bufferMSec);

    //qDebug() << "SampleRate" << pControlObjectSampleRate->get();
    //qDebug() << "Latency" << pControlObjectLatency->get();

    delete pControlObjectLatency;
    delete pControlObjectSampleRate;
    delete pControlObjectAudioBufferSize;

    if (m_pMasterUnderflowCount) {
        ControlObjectThreadMain* pMasterUnderflowCount =
                new ControlObjectThreadMain(m_pMasterUnderflowCount);
        pMasterUnderflowCount->slotSet(0);
        delete pMasterUnderflowCount;
    }

    return OK;
}

int SoundDevicePortAudio::close()
{
    //qDebug() << "SoundDevicePortAudio::close()" << this->getInternalName();
    if (m_pStream)
    {
        //Make sure the stream is not stopped before we try stopping it.
        PaError err = Pa_IsStreamStopped(m_pStream);
        if (err == 1) //1 means the stream is stopped. 0 means active.
        {
            qDebug() << "PortAudio: Stream already stopped, but no error.";
            return 1;
        }
        if (err < 0) //Real PaErrors are always negative.
        {
            qWarning() << "PortAudio: Stream already stopped:" << Pa_GetErrorText(err) << getInternalName();
            return 1;
        }

        //Stop the stream.
        err = Pa_StopStream(m_pStream);
        //PaError err = Pa_AbortStream(m_pStream); //Trying Pa_AbortStream instead, because StopStream seems to wait
                                                   //until all the buffers have been flushed, which can take a
                                                   //few (annoying) seconds when you're doing soundcard input.
                                                   //(it flushes the input buffer, and then some, or something)
                                                   //BIG FAT WARNING: Pa_AbortStream() will kill threads while they're
                                                   //waiting on a mutex, which will leave the mutex in an screwy
                                                   //state. Don't use it!

        if( err != paNoError )
        {
            qWarning() << "PortAudio: Stop stream error:" << Pa_GetErrorText(err) << getInternalName();
            return 1;
        }

        // Close stream
        err = Pa_CloseStream(m_pStream);
        if( err != paNoError )
        {
            qWarning() << "PortAudio: Close stream error:" << Pa_GetErrorText(err) << getInternalName();
            return 1;
        }
    }

    m_pStream = 0;

    return 0;
}

QString SoundDevicePortAudio::getError() const {
    return m_lastError;
}

/** -------- ------------------------------------------------------
        Purpose: This callback function gets called everytime the sound device runs
                 out of samples (ie. when it needs more sound to play)
        -------- ------------------------------------------------------
 */
int SoundDevicePortAudio::callbackProcess(unsigned long framesPerBuffer,
        float *output, short *in, const PaStreamCallbackTimeInfo *timeInfo,
        PaStreamCallbackFlags statusFlags) {
    Q_UNUSED(timeInfo);
    ScopedTimer t("SoundDevicePortAudio::callbackProcess " + getInternalName());

    //qDebug() << "SoundDevicePortAudio::callbackProcess:" << getInternalName();

    static ControlObject* pControlObjectVinylControlGain =
        ControlObject::getControl(ConfigKey("[VinylControl]", "gain"));
    static const float SHRT_CONVERSION_FACTOR = 1.0f/SHRT_MAX;
    int iFrameSize = m_outputParams.channelCount;
    int iVCGain = 1;

    // Turn on TimeCritical priority for the callback thread. If we are running
    // in Linux userland, for example, this will have no effect.
    if (!m_bSetThreadPriority) {
        QThread::currentThread()->setPriority(QThread::TimeCriticalPriority);
        m_bSetThreadPriority = true;
    }

    VisualPlayPosition::setTimeInfo(timeInfo);
    if (!m_undeflowUpdateCount) {
        if (statusFlags & (paOutputUnderflow | paInputOverflow)) {
            if (m_pMasterUnderflowCount) {
                m_pMasterUnderflowCount->add(1);
            }
            m_undeflowUpdateCount = 40;
        }
    } else {
        m_undeflowUpdateCount--;
    }


    //Send audio from the soundcard's input off to the SoundManager...
    if (in && framesPerBuffer > 0)
    {
        ScopedTimer t("SoundDevicePortAudio::callbackProcess input " + getInternalName());
        //Note: Input is processed first so that any ControlObject changes made in response to input
        //      is processed as soon as possible (that is, when m_pSoundManager->requestBuffer() is
        //      called below.)

        //Apply software preamp
        //Super big warning: Need to use channel_count here instead of iFrameSize because iFrameSize is
        //only for output buffers...
        // TODO(bkgood) move this to vcproxy or something, once we have other
        // inputs we don't want every input getting the vc gain
        iVCGain = pControlObjectVinylControlGain->get();
        for (unsigned int i = 0; i < framesPerBuffer * m_inputParams.channelCount; ++i)
            in[i] *= iVCGain;

        //qDebug() << in[0];

        // TODO(bkgood) deinterlace here and send a hashmap of buffers to
        // soundmanager so we have all our deinterlacing in one place and
        // soundmanager gets simplified to boot

        m_pSoundManager->pushBuffer(m_audioInputs, in, framesPerBuffer,
                                    m_inputParams.channelCount);
    }

    if (output && framesPerBuffer > 0)
    {
        ScopedTimer t("SoundDevicePortAudio::callbackProcess output " + getInternalName());
        assert(iFrameSize > 0);
        QHash<AudioOutput, const CSAMPLE*> outputAudio
            = m_pSoundManager->requestBuffer(m_audioOutputs,
                    framesPerBuffer, this);

        // Reset sample for each open channel
        memset(output, 0, framesPerBuffer * iFrameSize * sizeof(*output));

        // Interlace Audio data onto portaudio buffer.  We iterate through the
        // source list to find out what goes in the buffer data is interlaced in
        // the order of the list

        for (QList<AudioOutput>::const_iterator i = m_audioOutputs.begin(),
                     e = m_audioOutputs.end(); i != e; ++i) {
            const AudioOutput &out = *i;
            const CSAMPLE* input = outputAudio[out];
            const ChannelGroup outChans = out.getChannelGroup();
            const int iChannelCount = outChans.getChannelCount();
            const int iChannelBase = outChans.getChannelBase();

            for (unsigned int iFrameNo=0; iFrameNo < framesPerBuffer; ++iFrameNo) {
                // this will make sure a sample from each channel is copied
                for (int iChannel = 0; iChannel < iChannelCount; ++iChannel) {
                    // iFrameBase is the "base sample" in a frame (ie. the first
                    // sample in a frame)
                    unsigned int iFrameBase = iFrameNo * iFrameSize;
                    unsigned int iLocalFrameBase = iFrameNo * iChannelCount;

                    // note that if QHash gets request for a value with a key it
                    // doesn't know, it will return a default value (NULL is the
                    // likely choice here), but the old system would've done
                    // something similar (it would have gone over the bounds of
                    // the array)

                    output[iFrameBase + iChannelBase + iChannel] +=
                            input[iLocalFrameBase + iChannel] * SHRT_CONVERSION_FACTOR;

                    //Input audio pass-through (useful for debugging)
                    //if (in)
                    //    output[iFrameBase + src.channelBase + iChannel] +=
                    //    in[iFrameBase + src.channelBase + iChannel] * SHRT_CONVERSION_FACTOR;
                }
            }
        }
    }

    return paContinue;
}

/* -------- ------------------------------------------------------
   Purpose: Wrapper function to call processing loop function,
            implemented as a method in a class. Used in PortAudio,
            which knows nothing about C++.
   Input:   .
   Output:  -
   -------- ------------------------------------------------------ */
int paV19Callback(const void *inputBuffer, void *outputBuffer,
                  unsigned long framesPerBuffer,
                  const PaStreamCallbackTimeInfo *timeInfo,
                  PaStreamCallbackFlags statusFlags,
                  void *soundDevice) {
    return ((SoundDevicePortAudio*) soundDevice)->callbackProcess(framesPerBuffer,
            (float*) outputBuffer, (short*) inputBuffer, timeInfo, statusFlags);
}
