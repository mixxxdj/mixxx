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
#include "defs_recording.h"
#include "controllogpotmeter.h"
#include "configobject.h"
#include "controlobjectthread.h"
#include "controlobject.h"
#include "dlgprefrecord.h"

/***************************************************************************
*									   *
* Notice To Future Developpers:					   *
* 	There is code here to write the file in a seperate thread	   *
* 	however it is unstable and has been abondoned.  Its only use	   *
* 	was to support low priority recording, however I don't think its   *
* 	worth the trouble.						   *
* 									   *
***************************************************************************/

EngineRecord::EngineRecord(ConfigObject<ConfigValue> * _config)
{
    config = _config;
    recReadyCO = new ControlObject(ConfigKey("[Master]", "Record"));
    recReady = new ControlObjectThread(recReadyCO);
    fOut = new WriteAudioFile(_config);
}

EngineRecord::~EngineRecord()
{
    delete fOut;
    delete recReady;
    delete recReadyCO;
}

void EngineRecord::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
{
    CSAMPLE *Out = (CSAMPLE*) pOut;

    //We don't need to do silence detection or anything fancy
    //if recording isn't ON or READY
    if (recReady->get() != RECORD_READY && recReady->get() != RECORD_ON)
        return;

    for (int i=0; i < iBufferSize; i+=2)
    {
        //if(pIn != pOut)
        //    Out[i] = pIn[i];

        if(recReady->get() == RECORD_READY && pIn[i] > THRESHOLD_REC)
        {
            //If we are waiting for a track to start before recording
            //and the audio is high enough (a track is playing)
            //then we can set the record flag to TRUE
            qDebug("Setting Record flag to: ON");
            recReady->slotSet(RECORD_ON);
            fOut->open(); //FIXME: This is not a good spot for this. - Albert
        }
    }

    //Write record buffer to file
    if (recReady->get() == RECORD_ON)
        fOut->write(pIn, iBufferSize);
}


