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


PlayerPortAudio::PlayerPortAudio(ConfigObject<ConfigValue> *config) : Player(config)
{
    opendev = false;
    
    PaError err = Pa_Initialize();
    if (err!=paNoError)
        qFatal("PortAudio: Initialization error");
    
    const PaDeviceInfo *devInfo;

    //
    // Fill out devices list with info about available devices if list is empty
    //
    if (devices.isEmpty())
    {
        int no = Pa_CountDevices();
        for (int i=0; i<no; i++)
        {
            devInfo = Pa_GetDeviceInfo(i);

            // Add the device if it is an output device:
            if (devInfo!=0 && devInfo->maxOutputChannels > 0)
            {
                //qWarning("PortAudio: Name: %s, ID: %i, MaxOutput %i",devInfo->name,i,devInfo->maxOutputChannels);

                // Add new PlayerInfo object to devices list
                Player::Info *p = new Player::Info;
                devices.append(p);

                // ID & Name
                p->name = QString(devInfo->name);
                p->id = i;

                // Sample rates and latency
                if (devInfo->numSampleRates != -1)
                {
                    for (int j=0; j<devInfo->numSampleRates; j++)
                    {
                        p->sampleRates.append((int)devInfo->sampleRates[j]);

                        // Get minimum latency for sample rate
                        p->latency.append(minLatency((int)devInfo->sampleRates[j]));
                        //qDebug("SRATE: %i, Latency: %i",(int)devInfo->sampleRates[j],minLatency((int)devInfo->sampleRates[j]));
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
}

PlayerPortAudio::~PlayerPortAudio()
{
    Pa_Terminate();
}

bool PlayerPortAudio::open(QString nameMaster, QString nameHead, int srate, int bits, int bufferSizeMaster, int bufferSizeHead, int _chMaster, int _chHead)
{
    chMaster = _chMaster;
    chHead   = _chHead;
    
    // Adjust bufferSize and number of buffers
    int bufferNo     = 2;
    MasterBufferSize = bufferSizeMaster/bufferNo;
    
    // Extract bit information
    PaSampleFormat format = 16;
    switch (bits)
    {
        case 0:  format = paInt16; break;
        case 8:  format = paInt8; break;
        case 16: format = paInt16; break;
        case 24: format = paInt24; break;
        case 32: format = paInt32; break;
        default: qWarning("PortAudio: Sample format not supported (%i bits)", bits); return false;
    }

    // Determine if a separate device for headphone channel is requested
    if (nameHead.length()>0 && nameHead != "None" && nameHead != nameMaster)
        headActive = true;
    else
    {
        headActive = false;
        if (chMaster == chHead)
            return false;
    }
    
    // Get id's of devices
    int masterID = getDeviceID(nameMaster);
    if (masterID == -1) return false;
    int headID;
    if (headActive)
    {
        headID = getDeviceID(nameHead);
        if (headID==-1) return false;
    }

    // Number of channels to open and buffersize
    bufferIdx = 0;
    int chNoMaster, chNoHead;
    if (headActive)
    {
        chNoMaster = chMaster+1;
        chNoHead   = chHead+1;
        bufferIdxSlave = 2;
        HeadPerMasterBuffer = (int)floor((float)(bufferSizeHead/2.)/(float)MasterBufferSize);
    }
    else
    {
        chNoMaster = max(_chMaster,_chHead)+1;
        HeadPerMasterBuffer = 1;
    }
    
    // Open master device stream
    if (!open(masterID,&streamMaster,srate,format,chNoMaster,MasterBufferSize/chNoMaster,bufferNo,paCallback))
        return false;

    // Open head device stream
    if (headActive)
        if (!open(headID,&streamHead,srate,format,chNoHead,HeadPerMasterBuffer*MasterBufferSize/chNoHead,bufferNo,paCallbackSlave))
        {
            // Head device was unsuccessful, so close master properly
            opendev = true;
            headActive = false;
            close();

            return false;
        }

    opendev = true;
    
    return true;
}

bool PlayerPortAudio::open(int id, PortAudioStream **stream, int srate, PaSampleFormat format, int chNo, int bufferSize, int bufferNo, PortAudioCallback *callback)
{
    // Try to open device 5 times before giving up!
    qDebug("Device id: %i",id);
    PaError err = 0;
    {for (int i=0; i<5; i++)
    {
      err = Pa_OpenStream(stream,
                        paNoDevice,         // no input device
                        0,                  // no input
                        format,
                        NULL,
                        id,                 // output device
                        chNo,
                        format,
                        NULL,
                        (double)srate,
                        bufferSize,         // frames per buffer per channel
                        bufferNo,           // number of buffers, if zero then use default minimum
                        paClipOff,          // we won't output out of range samples so don't bother clipping them
                        callback,
                        this);

        if (err == paNoError)
            break;
    }}
    if( err != paNoError )
    {
        qWarning("PortAudio: Open stream error: %s", Pa_GetErrorText(err));
        err = Pa_GetHostError();
        qWarning("PortAudio: Open stream error: %s", Pa_GetErrorText(err));
        return false;
    }

    // Update SRATE and BASERATE in EngineObject
    setPlaySrate(srate);

    return true;
}


    

void PlayerPortAudio::close()
{
    // Close stream
    if (opendev)
    {
        PaError err = Pa_CloseStream(streamMaster);
        if( err != paNoError )
            qWarning("PortAudio: Close master stream error: %s", Pa_GetErrorText(err));
        if (headActive)
        {
            err = Pa_CloseStream(streamHead);
            if( err != paNoError )
                qWarning("PortAudio: Close headphone stream error: %s", Pa_GetErrorText(err));
        }
        opendev = false;
    }
}

void PlayerPortAudio::start()
{
    Player::start();

    if (opendev)
    {
        PaError err = Pa_StartStream(streamMaster);
        if (err != paNoError)
            qWarning("PortAudio: Start master stream error: %s", Pa_GetErrorText(err));
        if (headActive)
        {
            err = Pa_StartStream(streamHead);
            if (err != paNoError)
                qWarning("PortAudio: Start headphone stream error: %s", Pa_GetErrorText(err));
        }
    }
}

void PlayerPortAudio::wait()
{
}

void PlayerPortAudio::stop()
{
    PaError err = Pa_StopStream(streamMaster);
    if( err != paNoError )
        qFatal("PortAudio: Stop master stream error: %s,", Pa_GetErrorText(err));
    if (headActive)
    {
        err = Pa_StopStream(streamHead);
        if (err != paNoError)
            qWarning("PortAudio: Stop headphone stream error: %s", Pa_GetErrorText(err));
    }
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

QString PlayerPortAudio::getDefaultDevice()
{
    // Return default device name
    PaDeviceID id = Pa_GetDefaultOutputDeviceID();
    const PaDeviceInfo *devInfo = Pa_GetDeviceInfo(id);
    return QString(devInfo->name);
}

int PlayerPortAudio::getDeviceID(QString name)
{
    for (int i=0; i<(int)devices.count(); i++)
        if (name == devices.at(i)->name)
            return devices.at(i)->id;
    return -1;
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
    // player->BUFFERSIZE = framesPerBuffer*no_of_channels

    Player *player = (Player *)_player;
    SAMPLE *out = (SAMPLE*)outputBuffer;
    player->prepareBuffer();
    SAMPLE *buffer = player->out_buffer_offset;

//    qDebug("Master %i, MasterBufferSize %i",framesPerBuffer,player->MasterBufferSize);

    int openChNo = max(player->chHead,player->chMaster);

    for (int i=0; i<(long)framesPerBuffer; i++)
    {
        for (int j=0; j<=openChNo; j+=2)
        {
            if (j+1==player->chMaster)
            {
                *out++=buffer[(i*4)  ];
                *out++=buffer[(i*4)+1];
            }
            else if (j+1==player->chHead)
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

    return 0;
}

int paCallbackSlave(void *, void *outputBuffer,
                    unsigned long framesPerBuffer,
                    PaTimestamp, void *_player)
{
    PlayerPortAudio *player = (PlayerPortAudio *)_player;
    SAMPLE *out = (SAMPLE*)outputBuffer;
    SAMPLE *buffer = player->out_buffer + (2*player->MasterBufferSize*player->HeadPerMasterBuffer*bufferIdxSlave);
    for (int i=0; i<(long)framesPerBuffer; i++)
    {
        *out++=buffer[(i*4)+2];
        *out++=buffer[(i*4)+3];
    }

    if (bufferIdxSlave>3)
        bufferIdxSlave = 0;
    else
        bufferIdxSlave++;

    return 0;
}

