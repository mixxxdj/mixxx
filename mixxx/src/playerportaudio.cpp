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

PlayerPortAudio::PlayerPortAudio(int size, std::vector<EngineObject *> *engines) : Player(size, engines)
{
    PaError err;
    err = Pa_Initialize();
    if( err != paNoError ) qFatal("PortAudio initialization error");

    err = Pa_OpenStream(&stream,
                        paNoDevice,     /* default input device */
                        0,              /* no input */
                        paFloat32,      /* 32 bit floating point input */
                        NULL,
                        Pa_GetDefaultOutputDeviceID(), /* default output device */
                        2,              /* stereo output */
                        paFloat32,      /* 32 bit floating point output */
                        NULL,
                        SRATE,
                        SAMPLE_SIZE,    /* frames per buffer */
                        0,              /* number of buffers, if zero then use default minimum */
                        paClipOff,      /* we won't output out of range samples so don't bother clipping them */
                        paCallback,
                        this );
    if( err != paNoError ) qFatal("PortAudio open stream error");
    err = Pa_StartStream( stream );
    if( err != paNoError ) qFatal("PortAudio start stream error");

    BUFFER_SIZE = SAMPLE_SIZE;

    qDebug("Using PortAudio. Buffer size : %i samples.",BUFFER_SIZE/2);
	allocate();
}

PlayerPortAudio::~PlayerPortAudio()
{
	Pa_Terminate();
}

void PlayerPortAudio::start(EngineBuffer *_reader)
{
	Player::start(_reader);

	PaError err = Pa_StartStream(stream);
	if (err != paNoError) exit(-1);
}

void PlayerPortAudio::wait()
{
}

void PlayerPortAudio::stop()
{
	PaError err = Pa_StopStream( stream );
	if( err != paNoError ) exit(-1);

	err = Pa_CloseStream( stream );
	if( err != paNoError ) exit(-1);
}





/* -------- ------------------------------------------------------
   Purpose: Wrapper function to call processing loop function,
            implemented as a method in a class. Used in PortAudio,
            which knows nothing about C++.
   Input:   .
   Output:  -
   -------- ------------------------------------------------------ */
static int paCallback(void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      PaTimestamp outTime, void *_player)
{
    Player *player = (Player *)_player;
    SAMPLE *out = (SAMPLE*)outputBuffer;
    player->prepareBuffer();
    for (int i=0; i<BUFFER_SIZE; i++)
        *out++=player->out_buffer[i];
    return 0;
}
