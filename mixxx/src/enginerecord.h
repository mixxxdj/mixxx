/***************************************************************************
                          enginerecord.h  -  description
                             -------------------
    copyright            : (C) 2007 by John Sully
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

#ifndef ENGINERECORD_H
#define ENGINERECORD_H

#include "engineobject.h"
#include "qthread.h"
#include "configobject.h"
#include "writeaudiofile.h"

#define DEFAULT_BUFSIZE 512

class ControlLogpotmeter;
class ConfigKey;

class EngineRecord : public EngineObject, public QThread {
public:
    EngineRecord(ConfigObject<ConfigValue> *_config);
    ~EngineRecord();
    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
    void run();
private:
    void resizeBuf(int buf, int size);
    CSAMPLE *buffer1, *buffer2;
    int bufSize1, bufSize2;
    int validBuf1, validBuf2;
    bool curBuf1;
    QWaitCondition waitCond1, waitCond2;
    QMutex mutex1, mutex2;
    WriteAudioFile *fOut;
    ConfigObject<ConfigValue> *config;
};

#endif
