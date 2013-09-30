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

#include "engine/sidechain/enginerecord.h"

#include "configobject.h"
#include "controlobject.h"
#include "controlobjectthread.h"
#include "encoder/encoder.h"
#include "encoder/encodermp3.h"
#include "encoder/encodervorbis.h"
#include "errordialoghandler.h"
#include "playerinfo.h"
#include "recording/defs_recording.h"

const int kMetaDataLifeTimeout = 16;

EngineRecord::EngineRecord(ConfigObject<ConfigValue>* _config)
        : m_pConfig(_config),
          m_pEncoder(NULL),
          m_pSndfile(NULL),
          m_iMetaDataLife(0) {
    m_pRecReady = new ControlObjectThread(RECORDING_PREF_KEY, "status");
    m_pSamplerate = new ControlObjectThread("[Master]", "samplerate");
}

EngineRecord::~EngineRecord() {
    closeCueFile();
    closeFile();
    delete m_pRecReady;
    delete m_pSamplerate;
}

void EngineRecord::updateFromPreferences() {
    m_encoding = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Encoding")).toLatin1();
    // returns a number from 1 .. 10
    m_OGGquality = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "OGG_Quality")).toLatin1();
    m_MP3quality = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "MP3_Quality")).toLatin1();
    m_fileName = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Path"));
    m_baTitle = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Title")).toLatin1();
    m_baAuthor = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Author")).toLatin1();
    m_baAlbum = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Album")).toLatin1();
    m_cueFileName = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "CuePath")).toLatin1();
    m_bCueIsEnabled = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "CueEnabled")).toInt();

    // Delete m_pEncoder if it has been initalized (with maybe) different bitrate.
    if (m_pEncoder) {
        delete m_pEncoder;
        m_pEncoder = NULL;
    }

    if (m_encoding == ENCODING_MP3) {
        m_pEncoder = new EncoderMp3(this);
        m_pEncoder->updateMetaData(m_baAuthor.data(), m_baTitle.data(), m_baAlbum.data());

        if(m_pEncoder->initEncoder(Encoder::convertToBitrate(m_MP3quality.toInt()),
                                  m_pSamplerate->get()) < 0) {
            delete m_pEncoder;
            m_pEncoder = NULL;
            qDebug() << "MP3 recording is not supported. Lame could not be initialized";
        }
    } else if (m_encoding == ENCODING_OGG) {
        m_pEncoder = new EncoderVorbis(this);
        m_pEncoder->updateMetaData(m_baAuthor.data(), m_baTitle.data(), m_baAlbum.data());

        if (m_pEncoder->initEncoder(Encoder::convertToBitrate(m_OGGquality.toInt()),
                                   m_pSamplerate->get()) < 0) {
            delete m_pEncoder;
            m_pEncoder = NULL;
            qDebug() << "OGG recording is not supported. OGG/Vorbis library could not be initialized";
        }
    }
    // If we use WAVE OR AIFF the encoder will be NULL at all times.
}

bool EngineRecord::metaDataHasChanged()
{
    if (m_iMetaDataLife < kMetaDataLifeTimeout) {
        m_iMetaDataLife++;
        return false;
    }
    m_iMetaDataLife = 0;

    TrackPointer pTrack = PlayerInfo::Instance().getCurrentPlayingTrack();
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

void EngineRecord::process(const CSAMPLE* pBuffer, const int iBufferSize) {

    float recordingStatus = m_pRecReady->get();

    if (recordingStatus == RECORD_OFF) {
        //qDebug("Setting record flag to: OFF");
        if (fileOpen()) {
            closeFile();  // Close file and free encoder.
            emit(isRecording(false));
        }
    } else if (recordingStatus == RECORD_READY) {
        // If we are ready for recording, i.e, the output file has been selected, we
        // open a new file.
        updateFromPreferences();  // Update file location from preferences.
        if (openFile()) {
            qDebug("Setting record flag to: ON");
            m_pRecReady->slotSet(RECORD_ON);
            emit(isRecording(true));  // will notify the RecordingManager

            // Since we just started recording, timeout and clear the metadata.
            m_iMetaDataLife = kMetaDataLifeTimeout;
            m_pCurrentTrack = TrackPointer();

            if (m_bCueIsEnabled) {
                openCueFile();
                m_cueSamplePos = 0;
                m_cueTrack = 0;
            }
        } else {  // Maybe the encoder could not be initialized
            qDebug("Setting record flag to: OFF");
            m_pRecReady->slotSet(RECORD_OFF);
            emit(isRecording(false));
        }
    } else if (recordingStatus == RECORD_ON) {
        // If recording is enabled process audio to compressed or uncompressed data.
        if (m_encoding == ENCODING_WAVE || m_encoding == ENCODING_AIFF) {
            if (m_pSndfile != NULL) {
                sf_write_float(m_pSndfile, pBuffer, iBufferSize);
                emit(bytesRecorded(iBufferSize * 2));
            }
        } else {
            if (m_pEncoder) {
                // Compress audio. Encoder will call method 'write()' below to
                // write a file stream
                m_pEncoder->encodeBuffer(pBuffer, iBufferSize);
            }
        }

        if (m_bCueIsEnabled) {
            if (metaDataHasChanged()) {
                m_cueTrack++;
                writeCueLine();
                m_cueFile.flush();
            }
            m_cueSamplePos += iBufferSize;
        }
  	}
}

void EngineRecord::writeCueLine() {
    if (!m_pCurrentTrack) {
        return;
    }

    // account for multiple channels
    unsigned long samplerate = m_pSamplerate->get() * 2;
    // CDDA is specified as having 75 frames a second
    unsigned long frames = ((unsigned long)
                                ((m_cueSamplePos / (samplerate / 75)))
                                    % 75);

    unsigned long seconds =  ((unsigned long)
                                (m_cueSamplePos / samplerate)
                                    % 60 );

    unsigned long minutes = m_cueSamplePos / (samplerate * 60);

    m_cueFile.write(QString("  TRACK %1 AUDIO\n")
            .arg((double)m_cueTrack, 2, 'f', 0, '0')
        .toLatin1()
    );

    m_cueFile.write(QString("    TITLE \"%1\"\n")
        .arg(m_pCurrentTrack->getTitle()).toLatin1());
    m_cueFile.write(QString("    PERFORMER \"%1\"\n")
        .arg(m_pCurrentTrack->getArtist()).toLatin1());

    // Woefully inaccurate (at the seconds level anyways).
    // We'd need a signal fired state tracker
    // for the track detection code.
    m_cueFile.write(QString("    INDEX 01 %1:%2:%3\n").arg(
        QString("%1").arg((double)minutes, 2, 'f', 0, '0'),
        QString("%1").arg((double)seconds, 2, 'f', 0, '0'),
        QString("%1").arg((double)frames, 2, 'f', 0, '0')).toLatin1()
    );
}

// Encoder calls this method to write compressed audio
void EngineRecord::write(unsigned char *header, unsigned char *body,
                         int headerLen, int bodyLen) {
    if (!fileOpen()) {
        return;
    }
    // Relevant for OGG
    if (headerLen > 0) {
        m_dataStream.writeRawData((const char*) header, headerLen);
    }
    // Always write body
    m_dataStream.writeRawData((const char*) body, bodyLen);
    emit(bytesRecorded((headerLen+bodyLen)));

}

bool EngineRecord::fileOpen() {
    // Both encoder and file must be initalized.
    if (m_encoding == ENCODING_WAVE || m_encoding == ENCODING_AIFF) {
        return (m_pSndfile != NULL);
    } else {
        return (m_file.handle() != -1);
    }
}

bool EngineRecord::openFile() {
    // Unfortunately, we cannot use QFile for writing WAV and AIFF audio.
    if (m_encoding == ENCODING_WAVE || m_encoding == ENCODING_AIFF){
        unsigned long samplerate = m_pSamplerate->get();
        // set sfInfo
        m_sfInfo.samplerate = samplerate;
        m_sfInfo.channels = 2;

        if (m_encoding == ENCODING_WAVE)
            m_sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
        else
            m_sfInfo.format = SF_FORMAT_AIFF | SF_FORMAT_PCM_16;

        // Creates a new WAVE or AIFF file and writes header information.
        m_pSndfile = sf_open(m_fileName.toLocal8Bit(), SFM_WRITE, &m_sfInfo);
        if (m_pSndfile) {
            sf_command(m_pSndfile, SFC_SET_NORM_FLOAT, NULL, SF_FALSE) ;
            // Set meta data
            int ret;

            ret = sf_set_string(m_pSndfile, SF_STR_TITLE, m_baTitle.data());
            if(ret != 0)
                qDebug("libsndfile: %s", sf_error_number(ret));

            ret = sf_set_string(m_pSndfile, SF_STR_ARTIST, m_baAuthor.data());
            if(ret != 0)
                qDebug("libsndfile: %s", sf_error_number(ret));

            ret = sf_set_string(m_pSndfile, SF_STR_COMMENT, m_baAlbum.data());
            if(ret != 0)
                qDebug("libsndfile: %s", sf_error_number(ret));

        }
    } else {
        // We can use a QFile to write compressed audio.
        if (m_pEncoder) {
            m_file.setFileName(m_fileName);
            m_file.open(QIODevice::WriteOnly);
            if (m_file.handle() != -1) {
                m_dataStream.setDevice(&m_file);
            }
        } else {
            return false;
        }
    }

    // Check if file is really open.
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
    if (m_cueFileName.length() <= 0) {
        return false;
    }

    qDebug() << "Opening Cue File:" << m_cueFileName;
    m_cueFile.setFileName(m_cueFileName);
    m_cueFile.open(QIODevice::WriteOnly);

    if (m_baAuthor.length() > 0) {
        m_cueFile.write(QString("PERFORMER \"%1\"\n")
                        .arg(QString(m_baAuthor).replace(QString("\""), QString("\\\"")))
                        .toLatin1());
    }

    if (m_baTitle.length() > 0) {
        m_cueFile.write(QString("TITLE \"%1\"\n")
                        .arg(QString(m_baTitle).replace(QString("\""), QString("\\\"")))
                        .toLatin1());
    }

    m_cueFile.write(QString("FILE \"%1\" %2%3\n").arg(
        QString(m_fileName).replace(QString("\""), QString("\\\"")),
        QString(m_encoding).toUpper(),
        m_encoding == ENCODING_WAVE ? "E" : " ").toLatin1());
    return true;
}

void EngineRecord::closeFile() {
    if (m_encoding == ENCODING_WAVE || m_encoding == ENCODING_AIFF) {
        if (m_pSndfile != NULL) {
            sf_close(m_pSndfile);
            m_pSndfile = NULL;
        }
    } else if (m_file.handle() != -1) {
        // Close QFile and encoder, if open.
        if (m_pEncoder) {
            m_pEncoder->flush();
            delete m_pEncoder;
            m_pEncoder = NULL;
        }
        m_file.close();
    }
}

void EngineRecord::closeCueFile() {
    if (m_cueFile.handle() != -1) {
        m_cueFile.close();
    }
}
