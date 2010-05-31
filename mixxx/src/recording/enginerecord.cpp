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
#ifdef __SHOUTCAST__
	#include "encodervorbis.h"
	#include "encodermp3.h"
#endif

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
	//returns a number from 1 .. 10
	m_OGGquality = config->getValueString(ConfigKey(RECORDING_PREF_KEY,"OGG_Quality")).toLatin1();
	m_MP3quality = config->getValueString(ConfigKey(RECORDING_PREF_KEY,"MP3_Quality")).toLatin1();
	
	if(m_Encoding == ENCODING_MP3){
	#ifdef __SHOUTCAST__
		encoder = new EncoderMp3(config, this);
		encoder->initEncoder(convertToBitrate(m_MP3quality.toInt()));
	#else
		qDebug() << "MP3 recording requires Mixxx to build with shoutcast support";
	#endif
		
	}
	
	if(m_Encoding == ENCODING_OGG){
	#ifdef __SHOUTCAST__
		encoder = new EncoderVorbis(config, this);
		encoder->initEncoder(convertToBitrate(m_OGGquality.toInt()));
	#else
		qDebug() << "OGG recording requires Mixxx to build with shoutcast support";
	#endif
	
	}
	if(m_Encoding == ENCODING_WAVE || m_Encoding == ENCODING_AIFF){
		encoder = new WriteAudioFile(config, this);
	}
	if(encoder) encoder->openFile();
}

	

EngineRecord::~EngineRecord()
{
	
	if(encoder) {
		encoder->flush();
		encoder->writeFile();	//write to file	
		delete encoder;
	}
	
}
int EngineRecord::convertToBitrate(int quality){
	switch(quality)
        {
            case 1: return 16;
            case 2: return 24;
            case 3: return 32;
            case 4: return 64;
            case 5: return 128;
            case 6: return 160;
            case 7: return 192;
            case 8: return 224;
            case 9: return 256;
            case 10: return 320;
			default: return 128;
        }
}
void EngineRecord::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
{
  
    //Write record buffer to file
	encoder->encodeBuffer(pIn, iBufferSize); 
	encoder->writeFile(); //write to file
	
}
void EngineRecord::writePage(unsigned char *header, unsigned char *body,
                                int headerLen, int bodyLen)
{
     	//EngineRecord is not responsible for streaming to shoutcast
		// --> Empty method
}

