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
#include <vector>
#include <qvaluelist.h>
#include <qptrlist.h>
#include <qstring.h>

class Player : public EngineObject {
public:
    Player(int, std::vector<EngineObject *> *, QString device);
    ~Player();      // Deallocate
    bool reopen(QString name, int srate, int bits, int bufferSize, int chMaster, int chHead);
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

    static SAMPLE *out_buffer, *out_buffer_offset;
    int prepareBuffer(); // Calculates one buffer of sound
    
    /** MasterBufferSize is set when the master device is opened. If a head device is later opened
      * HeadPerMasterBuffer is calculated, which equals HeadBufferSize/MasterBufferSize. When the
      * master buffer is synthesized hereafter, a buffer three times MasterBufferSize*HeadPerMasterBuffer
      * is used. The buffer is read by the head device using its own block size */
    static int MasterBufferSize;
    static int HeadPerMasterBuffer;

protected:
    virtual bool open(QString name, int srate, int bits, int bufferSize, int chMaster, int chHead) = 0;
    virtual void close() = 0;
    void allocate();
    void deallocate();
    
    std::vector<EngineObject *> *engines;
    EngineObject* reader;
    QPtrList<Info>  devices;


    /** Indicates where in the out_buffer the current synthesized frame is placed. */
    int bufferIdx;
};

#endif




