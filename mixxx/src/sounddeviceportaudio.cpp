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
#include "sounddeviceportaudio.h"
#include "soundmanager.h"

SoundDevicePortAudio::SoundDevicePortAudio(ConfigObject<ConfigValue> *config, SoundManager* sm, const PaDeviceInfo *deviceInfo,
                                            unsigned int devIndex) 
                                           : SoundDevice(config, sm)
{
    //qDebug() << "SoundDevicePortAudio::SoundDevicePortAudio()";
    m_deviceInfo = deviceInfo;
    m_devId = devIndex;
    m_hostAPI = Pa_GetHostApiInfo(deviceInfo->hostApi)->name    ;
    m_dSampleRate = deviceInfo->defaultSampleRate;
    m_strName = deviceInfo->name;
    
    m_pStream = 0;
    //m_devId = -1;
    m_iNumberOfBuffers = 2;
    
}

SoundDevicePortAudio::~SoundDevicePortAudio()
{

}

int SoundDevicePortAudio::open()
{
    qDebug() << "SoundDevicePortAudio::open()" << this->getName();
    PaError err;
    
    if (m_audioSources.empty() && m_audioReceivers.empty())
    {
        return -1;
    }
    
    memset(&m_outputParams, 0, sizeof(m_outputParams));
    memset(&m_inputParams, 0, sizeof(m_inputParams));
    PaStreamParameters* pOutputParams = &m_outputParams;
    PaStreamParameters* pInputParams = &m_inputParams;  
            			
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
            m_outputParams.channelCount += 2;
            srcIt.next();
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
            m_inputParams.channelCount += 2;
            recvIt.next();
        }
    }


    //Sample rate
    m_dSampleRate = (double)m_pConfig->getValueString(ConfigKey("[Soundcard]","Samplerate")).toInt();
	if (m_dSampleRate <= 0)
		m_dSampleRate = 44100.0f;
	qDebug() << "m_dSampleRate" << m_dSampleRate;

    //Get latency in milleseconds
    int iLatencyMSec = m_pConfig->getValueString(ConfigKey("[Soundcard]","Latency")).toInt();
	if (iLatencyMSec <= 0) //Make sure we don't get a crazy latency value.
		iLatencyMSec = 75;

    qDebug() << "iLatencyMSec:" << iLatencyMSec;
    qDebug() << "output channels:" << m_outputParams.channelCount << "| input channels:" << m_inputParams.channelCount;

    //Calculate the latency in samples
    int iLatencySamples = (int)((float)(m_dSampleRate*m_outputParams.channelCount)/1000.f*(float)iLatencyMSec);
    
    //Round to the nearest multiple of 4.
    iLatencySamples -= (iLatencySamples % 4);
	iLatencySamples += 4;

	qDebug() << "iLatencySamples:" << iLatencySamples;
    
    int iNumberOfBuffers = 2;
    //Apply simple rule to determine number of buffers
    if (iLatencySamples / MIXXXPA_MAX_FRAME_SIZE < 2)
        iNumberOfBuffers = 2;
    else
        iNumberOfBuffers = iLatencySamples / MIXXXPA_MAX_FRAME_SIZE;

    //Frame size...    
    unsigned int iFramesPerBuffer = iLatencySamples/m_iNumberOfBuffers;

    //OSS is a bitch - It only likes power-of-two buffer sizes, so give it one.
    if (m_pConfig->getValueString(ConfigKey("[Soundcard]","SoundApi")) == "OSS")
    {
        unsigned int i;
        iFramesPerBuffer &= INT_MAX;
        for (i = 1; iFramesPerBuffer > i; i <<= 1) ;
        iFramesPerBuffer = i;
        qDebug() << "iFramesPerBuffer" << iFramesPerBuffer;
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
    PaStreamCallback *callback = paV19Callback;

	// Try open device using iChannelMax
    err = Pa_OpenStream(&m_pStream,
    					pInputParams,		// Input parameters 
    					pOutputParams, 	    // Output parameters
    					m_dSampleRate,	    // Sample rate
    					iFramesPerBuffer,	// Frames per buffer
    					paClipOff,			// Stream flags
    					callback,			// Stream callback
    					(void*)&m_callbackStuff); //Data pointer passed to the callback function

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
    
    ControlObject* pControlObjectSampleRate = ControlObject::getControl(ConfigKey("[Master]","samplerate"));
    ControlObject* pControlObjectLatency = ControlObject::getControl(ConfigKey("[Master]","latency"));
    qDebug() << "SampleRate" << pControlObjectSampleRate->get();
    qDebug() << "Latency" << pControlObjectLatency->get();
    
    
    //The latency ControlObject value MUST BE ZERO, otherwise the waveform view gets out of whack. 
    //Yes, this is confusing. Fortunately, the latency ControlObject is ONLY used in the waveform view
    //code. Here's my theory of what's happened: There's some code in the waveform view (visualbuffer.cpp) to
    //adjust the waveform for the latency, so it always lines up perfectly with the audio. I don't think that
    //code was ever properly finished though, which is why we need this hack. So, fixing the waveform code is
    //a TODO:
    pControlObjectLatency->queueFromThread(0);  
    
    pControlObjectSampleRate->set(m_dSampleRate);
    qDebug() << "SampleRate" << pControlObjectSampleRate->get();
    qDebug() << "Latency" << pControlObjectLatency->get();    
    
    return 0;
}

int SoundDevicePortAudio::close()
{
    if (m_pStream)
    {
        //PaError err = Pa_StopStream(m_pStream);
        PaError err = Pa_AbortStream(m_pStream); //Trying Pa_AbortStream instead, because StopStream waits until all
                                                 //the buffer have been flushed, which can take a few (annoying) seconds
                                                 //when you're doing soundcard input. (not sure why)
        
        if( err != paNoError )
        {
            qDebug() << "PortAudio: Stop stream error:" << Pa_GetErrorText(err) << getName();
            return 1;    
        }

        // Close stream
        err = Pa_CloseStream(m_pStream);
        if( err != paNoError )
        {
            qDebug() << "PortAudio: Close stream error:" << Pa_GetErrorText(err) << getName();
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
    int iFrameSize = m_audioSources.count()*2;
    int i = 0;
    CSAMPLE* outputAudio = m_pSoundManager->requestBuffer(m_audioSources, framesPerBuffer); 
    
    //Reset sample for each open channel
    for (i=0; i < framesPerBuffer * iFrameSize; i++)
        output[i] = 0.;    
    
    for (i=0; i < framesPerBuffer* iFrameSize; i += iFrameSize)
    {
        //TODO: Instead of having if statements here, just make a loop...
        if (m_audioSources.count() == 1)
        {
            output[i] += outputAudio[i]/32768.;
            output[i+1] += outputAudio[i+1]/32768.;
        }
        else if (m_audioSources.count() == 2)
        {
            output[i] += outputAudio[i]/32768.;
            output[i+1] += outputAudio[i+1]/32768.;
            output[i+2] += outputAudio[i+2]/32768.;
            output[i+3] += outputAudio[i+3]/32768.;          
        }
        else
        {
            qDebug() << "Wierd number of audio sources in PA callback:" << m_audioSources.count();
        }

        //TODO: Figure out why we use /32768's above, considering that we're dealing with floats...
        //      (it sounds crazy if we don't though...)
    }
    
    //Send audio from the soundcard's input off to the SoundManager...
    if (in)
    {
        m_pSoundManager->pushBuffer(m_audioReceivers, in, framesPerBuffer);
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
                      const PaStreamCallbackTimeInfo* timeInfo,
                      PaStreamCallbackFlags statusFlags,
                      void *_callbackStuff)
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
	return ((PADeviceCallbackStuff*)_callbackStuff)->soundDevice->callbackProcess(framesPerBuffer, (float *)outputBuffer, (short*)inputBuffer, ((PADeviceCallbackStuff*)_callbackStuff)->devIndex);
}

