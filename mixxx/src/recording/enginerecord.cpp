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
    m_config = _config;
	m_encoder = NULL;
	m_sndfile = NULL;
}

EngineRecord::~EngineRecord()
{
	closeFile();
	
}
void EngineRecord::updateFromPreferences()
{
	m_Encoding = m_config->getValueString(ConfigKey(RECORDING_PREF_KEY,"Encoding")).toLatin1();
	//returns a number from 1 .. 10
	m_OGGquality = m_config->getValueString(ConfigKey(RECORDING_PREF_KEY,"OGG_Quality")).toLatin1();
	m_MP3quality = m_config->getValueString(ConfigKey(RECORDING_PREF_KEY,"MP3_Quality")).toLatin1();
	m_filename = m_config->getValueString(ConfigKey(RECORDING_PREF_KEY,"Path")).toAscii();


	if(m_encoder){
		delete m_encoder;	//delete m_encoder if it has been initalized (with maybe) different bitrate
		m_encoder = NULL;
	}

	if(m_Encoding == ENCODING_MP3){
	#ifdef __SHOUTCAST__
		m_encoder = new EncoderMp3(m_config, this);
		if(m_encoder->initEncoder(convertToBitrate(m_MP3quality.toInt())) < 0){
			delete m_encoder;
			m_encoder = NULL;
			qDebug() << "MP3 recording is not supported. Lame could not be initialized";
		}
	#else
		qDebug() << "MP3 recording requires Mixxx to build with shoutcast support";
	#endif
		
	}
	if(m_Encoding == ENCODING_OGG){
	#ifdef __SHOUTCAST__
		m_encoder = new EncoderVorbis(m_config, this);
		if(m_encoder->initEncoder(convertToBitrate(m_OGGquality.toInt())) < 0){
			delete m_encoder;			
			m_encoder = NULL;
			qDebug() << "OGG recording is not supported. OGG/Vorbis library could not be initialized";
			
		}
	#else
		qDebug() << "OGG recording requires Mixxx to build with shoutcast support";
	#endif
	
	}
	/* 
	 * If we use WAVE OR AIFF
	 * the encoder will be NULL at all times
	 *
	 */
	
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
	if(m_Encoding == ENCODING_WAVE || m_Encoding == ENCODING_AIFF){
		if(m_sndfile != NULL)
			sf_write_float(m_sndfile, pIn, iBufferSize);
	}
	else{
		if(!m_encoder) return;
    	//Compress audio. Encoder will call method 'write()' below to write a file stream
		m_encoder->encodeBuffer(pIn, iBufferSize); 
	}
  	
	
	
}
/** encoder will call this method to write compressed audio **/
void EngineRecord::write(unsigned char *header, unsigned char *body,
                                int headerLen, int bodyLen)
{
     	if(!fileOpen()){
			return;
		}
		//Relevant for OGG
		if(headerLen > 0){
			m_file.write((const char*) header, headerLen);
		}
		//always write body
		m_file.write((const char*) body, bodyLen);
		
}
bool EngineRecord::fileOpen(){
	// Both encoder and file must be initalized

	if(m_Encoding == ENCODING_WAVE || m_Encoding == ENCODING_AIFF){
		return (m_sndfile != NULL);
	}
	else{
		return (m_file.handle() != -1);
	}
}
//Creates a new MP3 file
bool EngineRecord::openFile(){
		
	//Unfortunately, we cannot use QFile for writing WAV and AIFF audio
	if(m_Encoding == ENCODING_WAVE || m_Encoding == ENCODING_AIFF){
		
		//set sfInfo
        m_sfInfo.samplerate = m_config->getValueString(ConfigKey("[Soundcard]","Samplerate")).toULong();
        m_sfInfo.channels = 2;

		if (m_Encoding == ENCODING_WAVE)
            m_sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
        else
            m_sfInfo.format = SF_FORMAT_AIFF | SF_FORMAT_PCM_16;
		
		//creates a new WAVE or AIFF file and write header information
		m_sndfile = sf_open(m_filename, SFM_WRITE, &m_sfInfo);
		if(m_sndfile){
			sf_command (m_sndfile, SFC_SET_NORM_FLOAT, NULL, SF_FALSE) ;
			//set meta data
            int ret;
            QByteArray baTitle = m_config->getValueString(ConfigKey(RECORDING_PREF_KEY, "Title")).toLatin1();
            ret = sf_set_string(m_sndfile, SF_STR_TITLE, baTitle.data());
            if(ret != 0)
                qDebug("libsndfile: %s", sf_error_number(ret));
            QByteArray baAuthor = m_config->getValueString(ConfigKey(RECORDING_PREF_KEY, "Author")).toLatin1();
            ret = sf_set_string(m_sndfile, SF_STR_ARTIST, baAuthor.data());
            if(ret != 0)
                qDebug("libsndfile: %s", sf_error_number(ret));
            QByteArray baComment = m_config->getValueString(ConfigKey(RECORDING_PREF_KEY, "Album")).toLatin1();
            ret = sf_set_string(m_sndfile, SF_STR_COMMENT, baComment.data()); 
            if(ret != 0)
                qDebug("libsndfile: %s", sf_error_number(ret));

		}
	}
	else
	{
		//we can use a QFile to write compressed audio
		m_file.setFileName(m_filename);
		m_file.open(QIODevice::WriteOnly | QIODevice::Text);
		  
	}
	//check if file are really open
	if(!fileOpen()){
		ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
		props->setType(DLG_WARNING);
		props->setTitle(tr("Recording"));
		props->setText(tr("<html>Could not create audio file for recording!<p><br>Maybe you do not have enough free disk space or file permissions.</html>"));
		ErrorDialogHandler::instance()->requestErrorDialog(props);
		return false;
	}
	return true;

}
void EngineRecord::closeFile(){
	if(m_Encoding == ENCODING_WAVE || m_Encoding == ENCODING_AIFF){
		if(m_sndfile != NULL){
        	sf_close(m_sndfile);
        	m_sndfile = NULL;
    	}
	}
	else{
		//close QFile and encoder, if open
		if(m_file.handle() != -1){
			if(m_encoder){
				m_encoder->flush();
				delete m_encoder;
				m_encoder = NULL;
			}
			m_file.close();
		}
	}
}

