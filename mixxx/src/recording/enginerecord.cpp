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
#include "trackinfoobject.h"
#include "dlgprefrecord.h"
#ifdef __SHOUTCAST__
#include "encodervorbis.h"
#include "encodermp3.h"
#endif

/***************************************************************************
 *                                                                         *
 * Notice To Future Developpers:                                           *
 * 	There is code here to write the file in a seperate thread              *
 * 	however it is unstable and has been abondoned.  Its only use           *
 * 	was to support low priority recording, however I don't think its       *
 * 	worth the trouble.                                                     *
 *                                                                         *
 ***************************************************************************/

EngineRecord::EngineRecord(ConfigObject<ConfigValue> * _config)
{
    m_config = _config;
    m_encoder = NULL;
    m_sndfile = NULL;

    m_recReady = new ControlObjectThread(
                               ControlObject::getControl(ConfigKey(RECORDING_PREF_KEY, "status")));
    m_samplerate = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Master]", "samplerate")));

    m_iMetaDataLife = 0;
}

EngineRecord::~EngineRecord()
{
    closeCueFile();
    closeFile();
    if(m_recReady)      delete m_recReady;
    if(m_samplerate)    delete m_samplerate;
}

void EngineRecord::updateFromPreferences()
{
    m_Encoding = m_config->getValueString(ConfigKey(RECORDING_PREF_KEY,"Encoding")).toLatin1();
    //returns a number from 1 .. 10
    m_OGGquality = m_config->getValueString(ConfigKey(RECORDING_PREF_KEY,"OGG_Quality")).toLatin1();
    m_MP3quality = m_config->getValueString(ConfigKey(RECORDING_PREF_KEY,"MP3_Quality")).toLatin1();
    m_filename = m_config->getValueString(ConfigKey(RECORDING_PREF_KEY,"Path"));
    m_baTitle = m_config->getValueString(ConfigKey(RECORDING_PREF_KEY, "Title")).toLatin1();
    m_baAuthor = m_config->getValueString(ConfigKey(RECORDING_PREF_KEY, "Author")).toLatin1();
    m_baAlbum = m_config->getValueString(ConfigKey(RECORDING_PREF_KEY, "Album")).toLatin1();
    m_cuefilename = m_config->getValueString(ConfigKey(RECORDING_PREF_KEY, "CuePath")).toLatin1();
    m_bCueIsEnabled = m_config->getValueString(ConfigKey(RECORDING_PREF_KEY, "CueEnabled")).toInt();

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

/*
 * Check if the metadata has changed since the previous check.
 * We also check when was the last check performed to avoid using
 * too much CPU and as well to avoid changing the metadata during
 * scratches.
 */
bool EngineRecord::metaDataHasChanged()
{
    TrackPointer pTrack;

    if ( m_iMetaDataLife < 16 ) {
        m_iMetaDataLife++;
        return false;
    }
    m_iMetaDataLife = 0;

    pTrack = PlayerInfo::Instance().getCurrentPlayingTrack();
    if ( !pTrack )
        return false;

    if ( m_pCurrentTrack ) {
        if ((pTrack->getId() == -1) || (m_pCurrentTrack->getId() == -1)) {
            if ((pTrack->getArtist() == m_pCurrentTrack->getArtist()) &&
                (pTrack->getTitle() == m_pCurrentTrack->getArtist())) {
                return false;
            }
        }
        else if (pTrack->getId() == m_pCurrentTrack->getId()) {
            return false;
        }
    }

    m_pCurrentTrack = pTrack;
    return true;
}

void EngineRecord::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize) {
    Q_UNUSED(pOut);
    // Calculate the latency of this buffer
    m_dLatency = (double)iBufferSize / m_samplerate->get();

    //if recording is disabled
    if (m_recReady->get() == RECORD_OFF) {
        //qDebug("Setting record flag to: OFF");
        if (fileOpen()) {
            closeFile();    //close file and free encoder
            emit(isRecording(false));
        }
    }
    //if we are ready for recording, i.e, the output file has been selected, we open a new file
    if (m_recReady->get() == RECORD_READY) {
        updateFromPreferences();	//update file location from pref
        if (openFile()) {
            qDebug("Setting record flag to: ON");
            m_recReady->slotSet(RECORD_ON);
            emit(isRecording(true)); //will notify the RecordingManager

            if (m_bCueIsEnabled) {
                openCueFile();
                m_cuesamplepos = 0;
                m_cuetrack = 0;
            }
        } else { // Maybe the encoder could not be initialized
            qDebug("Setting record flag to: OFF");
            m_recReady->slotSet(RECORD_OFF);
            emit(isRecording(false));
        }
    }
    //If recording is enabled process audio to compressed or uncompressed data.
    if (m_recReady->get() == RECORD_ON) {
        if (m_Encoding == ENCODING_WAVE || m_Encoding == ENCODING_AIFF) {
            if (m_sndfile != NULL) {
                sf_write_float(m_sndfile, pIn, iBufferSize);
                emit(bytesRecorded(iBufferSize));
            }
        } else {
            if (m_encoder) {
                //Compress audio. Encoder will call method 'write()' below to write a file stream
                m_encoder->encodeBuffer(pIn, iBufferSize);
            }
        }

        if (m_bCueIsEnabled) {
            if (metaDataHasChanged()) {
                m_cuetrack++;
                writeCueLine();
                m_cuefile.flush();
            }
            m_cuesamplepos += iBufferSize;
        }
  	}
}

void EngineRecord::writeCueLine() {
    // account for multiple channels
    unsigned long samplerate = m_samplerate->get() * 2;
    // CDDA is specified as having 75 frames a second
    unsigned long frames = ((unsigned long)
                                ((m_cuesamplepos / (samplerate / 75)))
                                    % 75);

    unsigned long seconds =  ((unsigned long)
                                (m_cuesamplepos / samplerate)
                                    % 60 );

    unsigned long minutes = m_cuesamplepos / (samplerate * 60);

    m_cuefile.write(QString("  TRACK %1 AUDIO\n")
            .arg((double)m_cuetrack, 2, 'f', 0, '0')
        .toLatin1()
    );

    m_cuefile.write(QString("    TITLE %1\n")
        .arg(m_pCurrentTrack->getTitle()).toLatin1());
    m_cuefile.write(QString("    PERFORMER %1\n")
        .arg(m_pCurrentTrack->getArtist()).toLatin1());

    // Woefully inaccurate (at the seconds level anyways).
    // We'd need a signal fired state tracker
    // for the track detection code.
    m_cuefile.write(QString("    INDEX 01 %1:%2:%3\n").arg(
        QString("%1").arg((double)minutes, 2, 'f', 0, '0'),
        QString("%1").arg((double)seconds, 2, 'f', 0, '0'),
        QString("%1").arg((double)frames, 2, 'f', 0, '0')).toLatin1()
    );
}

/** encoder will call this method to write compressed audio **/
void EngineRecord::write(unsigned char *header, unsigned char *body,
                         int headerLen, int bodyLen)
{
    if (!fileOpen()) {
        return;
    }
    //Relevant for OGG
    if (headerLen > 0) {
        m_datastream.writeRawData((const char*) header, headerLen);
    }
    //always write body
    m_datastream.writeRawData((const char*) body, bodyLen);
    emit(bytesRecorded((headerLen+bodyLen)));

}

bool EngineRecord::fileOpen() {
    // Both encoder and file must be initalized

    if (m_Encoding == ENCODING_WAVE || m_Encoding == ENCODING_AIFF) {
        return (m_sndfile != NULL);
    } else {
        return (m_file.handle() != -1);
    }
}

//Creates a new MP3 file
bool EngineRecord::openFile() {
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
        m_sndfile = sf_open(m_filename.toLocal8Bit(), SFM_WRITE, &m_sfInfo);
        if (m_sndfile) {
            sf_command(m_sndfile, SFC_SET_NORM_FLOAT, NULL, SF_FALSE) ;
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
    } else {
        //we can use a QFile to write compressed audio
        if (m_encoder) {
            m_file.setFileName(m_filename);
            m_file.open(QIODevice::WriteOnly);
            if (m_file.handle() != -1) {
                m_datastream.setDevice(&m_file);
            }
        } else {
            return false;
        }
    }
    //check if file are really open
    if (!fileOpen()) {
        ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
        props->setType(DLG_WARNING);
        props->setTitle(tr("Recording"));
        props->setText(tr("<html>Could not create audio file for recording!<p><br>Maybe you do not have enough free disk space or file permissions.</html>"));
        ErrorDialogHandler::instance()->requestErrorDialog(props);
        return false;
    }
    return true;
}

bool EngineRecord::openCueFile() {
    if (m_cuefilename.length() <= 0) {
        return false;
    }

    qDebug() << "Opening Cue File:" << m_cuefilename;
    m_cuefile.setFileName(m_cuefilename);
    m_cuefile.open(QIODevice::WriteOnly);

    if (m_baAuthor.length() > 0) {
        m_cuefile.write(QString("PERFORMER \"%1\"\n")
                        .arg(QString(m_baAuthor).replace(QString("\""), QString("\\\"")))
                        .toLatin1());
    }

    if (m_baTitle.length() > 0) {
        m_cuefile.write(QString("TITLE \"%1\"\n")
                        .arg(QString(m_baTitle).replace(QString("\""), QString("\\\"")))
                        .toLatin1());
    }

    m_cuefile.write(QString("FILE \"%1\" %2%3\n").arg(
        QString(m_filename).replace(QString("\""), QString("\\\"")),
        QString(m_Encoding).toUpper(),
        m_Encoding == ENCODING_WAVE ? "E" : " ").toLatin1());
    return true;
}

void EngineRecord::closeFile() {
    if (m_Encoding == ENCODING_WAVE || m_Encoding == ENCODING_AIFF) {
        if (m_sndfile != NULL) {
            sf_close(m_sndfile);
            m_sndfile = NULL;
        }
    } else if (m_file.handle() != -1) {
        // close QFile and encoder, if open
        if (m_encoder) {
            m_encoder->flush();
            delete m_encoder;
            m_encoder = NULL;
        }
        m_file.close();
    }
}

void EngineRecord::closeCueFile() {
    if ( m_cuefile.handle() != -1) {
        m_cuefile.close();
    }
}
