/***************************************************************************
                          engineshoutcast.h  -  description
                             -------------------
    copyright            : (C) 2007 by John Sully
                           (C) 2007 by Albert Santoni
                           (C) 2007 by Wesley Stessens
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENGINESHOUTCAST_H
#define ENGINESHOUTCAST_H

#include "engineobject.h"
#include "configobject.h"
#include "controlobject.h"
#include <shout/shout.h>

#include <QObject>

#define DEFAULT_BUFSIZE 512
#define THRESHOLD_REC 2. //high enough that its not triggered by white noise

//class ControlLogpotmeter;
//class ConfigKey;
class EncoderVorbis;

/*typedef struct
{
    CSAMPLE *data;
    int size ;
    int valid;  //how much of the buffer is valid
} shoutBuffer;*/

class EngineShoutcast : public EngineObject {
    Q_OBJECT
public:
    EngineShoutcast(ConfigObject<ConfigValue> *_config);
    ~EngineShoutcast();
    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
//    void run();
private slots:
    void writePage(unsigned char *header, unsigned char *body,
                   int headerLen, int bodyLen);
private:
    void resizeBuf(int buf, int size);
//    shoutBuffer *fill, *write;   //fill buffer is written to by mixxx, write is written to file
    bool curBuf1;
    QWaitCondition waitCondFill;
    QMutex mutexFill;
    //WriteAudioFile *fOut;
    shout_t *m_pShout;
    long m_iShoutStatus;
    ConfigObject<ConfigValue> *config;
    ControlObject* recReady;
    EncoderVorbis *encoder;
};

#endif
