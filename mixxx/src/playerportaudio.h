/***************************************************************************
                          playerportaudio.h  -  description
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

#ifndef PLAYERPORTAUDIO_H
#define PLAYERPORTAUDIO_H

#include "player.h"
#include <portaudio.h>

/**
  *@author Tue and Ken Haste Andersen
  */

class PlayerPortAudio : public Player  {
public: 
    PlayerPortAudio(ConfigObject<ConfigValue> *config);
    ~PlayerPortAudio();
    /** Close device */
    void close();
    /** Stop playback */
    void stop();
    /** Start playback */
    void start();
    /** Wait for playback to finish */
    void wait();
    int minLatency(int SRATE);
    CSAMPLE *process(const CSAMPLE *, const int);
protected:
    /** Get default device name */
    QString getDefaultDevice();
    /** Open device */
    bool open(QString name, int srate, int bits, int bufferSize, int chMaster, int chHead);
    /** PortAudio stream */
    PortAudioStream *stream;
};


int paCallback(void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      PaTimestamp outTime, void *_player);
int paCallbackSlave(void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      PaTimestamp outTime, void *_player);
#endif
