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
#include <qapplication.h>

class ControlEngineQueue;

/**
  *@author Tue and Ken Haste Andersen
  */

class PlayerPortAudio : public Player  {
public: 
    PlayerPortAudio(ConfigObject<ConfigValue> *config, ControlEngineQueue *queue, QApplication *app);
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
    /** Open a device stream */
    bool open(int id, PortAudioStream **stream, int srate, PaSampleFormat format, int chNo, int bufferSize, int bufferNo, PortAudioCallback *callback);
    /** Get default device name */
    QString getDefaultDevice();
    /** Get id of device with name name */
    int getDeviceID(QString name);
    /** Open device */
    bool open(QString nameMaster, QString nameHead, int srate, int bits, int bufferSizeMaster, int bufferSizeHead, int _chMaster, int _chHead);
    /** PortAudio streams */
    PortAudioStream *streamMaster, *streamHead;
    /** true if streamHead is active */
    bool headActive;
    /** True if master device has been successfully opened */
    bool opendev;
};


int paCallback(void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      PaTimestamp outTime, void *_player);
int paCallbackSlave(void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      PaTimestamp outTime, void *_player);
#endif
