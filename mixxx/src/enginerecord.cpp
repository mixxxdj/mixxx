/***************************************************************************
                          enginerecord.cpp  -  class to record the mix
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

#include "enginerecord.h"
#include "controllogpotmeter.h"
#include "configobject.h"

EngineRecord::EngineRecord(ConfigObject<ConfigValue> *_config)
{
    bufSize1 = DEFAULT_BUFSIZE;
    bufSize2 = DEFAULT_BUFSIZE;
    buffer1 = new CSAMPLE[ bufSize1 ];
    buffer2 = new CSAMPLE[ bufSize2 ];
    validBuf1 = 0;
    validBuf2 = 0;
    curBuf1 = true;
    config = _config;
    fOut = new WriteAudioFile(_config);
    QThread::start();
}

EngineRecord::~EngineRecord()
{
    delete buffer1;
    delete buffer2;
    QThread::terminate();
    qDebug("rec thread terminated");
}

void EngineRecord::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize)
{
    CSAMPLE *Out = (CSAMPLE*) pOut;
    if(curBuf1)
    {
	mutex1.lock();
	if(bufSize1 < iBufferSize)
	{
	    bufSize1 = iBufferSize;
	    delete buffer1;
	    buffer1 = new CSAMPLE[ bufSize1 ];
	}
	validBuf1 = iBufferSize;
	for(int i=0; i<iBufferSize; i++)
	{
	    if(pIn != pOut)
		Out[i] = pIn[i];
	    buffer1[i] = pIn[i];
	}
	mutex1.unlock();
	waitCond1.wakeAll();
    }
    else
    {
	mutex2.lock();
	if(bufSize2 < iBufferSize)
	{
	    bufSize2 = iBufferSize;
	    delete buffer2;
	    buffer2 = new CSAMPLE[bufSize2];
	}
	validBuf2 = iBufferSize;
	for(int i=0; i<iBufferSize; i++)
	{
	    if(pIn != pOut)
		Out[i] = pIn[i];
	    buffer2[i] = pIn[i];
	}
	mutex2.unlock();
	waitCond2.wakeAll();
    }
}


void EngineRecord::run()
{
    //Method will record the buffer to file
    while(1)
    {
	if(curBuf1)
	{
	    mutex1.lock();
	    waitCond1.wait(&mutex1);
	    for(int i=0; i<validBuf1; i+=2)
	    {
		//qDebug("buf1: %f, %f", buffer1[i], buffer1[i+1]);
		fOut->write(buffer1, validBuf1);
	    }
	    validBuf1 = 0;
	    curBuf1 = !curBuf1;
	    mutex1.unlock();
	}
	else
	{
	    mutex2.lock();
	    waitCond2.wait(&mutex2);
	    for(int i=0; i<validBuf2; i+=2)
	    {
		//qDebug("buf2: %f, %f", buffer2[i], buffer2[i+1]);
		fOut->write(buffer2, validBuf2);
	    }
	    validBuf2 = 0;
	    curBuf1 = !curBuf1;
	    mutex2.unlock();
	}
    }
}

