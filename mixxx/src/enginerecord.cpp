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
    curBuf1 = true;
    config = _config;
    fOut = new WriteAudioFile(_config);
    
    //Allocate Buffers
    fill = new Buffer;
    fill->size = DEFAULT_BUFSIZE;
    fill->data = new CSAMPLE[fill->size];
    fill->valid = 0;

    write = new Buffer;
    write->size = DEFAULT_BUFSIZE;
    write->data = new CSAMPLE[fill->size];
    write->valid = 0;
    //QThread::start();
}

EngineRecord::~EngineRecord()
{
    delete fill->data;
    delete fill;
    delete write->data;
    delete write;
    QThread::terminate();
    qDebug("rec thread terminated");
}

void EngineRecord::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize)
{
    CSAMPLE *Out = (CSAMPLE*) pOut;
    
    mutexFill.lock();
    if(fill->size < iBufferSize)
    {
	delete fill->data;
	fill->size = iBufferSize;
	fill->data = new CSAMPLE[ fill->size ];
    }
    for(int i=0; i<iBufferSize; i++)
    {
	if(pIn != pOut)
	    Out[i] = pIn[i];
	fill->data[i] = pIn[i];
    }
    fill->valid = iBufferSize;
    mutexFill.unlock();
    waitCondFill.wakeAll();
}


void EngineRecord::run()
{
    //Method will record the buffer to file
    //fOut->write(buffer1, validBuf1);
    Buffer *temp;
    while(1)
    {
	mutexFill.lock();
	waitCondFill.wait(&mutexFill);
	
	//swap buffers
	temp = fill;
	fill = write;
	write = temp;

	mutexFill.unlock();
	
	//write record buffer to file
	fOut->write(write->data, write->valid);
	write->valid = 0;
    }
}

