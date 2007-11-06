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
#define THRESHOLD_REC 2. //high enough that its not triggered by white noise

class ControlLogpotmeter;
class ConfigKey;
class ControlObject;
class ControlObjectThreadMain;

typedef struct
{
    CSAMPLE *data;
    int size ;
    int valid;  //how much of the buffer is valid
} Buffer;

class EngineRecord : public EngineObject, public QThread {
public:
    EngineRecord(ConfigObject<ConfigValue> *_config);
    ~EngineRecord();
    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
    void run();
private:
    void resizeBuf(int buf, int size);
    Buffer *fill, *write;   //fill buffer is written to by mixxx, write is written to file
    bool curBuf1;
    QWaitCondition waitCondFill;
    QMutex mutexFill;
    WriteAudioFile *fOut;
    ConfigObject<ConfigValue> *config;
    ControlObjectThreadMain* recReady;
    ControlObject* recReadyCO;
};

#endif
