/***************************************************************************
                          playerportaudio.cpp  -  description
                             -------------------
    begin                : Wed Feb 20 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "playerportaudio.h"

int bufferIdxSlave = 0;

PlayerPortAudio::PlayerPortAudio(int size, std::vector<EngineObject *> *engines, QString device, int chMaster, int chHead) : Player(size, engines, device)
{
    PaError err = Pa_Initialize();
    if( err != paNoError )
        qFatal("PortAudio: Initialization error");

    // Default device ID.
    PaDeviceID id = -1;

    const PaDeviceInfo *devInfo;

    // Fill out devices list with info about available devices if list is empty
    if (devices.isEmpty())
    {
        int no = Pa_CountDevices();
        for (int i=0; i<no; i++)
        {
            devInfo = Pa_GetDeviceInfo(i);

            // Add the device if it is an output device:
            if (devInfo!=0 && devInfo->maxOutputChannels > 0)
            {
//              qDebug("PortAudio: Name: %s, ID: %i, MaxOutput %i",devInfo->name,i,devInfo->maxOutputChannels);

                // Add new PlayerInfo object to devices list
                Player::Info *p = new Player::Info;
                devices.append(p);

                // ID & Name
                p->name = QString(devInfo->name);
                p->id = i;

                // Check for default device
                if (p->name == device)
                    id = i;

                // Sample rates and latency
                if (devInfo->numSampleRates != -1)
                {
                    for (int j=0; j<devInfo->numSampleRates; j++)
                    {
                        p->sampleRates.append((int)devInfo->sampleRates[j]);

                        // Get minimum latency for sample rate
                        p->latency.append(minLatency((int)devInfo->sampleRates[j]));
//                        qDebug("SRATE: %i, Latency: %i",(int)devInfo->sampleRates[j],minLatency((int)devInfo->sampleRates[j]));
                    }
                }
                else
                {
                    // If we're just given a range of samplerates, then just
                    // assume some standard rates:
                    p->sampleRates.append(11025); p->latency.append(minLatency(11025));
                    p->sampleRates.append(22050); p->latency.append(minLatency(22050));
                    p->sampleRates.append(44100); p->latency.append(minLatency(44100));
                    p->sampleRates.append(48000); p->latency.append(minLatency(48000));
                    p->sampleRates.append(96000); p->latency.append(minLatency(96000));
                }
                                                                                                                                
                // Bits
                if (devInfo->nativeSampleFormats & paInt8)        p->bits.append(8);
                if (devInfo->nativeSampleFormats & paInt16)       p->bits.append(16);
                //if (devInfo->nativeSampleFormats & paPackedInt24) p->bits.append(24);
                if (devInfo->nativeSampleFormats & paInt24)       p->bits.append(24);
                if (devInfo->nativeSampleFormats & paInt32)       p->bits.append(32);

                // Number of available channels
                p->noChannels = devInfo->maxOutputChannels;
            }
        }
    }
    
    // Get id of default playback device if ID of device was not found in previous loop
    if (id<0)
        id = Pa_GetDefaultOutputDeviceID();
 
    devInfo = Pa_GetDeviceInfo(id);
//    qDebug("PortAudio: Device name %s",devInfo->name);

    // Ensure requested number of channels is supported
    int channels = devInfo->maxOutputChannels;
    if (channels < max(chMaster,chHead)+1)
        qFatal("PortAudio: Not enough channels available on output device, only %i is supported.",channels);

    // Set sample rate to 44100 if possible, otherwise highest possible
    int temp_sr = 0;
    if (devInfo->numSampleRates>0)
    {
        for (int i=0; i<=devInfo->numSampleRates; i++)
            if (devInfo->sampleRates[i] == 44100.)
                temp_sr = 44100;
    }
    else
        temp_sr = 44100;
                
    if (temp_sr == 0)
        temp_sr = (int)devInfo->sampleRates[devInfo->numSampleRates-1];
    if (!open(QString(devInfo->name),temp_sr,16,size,chMaster,chHead))
        qFatal("PortAudio: Error opening device");
}

PlayerPortAudio::~PlayerPortAudio()
{
    Pa_Terminate();
}

bool PlayerPortAudio::open(QString name, int srate, int bits, int bufferSize, int chMaster, int chHead)
{
    // Adjust bufferSize and number of buffers
    int bufferNo   = 2;
    bufferSize = bufferSize/bufferNo;


    // Extract bit information
    PaSampleFormat format = 0;
    switch (bits)
    {
        case 8:  format = paInt8; break;
        case 16: format = paInt16; break;
        case 24: format = paInt24; break;
        case 32: format = paInt32; break;
        default: qFatal("PortAudio: Sample format not supported (%i bits)", bits); return false;
    }

    // Extract device information
    unsigned int id = 0;
    unsigned int i;
    for (i=0; i<devices.count(); i++)
        if (name == devices.at(i)->name && chMaster+chHead <= devices.at(i)->noChannels)
        {
            id = devices.at(i)->id;
            break;
        }

    // Get number of channels to open
    int chNo = max(chMaster,chHead)+1;
    
    // Verify srate and bufferSize 
    unsigned int j = 0;
    while (j<devices.at(i)->sampleRates.count() && *devices.at(i)->sampleRates.at(j) != srate)
    { j++; qDebug("j: %i",j); }
    if (j>=devices.at(i)->sampleRates.count())
    {
         j=0;
         srate = *devices.at(i)->sampleRates.at(j);
    }
    bufferSize = max(*devices.at(i)->latency.at(j),bufferSize);
    qDebug("PortAudio: Latency %i samples",bufferSize);

    // Determine which callback function to use
    PortAudioCallback *callbackFunc;
    if (chMaster>0)
        callbackFunc = paCallback;
    else
    {
        callbackFunc = paCallbackSlave;
        bufferIdxSlave = 0;
    }

    // Try to open device 5 times before giving up!
    PaError err = 0;
    {for (int i=0; i<5; i++)
    {
      err = Pa_OpenStream(&stream,
                        paNoDevice,         // no input device
                        0,                  // no input
                        format,
                        NULL,
                        id,                 // output device
                        chNo,
                        format,
                        NULL,
                        (double)srate,
                        bufferSize/chNo,    // frames per buffer per channel
                        bufferNo,           // number of buffers, if zero then use default minimum
                        paClipOff,          // we won't output out of range samples so don't bother clipping them
                        callbackFunc,
                        this );

        if (err == paNoError)
            break;
    }}
    if( err != paNoError )
    {
        qFatal("PortAudio: Open stream error: %s", Pa_GetErrorText(err));
        return false;
    }

    qDebug("Number of channels: %i",chNo);
    
    // Fill in active config information
    setParams(QString(Pa_GetDeviceInfo(id)->name), srate, 16, bufferSize, chMaster, chHead);

    allocate();

    return true;
}

void PlayerPortAudio::close()
{
    PaError err = Pa_CloseStream( stream );
    if( err != paNoError )
        qFatal("PortAudio: Close stream error: %s", Pa_GetErrorText(err));

    deallocate();
}

void PlayerPortAudio::start()
{
    Player::start();

    PaError err = Pa_StartStream(stream);
    if (err != paNoError)
        qFatal("PortAudio: Start stream error: %s", Pa_GetErrorText(err));
}

void PlayerPortAudio::wait()
{
}

void PlayerPortAudio::stop()
{
    PaError err = Pa_StopStream( stream );
    if( err != paNoError )
        exit(-1);
}

int PlayerPortAudio::minLatency(int SRATE)
{
    // Initial minimum parameters
    int l = 16384;

    for (int buffersize=1; buffersize<16384; buffersize++)
    {
        int bufno = Pa_GetMinNumBuffers(buffersize, SRATE);
        if (bufno*buffersize<l)
            l = bufno*buffersize;
    }
    return l;
}

CSAMPLE *PlayerPortAudio::process(const CSAMPLE *, const int)
{
    return 0;
}

/* -------- ------------------------------------------------------
   Purpose: Wrapper function to call processing loop function,
            implemented as a method in a class. Used in PortAudio,
            which knows nothing about C++.
   Input:   .
   Output:  -
   -------- ------------------------------------------------------ */
int paCallback(void *, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      PaTimestamp, void *_player)
{
    Player *player = (Player *)_player;
    SAMPLE *out = (SAMPLE*)outputBuffer;
    player->prepareBuffer();
    SAMPLE *buffer = player->out_buffer_offset;

    //qDebug("chMaster: %i, chHead: %i, frames: %i, buffersize: %i",player->CH_MASTER,player->CH_HEAD,framesPerBuffer,player->BUFFERSIZE);
    
    int openChNo = max(player->CH_HEAD,player->CH_MASTER);

    for (int i=0; i<(long)framesPerBuffer; i++)
    {
        for (int j=0; j<=openChNo; j+=2)
        {
            if (j+1==player->CH_MASTER)
            {
                *out++=buffer[(i*4)  ];
                *out++=buffer[(i*4)+1];
            }
            else if (j+1==player->CH_HEAD)
            {
                *out++=buffer[(i*4)+2];
                *out++=buffer[(i*4)+3];
            }
            else
            {
                *out++=0;
                *out++=0;
            }
        }    
    }

    //if (player->CH_HEAD>0)
    //    player->buffersync.lock();

    return 0;
}

int paCallbackSlave(void *, void *outputBuffer,
                    unsigned long framesPerBuffer,
                    PaTimestamp, void *_player)
{
    PlayerPortAudio *player = (PlayerPortAudio *)_player;
    SAMPLE *out = (SAMPLE*)outputBuffer;
    SAMPLE *buffer = player->out_buffer + (player->BUFFERSIZE*2*bufferIdxSlave);
    for (int i=0; i<(long)framesPerBuffer; i++)
    {
        *out++=buffer[(i*4)+2];
        *out++=buffer[(i*4)+3];
    }

    if (bufferIdxSlave>20)
        bufferIdxSlave = 0;
    else
        bufferIdxSlave++;

    //if (player->buffersync.locked())
    //    player->buffersync.unlock();
    
    return 0;
}

