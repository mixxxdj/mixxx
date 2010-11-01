/***************************************************************************
                          enginerecord.cpp  -  class to record the mix
                             -------------------
    copyright            : (C) 2007 by John Sully
    copyright            : (C) 2010 by Tobias Rafreider
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

    m_recReadyCO = new ControlObject(ConfigKey("[Master]", "Record"));
    m_recReady = new ControlObjectThread(m_recReadyCO);
    m_samplerate = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Master]", "samplerate")));

}

EngineRecord::~EngineRecord()
{
    closeFile();
    if(m_recReadyCO)    delete m_recReadyCO;
    if(m_recReady)      delete m_recReady;
    if(m_samplerate)    delete m_samplerate;

}
void EngineRecord::updateFromPreferences()
{
    m_Encoding = m_config->getValueString(ConfigKey(RECORDING_PREF_KEY,"Encoding")).toLatin1();
    //returns a number from 1 .. 10
    m_OGGquality = m_config->getValueString(ConfigKey(RECORDING_PREF_KEY,"OGG_Quality")).toLatin1();
    m_MP3quality = m_config->getValueString(ConfigKey(RECORDING_PREF_KEY,"MP3_Quality")).toLatin1();
    m_filename = m_config->getValueString(ConfigKey(RECORDING_PREF_KEY,"Path")).toAscii();
    m_baTitle = m_config->getValueString(ConfigKey(RECORDING_PREF_KEY, "Title")).toLatin1();
    m_baAuthor = m_config->getValueString(ConfigKey(RECORDING_PREF_KEY, "Author")).toLatin1();
    m_baAlbum = m_config->getValueString(ConfigKey(RECORDING_PREF_KEY, "Album")).toLatin1();

    if(m_encoder){
        delete m_encoder;	//delete m_encoder if it has been initalized (with maybe) different bitrate
        m_encoder = NULL;
    }

    if(m_Encoding == ENCODING_MP3){
#ifdef __SHOUTCAST__
        m_encoder = new EncoderMp3(this);
        m_encoder->updateMetaData(m_baAuthor.data(),m_baTitle.data(),m_baAlbum.data());

        if(m_encoder->initEncoder(Encoder::convertToBitrate(m_MP3quality.toInt())) < 0){
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
        m_encoder = new EncoderVorbis(this);
        m_encoder->updateMetaData(m_baAuthor.data(),m_baTitle.data(),m_baAlbum.data());

        if(m_encoder->initEncoder(Encoder::convertToBitrate(m_OGGquality.toInt())) < 0){
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

void EngineRecord::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
{
    //if recording is disabled
    if(m_recReady->get() == RECORD_OFF){
        if(fileOpen()){
            closeFile();	//close file and free encoder
        }
    }
    //if we are ready for recording, i.e, the output file has been selected, we open a new file
    if(m_recReady->get() == RECORD_READY){
        updateFromPreferences();	//update file location from pref
        if(openFile()){
            qDebug("Setting record flag to: ON");
            m_recReady->slotSet(RECORD_ON);
        }
        else{ //Maybe the encoder could not be initialized
            qDebug("Setting record flag to: OFF");
            m_recReady->slotSet(RECORD_OFF);
        }
    }
    //If recording is enabled process audio to compressed or uncompressed data.
    if(m_recReady->get() == RECORD_ON){
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
        m_datastream.writeRawData((const char*) header, headerLen);
		}
		//always write body
		m_datastream.writeRawData((const char*) body, bodyLen);

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
        unsigned long samplerate = m_samplerate->get();
        //set sfInfo
        m_sfInfo.samplerate = samplerate;
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

            ret = sf_set_string(m_sndfile, SF_STR_TITLE, m_baTitle.data());
            if(ret != 0)
                qDebug("libsndfile: %s", sf_error_number(ret));

            ret = sf_set_string(m_sndfile, SF_STR_ARTIST, m_baAuthor.data());
            if(ret != 0)
                qDebug("libsndfile: %s", sf_error_number(ret));

            ret = sf_set_string(m_sndfile, SF_STR_COMMENT, m_baAlbum.data());
            if(ret != 0)
                qDebug("libsndfile: %s", sf_error_number(ret));

        }
    }
    else
    {
        //we can use a QFile to write compressed audio
        if(m_encoder){
            m_file.setFileName(m_filename);
            m_file.open(QIODevice::WriteOnly);
            if(m_file.handle() != -1){
                m_datastream.setDevice(&m_file);
            }
        }
        else{
            return false;
        }

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

