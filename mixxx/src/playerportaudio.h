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

#include "player.h"
#include "portaudio.h"

/**
  *@author Tue and Ken Haste Andersen
  */

/** Maximum frame size used with PortAudio. Used to determine no of buffers
  * when setting latency */
const int kiMaxFrameSize = 1024;

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
    /** Satisfy virtual declaration in EngineObject */
    void process(const CSAMPLE *, const CSAMPLE *, const int) {};
    /** Process samples. Called from PortAudio callback */
    int callbackProcess(int iBufferSize, float *out);

protected:
    /** Get id of device with a given name. Returns -1 if device is not found */
    PaDeviceIndex getDeviceID(QString name);
    /** Get channel number of device with a given name. Returns -1 if device is no found */
    int getChannelNo(QString name);

    /** PortAudio stream */
    PaStream *m_pStream;
    /** Id of currently open device. -1 if no device is open */
    PaDeviceIndex m_devId;
    /** Channels used for each output from Mixxx. Set to -1 when not in use */
    int m_iMasterLeftCh, m_iMasterRigthCh, m_iHeadLeftCh, m_iHeadRightCh;
    /** True if PortAudio was sucessfully initialized */
    bool m_bInit;
    /** Number of buffers */
    int m_iNumberOfBuffers;
    /** Name of the current audio API inside PortAudio **/
    QString m_HostAPI;
    
};


int paV19Callback(const void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo* timeInfo,
                      PaStreamCallbackFlags statusFlags,
                      void *_player);
#endif
