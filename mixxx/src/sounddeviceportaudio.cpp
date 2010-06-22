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
#include "audiopath.h"

SoundDevicePortAudio::SoundDevicePortAudio(ConfigObject<ConfigValue> * config, SoundManager * sm,
                                           const PaDeviceInfo * deviceInfo, unsigned int devIndex)
        : SoundDevice(config, sm),
          m_bSetThreadPriority(false)
{
    //qDebug() << "SoundDevicePortAudio::SoundDevicePortAudio()";
    m_deviceInfo = deviceInfo;
    m_devId = devIndex;
    m_hostAPI = Pa_GetHostApiInfo(deviceInfo->hostApi)->name;
    m_dSampleRate = deviceInfo->defaultSampleRate;
    m_strInternalName = QString("%1, %2").arg(QString::number(m_devId)).arg(deviceInfo->name);
    m_strDisplayName = QString(deviceInfo->name);

    m_pStream = 0;
    //m_devId = -1;
    m_iNumberOfBuffers = 2;
    m_iNumInputChannels = m_deviceInfo->maxInputChannels;
    m_iNumOutputChannels = m_deviceInfo->maxOutputChannels;
}

SoundDevicePortAudio::~SoundDevicePortAudio()
{

}

int SoundDevicePortAudio::open()
{
    qDebug() << "SoundDevicePortAudio::open()" << this->getInternalName();
    PaError err;

    if (m_audioSources.empty() && m_audioReceivers.empty())
    {
        return -1;
    }

    memset(&m_outputParams, 0, sizeof(m_outputParams));
    memset(&m_inputParams, 0, sizeof(m_inputParams));
    PaStreamParameters * pOutputParams = &m_outputParams;
    PaStreamParameters * pInputParams = &m_inputParams;

    //Look at how many audio sources we have, so we can figure out how many output channels we need to open.
    if (m_audioSources.empty())
    {
        pOutputParams = NULL;
    }
    else
    {
        QListIterator<AudioSource> srcIt(m_audioSources);
        while (srcIt.hasNext())
        {
            AudioSource src = srcIt.next();
            ChannelGroup channelGroup = src.getChannelGroup();
            int highChannel = channelGroup.getChannelBase()
                + channelGroup.getChannelCount();
            if (m_outputParams.channelCount <= highChannel) {
                m_outputParams.channelCount = highChannel;
            }
        }
    }


    //Look at how many audio receivers we're connected to, so we can figure out how many input channels we need to open.
    if (m_audioReceivers.empty())
    {
        pInputParams = NULL;
    }
    else
    {
        QListIterator<AudioReceiver> recvIt(m_audioReceivers);
        while (recvIt.hasNext())
        {
            AudioReceiver recv = recvIt.next();
            ChannelGroup channelGroup = recv.getChannelGroup();
            int highChannel = channelGroup.getChannelBase()
                + channelGroup.getChannelCount();
            if (m_inputParams.channelCount <= highChannel) {
                m_inputParams.channelCount = highChannel;
            }
        }
    }


    //Sample rate
    m_dSampleRate = (double)m_pConfig->getValueString(ConfigKey("[Soundcard]","Samplerate")).toInt();
    if (m_dSampleRate <= 0)
        m_dSampleRate = 44100.0f;
    qDebug() << "m_dSampleRate" << m_dSampleRate;

    //Get latency in milleseconds
    int iLatencyMSec = m_pConfig->getValueString(ConfigKey("[Soundcard]","Latency")).toInt();
    if (iLatencyMSec <= 0)     //Make sure we don't get a crazy latency value.
        iLatencyMSec = 75;

    qDebug() << "iLatencyMSec:" << iLatencyMSec;
    qDebug() << "output channels:" << m_outputParams.channelCount << "| input channels:" << m_inputParams.channelCount;

    //Calculate the latency in samples
    int iMaxChannels = math_max(m_outputParams.channelCount, m_inputParams.channelCount); //Max channels opened for input or output

    /*
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

    //Drastically simplified frames per buffer calculation. PortAudio-v19 uses the
    //suggested latency field in input/output params to figure out the number of
    //buffers for us.
    unsigned int iFramesPerBuffer = ((float)iLatencyMSec/1000.0f)*m_dSampleRate;

    //Round to the nearest power-of-two buffer size. This is improves compatibility with
    //soundcards (and buggy drivers and APIs like ALSA/PulseAudio that can crash), and also
    //seems to give us lower latencies. Win-win. :)
    unsigned int i;
    iFramesPerBuffer &= INT_MAX;
    for (i = 1; iFramesPerBuffer > i; i <<= 1) ;
    iFramesPerBuffer = i;
    qDebug() << "iFramesPerBuffer" << iFramesPerBuffer;

    //PortAudio's JACK backend also only properly supports paFramesPerBufferUnspecified in non-blocking mode
    //because the latency comes from the JACK daemon. (PA should give an error or something though, but it doesn't.)
    if (m_pConfig->getValueString(ConfigKey("[Soundcard]","SoundApi")) == MIXXX_PORTAUDIO_JACK_STRING)
    {
        iFramesPerBuffer = paFramesPerBufferUnspecified;
    }


    //Fill out the rest of the info.
    m_outputParams.device = m_devId;
    m_outputParams.sampleFormat = paFloat32;
    m_outputParams.suggestedLatency = ((float)iLatencyMSec) / 1000.0f;
    m_outputParams.hostApiSpecificStreamInfo = NULL;

    m_inputParams.device  = m_devId;
    m_inputParams.sampleFormat  = paInt16; //This is how our vinyl control stuff like samples.
    m_inputParams.suggestedLatency = ((float)iLatencyMSec) / 1000.0f;
    m_inputParams.hostApiSpecificStreamInfo = NULL;

    qDebug() << "iLatencyMSec:" << iLatencyMSec;

    m_callbackStuff.soundDevice = this;
    m_callbackStuff.devIndex = m_devId; //FIXME: No longer necessary?

    qDebug() << "Opening stream with id" << m_devId;

    //Create the callback function pointer.
    PaStreamCallback * callback = paV19Callback;

    // Try open device using iChannelMax
    err = Pa_OpenStream(&m_pStream,
                        pInputParams,                           // Input parameters
                        pOutputParams,                      // Output parameters
                        m_dSampleRate,                      // Sample rate
                        iFramesPerBuffer,                       // Frames per buffer
                        paClipOff,                                      // Stream flags
                        callback,                                       // Stream callback
                        (void *)&m_callbackStuff);                //Data pointer passed to the callback function

    if (err != paNoError)
    {
        qDebug() << "Error opening stream:" << Pa_GetErrorText(err);
        m_pStream = 0;
        return 1;
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
       qDebug() << "Failed to dynamically load PortAudio library";
    else
       qDebug() << "Dynamically loaded PortAudio library!";

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
        qDebug() << "PortAudio: Start stream error:" << Pa_GetErrorText(err);
        m_pStream = 0;
        return 1;
    }
    else
        qDebug() << "PortAudio: Started stream successfully";

    //Update the samplerate and latency ControlObjects, which allow the waveform view to properly correct
    //for the latency.

    ControlObjectThreadMain* pControlObjectSampleRate = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Master]","samplerate")));
    ControlObjectThreadMain* pControlObjectLatency = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Master]","latency")));

    //The latency ControlObject value MUST BE ZERO, otherwise the waveform view gets out of whack.
    //Yes, this is confusing. Fortunately, the latency ControlObject is ONLY used in the waveform view
    //code. Here's my theory of what's happened: There's some code in the waveform view (visualbuffer.cpp) to
    //adjust the waveform for the latency, so it always lines up perfectly with the audio. I don't think that
    //code was ever properly finished though, which is why we need this hack. So, fixing the waveform code is
    //a TODO:
    pControlObjectLatency->slotSet(0);

    pControlObjectSampleRate->slotSet(m_dSampleRate);
    //qDebug() << "SampleRate" << pControlObjectSampleRate->get();
    //qDebug() << "Latency" << pControlObjectLatency->get();

    return 0;
}

int SoundDevicePortAudio::close()
{
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
            qDebug() << "PortAudio: Stream already stopped:" << Pa_GetErrorText(err) << getInternalName();
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
            qDebug() << "PortAudio: Stop stream error:" << Pa_GetErrorText(err) << getInternalName();
            return 1;
        }

        // Close stream
        err = Pa_CloseStream(m_pStream);
        if( err != paNoError )
        {
            qDebug() << "PortAudio: Close stream error:" << Pa_GetErrorText(err) << getInternalName();
            return 1;
        }
    }

    m_pStream = 0;

    return 0;
}


/** -------- ------------------------------------------------------
        Purpose: This callback function gets called everytime the sound device runs
                 out of samples (ie. when it needs more sound to play)
        -------- ------------------------------------------------------
 */
int SoundDevicePortAudio::callbackProcess(unsigned long framesPerBuffer, float *output, short *in, int devIndex)
{
    //qDebug() << "SoundDevicePortAudio::callbackProcess";
    int iFrameSize;
    int iVCGain;
    int i;
    static ControlObject* pControlObjectVinylControlGain = ControlObject::getControl(ConfigKey("[VinylControl]", "VinylControlGain"));
    static const float SHRT_CONVERSION_FACTOR = 1.0f/SHRT_MAX;

    //Initialize some variables.
    iFrameSize = m_outputParams.channelCount;
    iVCGain = 1;
    i = 0;

    // Turn on TimeCritical priority for the callback thread. If we are running
    // in Linux userland, for example, this will have no effect.
    if(!m_bSetThreadPriority) {
        QThread::currentThread()->setPriority(QThread::TimeCriticalPriority);
        m_bSetThreadPriority = true;
    }

    //Send audio from the soundcard's input off to the SoundManager...
    if (in && framesPerBuffer > 0)
    {
        //Note: Input is processed first so that any ControlObject changes made in response to input
        //      is processed as soon as possible (that is, when m_pSoundManager->requestBuffer() is
        //      called below.)

        //Apply software preamp
        //Super big warning: Need to use channel_count here instead of iFrameSize because iFrameSize is
        //only for output buffers...
        iVCGain = pControlObjectVinylControlGain->get();
        for (i=0; i < framesPerBuffer*m_inputParams.channelCount; i++)
            in[i] *= iVCGain;

        //qDebug() << in[0];

        m_pSoundManager->pushBuffer(m_audioReceivers, in, framesPerBuffer, m_inputParams.channelCount);
    }

    if (output && framesPerBuffer > 0)
    {
        assert(iFrameSize > 0);
        QHash<AudioSource, const CSAMPLE*> outputAudio
            = m_pSoundManager->requestBuffer(m_audioSources, framesPerBuffer);

        //qDebug() << framesPerBuffer;

        //Reset sample for each open channel
        memset(output, 0, framesPerBuffer * iFrameSize * sizeof(*output));

        //iFrameBase is the "base sample" in a frame (ie. the first sample in a frame)

        for (unsigned int iFrameBase=0; iFrameBase < framesPerBuffer*iFrameSize; iFrameBase += iFrameSize)
        {
            //Interlace Audio data onto portaudio buffer
            //We iterate through the source list to find out what goes in the buffer
            //data is interlaced in the order of the list
            QListIterator<AudioSource> devItr(m_audioSources);
            int iChannel;
            while (devItr.hasNext())
            {
                AudioSource src = devItr.next();
                ChannelGroup srcChans = src.getChannelGroup();
                int iLocalFrameBase = (iFrameBase/iFrameSize) * srcChans.getChannelCount();
                for (iChannel = 0; iChannel < srcChans.getChannelCount(); iChannel++) //this will make sure a sample from each channel is copied
                {
                    // note that if QHash gets request for a value with a key it doesn't know, it
                    // will return a default value (NULL is the likely choice here), but the old system
                    // would've done something similar (it would have gone over the bounds of the array)
                    output[iFrameBase + srcChans.getChannelBase() + iChannel] += outputAudio[src][iLocalFrameBase + iChannel] * SHRT_CONVERSION_FACTOR;
                    //Input audio pass-through (useful for debugging)
                    //if (in)
                    //    output[iFrameBase + src.channelBase + iChannel] += in[iFrameBase + src.channelBase + iChannel] * SHRT_CONVERSION_FACTOR;
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
int paV19Callback(const void * inputBuffer, void * outputBuffer,
                  unsigned long framesPerBuffer,
                  const PaStreamCallbackTimeInfo * timeInfo,
                  PaStreamCallbackFlags statusFlags,
                  void * _callbackStuff)
{
    /*
       //Variables that are used in the human-readable form of function call from hell (below).
       static PlayerPortAudio* _player;
       static int devIndex;
       _player = ((PAPlayerCallbackStuff*)_callbackStuff)->player;
       devIndex = ((PAPlayerCallbackStuff*)_callbackStuff)->devIndex;
     */

    //Human-readable form of the function call from hell:
    //return _player->callbackProcess(framesPerBuffer, (float *)outputBuffer, devIndex);

    //Function call from hell:
    return ((PADeviceCallbackStuff *)_callbackStuff)->soundDevice->callbackProcess(framesPerBuffer, (float *)outputBuffer, (short *)inputBuffer, ((PADeviceCallbackStuff *)_callbackStuff)->devIndex);
}

