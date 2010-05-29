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
#include "encodervorbis.h"
#include "encodermp3.h"

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
	encoder = NULL;
    //fOut = new WriteAudioFile(config);
	//fOut->open(); 

	m_Encoding = config->getValueString(ConfigKey(RECORDING_PREF_KEY,"Encoding")).toLatin1();
	m_OGGquality = config->getValueString(ConfigKey(RECORDING_PREF_KEY,"OGG_Quality")).toLatin1();
	m_MP3quality = config->getValueString(ConfigKey(RECORDING_PREF_KEY,"MP3_Quality")).toLatin1();
	
	if(m_Encoding == ENCODING_MP3){
		encoder = new EncoderMp3(config, this);
		encoder->initEncoder(m_MP3quality.toInt());
	}
	if(m_Encoding == ENCODING_OGG){
		encoder = new EncoderVorbis(config, this);
		encoder->initEncoder(m_OGGquality.toInt());	
	
	}
	if(m_Encoding == ENCODING_WAVE || m_Encoding == ENCODING_AIFF){
		encoder = new WriteAudioFile(config, this);
	}
	if(encoder) encoder->open();
}

	

EngineRecord::~EngineRecord()
{
    delete fOut;
    //delete recReady;
    //delete recReadyCO;
	if(encoder) 
		delete encoder;
	
}

void EngineRecord::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
{
    //CSAMPLE *Out = (CSAMPLE*) pOut;

 /*
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
            //fOut->open(); //FIXME: This is not a good spot for this. - Albert
        }
    }
	*/

    //Write record buffer to file
    if (m_Encoding != ENCODING_OGG && m_Encoding != ENCODING_MP3)
        fOut->write(pIn, iBufferSize);
	else{
		encoder->encodeBuffer(pIn, iBufferSize); //encodeBuffer calls writePage
		//encoder->write();
	}
}
void EngineRecord::writePage(unsigned char *header, unsigned char *body,
                                int headerLen, int bodyLen)
{
        // Send header if there is one
        if ( headerLen > 0 ) {
            //Write header
			//fOut->write(header, headerLen);
        }
		//write body
		//fOut->write(body, bodyLen);        
}

