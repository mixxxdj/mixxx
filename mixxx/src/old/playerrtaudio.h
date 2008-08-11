/***************************************************************************
                          playerrtaudio.h  -  description
                             -------------------
    begin                : Thu May 20 2004
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

#ifndef PLAYERRTAUDIO_H
#define PLAYERRTAUDIO_H

#include "player.h"
#include <RtAudio.h>

/**
  *@author Tue and Ken Haste Andersen
  */


class PlayerRtAudio : public Player  {
public:
    PlayerRtAudio(ConfigObject<ConfigValue> *config);
    ~PlayerRtAudio();
    bool initialize();
    bool open();
    void close();
    void setDefaults();
    QStringList getInterfaces();
    QStringList getSampleRates();
    static QString getSoundApi();
    QString getSoundApiName() { return getSoundApi(); };
    /** Satisfy virtual declaration in EngineObject */
    void process(const CSAMPLE *, const CSAMPLE *, const int) {};
    /** Process samples. Called from RtAudio callback */
    int callbackProcess(int iBufferSize, float *out);

protected:
    /** Pointer to RtAudio object */
    RtAudio *m_pRtAudio;
    /** Get id of device with a given name. Returns -1 if device is not found */
    int getDeviceID(QString name);
    /** Get channel number of device with a given name. Returns -1 if device is no found */
    int getChannelNo(QString name);
    /** Id of currently open device. -1 if no device is open */
    int m_devId;
    /** Channels used for each output from Mixxx. Set to -1 when not in use */
    int m_iMasterLeftCh, m_iMasterRigthCh, m_iHeadLeftCh, m_iHeadRightCh;
    /** True if RtAudio was sucessfully initialized */
    bool m_bInit;
    /** Number of buffers */
    int m_iNumberOfBuffers;
};

int rtCallback(char *outputBuffer, int framesPerBuffer, void *_player);

#endif
