/***************************************************************************
                          player.h  -  description
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

#ifndef PLAYER_H
#define PLAYER_H

#include <qobject.h>
#include "defs.h"
#include <stdlib.h>
#include <iostream>
#include "engineobject.h"
#include "controlobject.h"
#include "configobject.h"
#include <vector>
#include <qvaluelist.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qapplication.h>

class ControlEngineQueue;

class Player : public EngineObject
{
public:
    Player(ConfigObject<ConfigValue> *_config, ControlEngineQueue *queue, QApplication *_app);
    virtual ~Player();      // Deallocate
    void notify(double) {};
    bool open(bool useDefault);
    virtual void close() = 0;
    virtual void start() {}; // Start audio stream
    virtual void stop() = 0;           // Stops audio stream
    virtual void wait() = 0;           // Wait for audio stream to finish
    virtual int minLatency(int SRATE) = 0; // Given a sample rate, return the minimum latency for that card
    void setReader(EngineObject *);
    typedef struct
    {
        int             id;
        QString         name;
        QValueList<int> sampleRates;
        QValueList<int> bits;
        QValueList<int> latency;
        int             noChannels;
    } Info;
    
    QPtrList<Info> *getInfo();

    static SAMPLE       *out_buffer, *out_buffer_offset;
    int prepareBuffer(); // Calculates one buffer of sound
    /** MasterBufferSize is set when the master device is opened. If a head device is later opened
      * HeadPerMasterBuffer is calculated, which equals HeadBufferSize/MasterBufferSize. When the
      * master buffer is synthesized hereafter, a buffer three times MasterBufferSize*HeadPerMasterBuffer
      * is used. The buffer is read by the head device using its own block size */
    static int          MasterBufferSize;
    static int          HeadPerMasterBuffer;
    /** Varaiables containing data already in config database, like the buffer size varaiables.
        Cached here for efficiency reasons */
    int                 chMaster, chHead;
    
protected:
    void allocate();
    void deallocate();
    virtual QString getDefaultDevice() = 0;
    virtual bool open(QString nameMaster, QString nameHead, int srate, int bits, int bufferSizeMaster, int bufferSizeHead, int _chMaster, int _chHead) = 0;

    /** Configuration data */
    ConfigObject<ConfigValue> *config;
    EngineObject        *reader;
    QPtrList<Info>      devices;

    /** Indicates where in the out_buffer the current synthesized frame is placed. */
    int bufferIdx;

    /** Pointer to ControlEngineQueue taking care of syncing control parameters
        from GUI (main) thread to player thread */
    ControlEngineQueue *queue;
    /** Pointer to qapp */
    QApplication *app;
};

#endif




