/***************************************************************************
                          playerjack.h  -  description
                             -------------------
    begin                : Sat Nov 15 2003
    copyright            : (C) 2003 by Tue Haste Andersen
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

#ifndef PLAYERJACK_H
#define PLAYERJACK_H

#include "player.h"
#include <jack/jack.h>

/**
  *@author Tue and Ken Haste Andersen
  */

class PlayerJack : public Player  {
public:
    PlayerJack(ConfigObject<ConfigValue> *config, ControlObject *pControl);
    ~PlayerJack();
    bool open();
    void close();
    void setDefaults();
    QStringList getInterfaces();
    QStringList getSampleRates();
    /** Satisfy virtual declaration in EngineObject */
    CSAMPLE *process(const CSAMPLE *, const int) { return 0; };
    /** Process samples. Called from jack callback */
    int callbackProcess(int iBufferSize);
    /** Used to set sample rate from Jack callback */
    void callbackSetSrate(int iSrate);
    /** Used to set current buffer size */
    void callbackSetBufferSize(int iBufferSize);
    /** Used to reinitialize when shut down by Jack server */
    void callbackShutdown();


protected:
    /** Get default device name */
    QString getDefaultDevice();
    /** Get id of device with name name */
    int getDeviceID(QString name);
    /** Open device */
    bool open(QString nameMaster, QString nameHead, int srate, int bits, int bufferSizeMaster, int bufferSizeHead, int _chMaster, int _chHead);
    int minLatency(int SRATE);

    /** Pointer to client info */
    jack_client_t *client;
    /** Null terminated array of port names */
    const char **ports;
    /** Playback ports */
    jack_port_t *output_master_left, *output_master_right, *output_head_left, *output_head_right;
    /** True if devices are open */
    bool m_bOpen;
    /** Current buffer size in use */
    int m_iBufferSize;

};

// Jack callbacks:
void jackError(const char *desc);
int jackProcess(jack_nframes_t nframes, void *arg);
int jackSrate(jack_nframes_t nframes, void *arg);
void jackShutdown(void *arg);
void jackBufferSize(jack_nframes_t nframes, void *arg);

#endif
