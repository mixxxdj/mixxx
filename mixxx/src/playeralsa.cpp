/***************************************************************************
                          playeralsa.cpp  -  description
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
#include "playeralsa.h"

PlayerALSA::PlayerALSA(int size, vector<EngineObject *> *engines) : Player(size, engines)
{
    // Open device 0 on card 0
    int err = snd_pcm_open(&handle, 0, 0, SND_PCM_OPEN_PLAYBACK);
    if (err != 0) {
      qFatal("Error opening device (%i): %s",err,snd_strerror(err));
      std::exit(-1);
    }

    // ALSA channel parameters
    params = new snd_pcm_channel_params_t;
    params->mode=SND_PCM_MODE_BLOCK;
    params->start_mode=SND_PCM_START_FULL;
    params->stop_mode=SND_PCM_STOP_ROLLOVER; // This makes the audio keep playing even
                                             // if underruns occurs
    params->buf.block.frag_size = size*SAMPLE_SIZE; // Buffer size in bytes
    params->buf.block.frags_max=3;
    params->buf.block.frags_min=1;
    params->format.interleave=1;
    params->format.format=SND_PCM_SFMT_S16_LE;
    params->format.rate=SRATE;
    params->channel=SND_PCM_CHANNEL_PLAYBACK;
    params->format.voices=2;

    err = snd_pcm_channel_params(handle,params);
    if (err != 0) {
      // try to close what was opened!
      snd_pcm_close(handle);
      qFatal("Error setting parameters for device %i", err);
      std::exit(-1);
    }
    setup = new snd_pcm_channel_setup_t;
    setup->channel=SND_PCM_CHANNEL_PLAYBACK;
    err = snd_pcm_channel_setup(handle, setup);
    if (err>0) {
      qFatal("Error setting up channel (%i)", err);
      std::exit(-1);
    }

    err = snd_pcm_playback_flush(handle);
    if (err>0) {
      qFatal("Error flushing playback buffer %i):%s", err, snd_strerror(err));
      std::exit(-1);
    }

    // The buffer size has possible been changed by the driver. The size returned
    // by the driver is given in number of bytes, and BUFFER_SIZE indicates the
    // same size in samples
    if ((buffer_size = setup->buf.block.frag_size/SAMPLE_SIZE) == 0) {
      qFatal("Driver returned zero buffer size.");
      std::exit(-1);
    }

    qDebug("Using ALSA. Buffer size : %i samples.", buffer_size/2);
	allocate();

	// Allocate semaphore to stop playback
	requestStop = new QSemaphore(1);
}

PlayerALSA::~PlayerALSA()
{
	qDebug("dealloc buffer");
	if (running())
	{
		qDebug("Stopping buffer");
		stop();
	}

	// Close audio device
	snd_pcm_close(handle);

	// Deallocate objects
	delete setup;
	delete params;
}

void PlayerALSA::start(EngineObject *_reader)
{
	Player::start(_reader);

	// Prepare for playback
	int err = snd_pcm_channel_prepare(handle, params->channel);
	if (err>0)
	{
		qFatal("Error preparing channel (%i)%s",err , snd_strerror(err));
     	std::exit(-1);
	}

	// Start thread
	QThread::start();
}

void PlayerALSA::stop()
{
	qDebug("Request stop");
	requestStop->operator++(1);
	qDebug("Waiting for thread to stop: %i",requestStop->total());
	wait();
	requestStop->operator--(1);

	qDebug("drain audio");

	// Stop audio
	snd_pcm_playback_drain(handle);

	// Terminate synth thread
	//pthread_cancel(p_thread);

	// Wait for synth thread to terminate
	//void *thread_state;
	//pthread_join(p_thread, &thread_state);
}

void PlayerALSA::wait()
{
	QThread::wait();
}

void PlayerALSA::run()
{
	qDebug("PlayerALSA: Beginning of thread.");
	rt_priority();

	// Loop the synthesis, and pass the buffers to ALSA
	int res = 0;
	int BUFFER_SIZE_BYTES = buffer_size*SAMPLE_SIZE;
	//std::cout << "Starting playback thread\n" << flush;
	while ((res == 0) && (requestStop->available()))
	{
		res = prepareBuffer();
		if ((res == 0) && (snd_pcm_write(handle,out_buffer,BUFFER_SIZE_BYTES)
						   != BUFFER_SIZE_BYTES))
			qFatal("Error writing samples to hardware %i",res);
	}
	qDebug("Leaving thread");
}

void PlayerALSA::rt_priority()
{
	// Try to set realtime priority on the current executing thread
	struct sched_param schp;
	memset(&schp, 0, sizeof(schp));
	schp.sched_priority = 50; //sched_get_priority_max(SCHED_FIFO);
	if (sched_setscheduler(0, SCHED_FIFO, &schp) != 0)
		qWarning("Not possible to give audio I/O thread realtime prioriy.");
}


