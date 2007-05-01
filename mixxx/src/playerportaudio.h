/***************************************************************************
                          playerportaudiov19.h  -  description
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

#include <qsemaphore.h>
#include <qthread.h>
#include "player.h"
#include "portaudio.h"

/**
  *@author Tue and Ken Haste Andersen
  */

/** Maximum frame size used with PortAudio. Used to determine no of buffers
  * when setting latency */
const int kiMaxFrameSize = 1024;

class PlayerPortAudio; //Forward declaration

/** A struct to some stuff we need to pass along to the callback through PortAudio **/
struct PAPlayerCallbackStuff
{
	PlayerPortAudio* player;
	int devIndex;
};

class PlayerPortAudio : public Player  {
public:
    PlayerPortAudio(ConfigObject<ConfigValue> *config, QString api_name);
    ~PlayerPortAudio();
    bool initialize();
    bool open();
    void close();
    void setDefaults();
    QStringList getInterfaces();
    QStringList getSampleRates();
    static QStringList getSoundApiList();
    QString getSoundApiName() { return getSoundApiList().front(); };
    void calculateNumActiveDevices();
    /** Satisfy virtual declaration in EngineObject */
    void process(const CSAMPLE *, const CSAMPLE *, const int) {};
    /** Process samples. Called from PortAudio callback */
    int callbackProcess(int iBufferSize, float *out, int devIndex);

    static bool m_painited;
protected:
    /** Get id of device with a given name. Returns -1 if device is not found */
    PaDeviceIndex getDeviceID(QString name);
    /** Get channel number of device with a given name. Returns -1 if device is no found */
    int getChannelNo(QString name);

    /** PortAudio stream */
    PaStream *m_pStream[MAX_AUDIODEVICES];
    /** Id of currently open device. -1 if no device is open */
    PaDeviceIndex m_devId[MAX_AUDIODEVICES];
    /** Channels used for each output from Mixxx. Set to -1 when not in use */
    int m_iMasterLeftCh, m_iMasterRigthCh, m_iHeadLeftCh, m_iHeadRightCh;
    /** True if PortAudio was sucessfully initialized */
    bool m_bInit;
    /** A struct to hold some information/pointers we need to pass to our callback function */
    PAPlayerCallbackStuff callbackStuff[MAX_AUDIODEVICES];
    /** Number of buffers */
    int m_iNumberOfBuffers;
    /** Number of active/open soundcards **/
    int m_iNumActiveDevices;
    /** Name of the current audio API inside PortAudio **/
    QString m_HostAPI;
    /** Mutex so that two threads don't try to prepare a new buffer full of samples from Mixxx at the same time */
    QMutex lockSamples; 
    /** Wait condition that forces multiple PortAudio callbacks in separate threads to play nicely */
    QWaitCondition waitForNextOutput;
};

int paV19Callback(const void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo* timeInfo,
                      PaStreamCallbackFlags statusFlags,
                      void *_player);
#endif
