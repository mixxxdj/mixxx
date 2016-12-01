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

#include "preferences/usersettings.h"
#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "encoder/encoder.h"

#ifdef __FFMPEGFILE__
#include "encoder/encoderffmpegmp3.h"
#include "encoder/encoderffmpegvorbis.h"
#else
#include "encoder/encodermp3.h"
#include "encoder/encodervorbis.h"
#endif

#include "mixer/playerinfo.h"
#include "recording/defs_recording.h"
#include "util/event.h"

const int kMetaDataLifeTimeout = 16;

EngineRecord::EngineRecord(UserSettingsPointer pConfig)
        : m_pConfig(pConfig),
          m_pEncoder(NULL),
          m_pSndfile(NULL),
          m_frames(0),
          m_recordedDuration(0),
          m_iMetaDataLife(0),
          m_cueTrack(0),
          m_bCueIsEnabled(false) {
    m_sfInfo.frames = 0;
    m_sfInfo.samplerate = 0;
    m_sfInfo.channels = 0;
    m_sfInfo.format = 0;
    m_sfInfo.sections = 0;
    m_sfInfo.seekable = 0;

    m_pRecReady = new ControlProxy(RECORDING_PREF_KEY, "status", this);
    m_pSamplerate = new ControlProxy("[Master]", "samplerate", this);
    m_sampleRate = m_pSamplerate->get();
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
    m_sampleRate = m_pSamplerate->get();

    // Delete m_pEncoder if it has been initialized (with maybe) different bitrate.
    if (m_pEncoder) {
        delete m_pEncoder;
        m_pEncoder = NULL;
    }

    if (m_encoding == ENCODING_MP3) {
#ifdef __FFMPEGFILE__
        m_pEncoder = new EncoderFfmpegMp3(this);
#else
        m_pEncoder = new EncoderMp3(this);
#endif
        m_pEncoder->updateMetaData(m_baAuthor.data(),m_baTitle.data(),m_baAlbum.data());

        if(m_pEncoder->initEncoder(Encoder::convertToBitrate(m_MP3quality.toInt()),
                                   m_sampleRate) < 0) {
            delete m_pEncoder;
            m_pEncoder = NULL;
#ifdef __FFMPEGFILE__
            qDebug() << "MP3 recording is not supported. FFMPEG mp3 could not be initialized";
#else
            qDebug() << "MP3 recording is not supported. Lame could not be initialized";
#endif
        }
    } else if (m_encoding == ENCODING_OGG) {
#ifdef __FFMPEGFILE__
        m_pEncoder = new EncoderFfmpegVorbis(this);
#else
        m_pEncoder = new EncoderVorbis(this);
#endif
        m_pEncoder->updateMetaData(m_baAuthor.data(),m_baTitle.data(),m_baAlbum.data());

        if (m_pEncoder->initEncoder(Encoder::convertToBitrate(m_OGGquality.toInt()),
                                   m_sampleRate) < 0) {
            delete m_pEncoder;
            m_pEncoder = NULL;
#ifdef __FFMPEGFILE__
            qDebug() << "OGG recording is not supported. FFMPEG OGG/Vorbis could not be initialized";
#else
            qDebug() << "OGG recording is not supported. OGG/Vorbis library could not be initialized";
#endif
        }
    }
    // If we use WAVE OR AIFF the encoder will be NULL at all times.
}

bool EngineRecord::metaDataHasChanged()
{
    //Originally, m_iMetaDataLife was used so that getCurrentPlayingTrack was called
    //less often, because it was calculating it.
    //Nowadays (since Mixxx 1.11), it just accesses a map on a thread safe method.
    TrackPointer pTrack = PlayerInfo::instance().getCurrentPlayingTrack();
    if (!pTrack) {
        m_iMetaDataLife = kMetaDataLifeTimeout;
        return false;
    }

    //The counter is kept so that changes back and forth with the faders/crossfader
    //(like in scratching or other effects) are not counted as multiple track changes
    //in the cue file. A better solution could consist of a signal from PlayerInfo and
    //a slot that decides if the changes received are valid or are to be ignored once
    //the next process call comes. This could also help improve the time written in the CUE.
    if (m_iMetaDataLife < kMetaDataLifeTimeout) {
        m_iMetaDataLife++;
        return false;
    }
    m_iMetaDataLife = 0;

    if (m_pCurrentTrack) {
        if (!pTrack->getId().isValid() || !m_pCurrentTrack->getId().isValid()) {
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
            Event::end("EngineRecord recording");
            closeFile();  // Close file and free encoder.
            if (m_bCueIsEnabled) {
                closeCueFile();
            }
            emit(isRecording(false, false));
        }
    } else if (recordingStatus == RECORD_READY) {
        // If we are ready for recording, i.e, the output file has been selected, we
        // open a new file.
        updateFromPreferences();  // Update file location from preferences.
        if (openFile()) {
            Event::start("EngineRecord recording");
            qDebug("Setting record flag to: ON");
            m_pRecReady->set(RECORD_ON);
            emit(isRecording(true, false));  // will notify the RecordingManager

            // Since we just started recording, timeout and clear the metadata.
            m_iMetaDataLife = kMetaDataLifeTimeout;
            m_pCurrentTrack.reset();

            // clean frames couting and get current sample rate.
            m_frames = 0;
            m_sampleRate = m_pSamplerate->get();

            if (m_bCueIsEnabled) {
                openCueFile();
                m_cueTrack = 0;
            }
        } else {  // Maybe the encoder could not be initialized
            qDebug() << "Could not open" << m_fileName << "for writing.";
            qDebug("Setting record flag to: OFF");
            m_pRecReady->slotSet(RECORD_OFF);
            // An error occurred.
            emit(isRecording(false, true));
        }
    } else if (recordingStatus == RECORD_SPLIT_CONTINUE) {
        if (fileOpen()) {
            closeFile();  // Close file and free encoder.
            if (m_bCueIsEnabled) {
                closeCueFile();
            }
        }
        updateFromPreferences();  // Update file location from preferences.
        if (openFile()) {
            qDebug() << "Splitting to a new file: "<< m_fileName;
            m_pRecReady->set(RECORD_ON);
            emit(isRecording(true, false));  // will notify the RecordingManager

            // Since we just started recording, timeout and clear the metadata.
            m_iMetaDataLife = kMetaDataLifeTimeout;
            m_pCurrentTrack.reset();

            // clean frames counting and get current sample rate.
            m_frames = 0;
            m_sampleRate = m_pSamplerate->get();
            m_recordedDuration = 0;

            if (m_bCueIsEnabled) {
                openCueFile();
                m_cueTrack = 0;
            }
        } else {  // Maybe the encoder could not be initialized
            qDebug() << "Could not open" << m_fileName << "for writing.";
            Event::end("EngineRecord recording");
            qDebug("Setting record flag to: OFF");
            m_pRecReady->slotSet(RECORD_OFF);
            // An error occurred.
            emit(isRecording(false, true));
        }
    }

    if (recordingStatus == RECORD_ON || recordingStatus == RECORD_SPLIT_CONTINUE) {
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
        
        //Writing cueLine before updating the time counter since we preffer to be ahead
        //rather than late.
        if (m_bCueIsEnabled) {
            if (metaDataHasChanged()) {
                m_cueTrack++;
                writeCueLine();
                m_cueFile.flush();
            }
        }

        // update frames counting and recorded duration (seconds)
        m_frames += iBufferSize / 2;
        unsigned long lastDuration = m_recordedDuration;
        m_recordedDuration = m_frames / m_sampleRate;

        // gets recorded duration and emit signal that will be used
        // by RecordingManager to update the label besides start/stop button
        if (lastDuration != m_recordedDuration) {
            emit(durationRecorded(m_recordedDuration));
        }
    }
}

QString EngineRecord::getRecordedDurationStr() {
    return QString("%1:%2")
                 .arg(m_recordedDuration / 60, 2, 'f', 0, '0')   // minutes
                 .arg(m_recordedDuration % 60, 2, 'f', 0, '0');  // seconds
}

void EngineRecord::writeCueLine() {
    if (!m_pCurrentTrack) {
        return;
    }

    // CDDA is specified as having 75 frames a second
    unsigned long cueFrame = ((unsigned long)
                                ((m_frames / (m_sampleRate / 75)))
                                    % 75);

    m_cueFile.write(QString("  TRACK %1 AUDIO\n")
                    .arg((double)m_cueTrack, 2, 'f', 0, '0')
                    .toLatin1());

    m_cueFile.write(QString("    TITLE \"%1\"\n")
        .arg(m_pCurrentTrack->getTitle()).toLatin1());
    m_cueFile.write(QString("    PERFORMER \"%1\"\n")
        .arg(m_pCurrentTrack->getArtist()).toLatin1());

    // Woefully inaccurate (at the seconds level anyways).
    // We'd need a signal fired state tracker
    // for the track detection code.
    m_cueFile.write(QString("    INDEX 01 %1:%2\n")
                    .arg(getRecordedDurationStr())
                    .arg((double)cueFrame, 2, 'f', 0, '0').toLatin1());
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
    // Both encoder and file must be initialized.
    if (m_encoding == ENCODING_WAVE || m_encoding == ENCODING_AIFF) {
        return (m_pSndfile != NULL);
    } else {
        return (m_file.handle() != -1);
    }
}

bool EngineRecord::openFile() {
    // Unfortunately, we cannot use QFile for writing WAV and AIFF audio.
    if (m_encoding == ENCODING_WAVE || m_encoding == ENCODING_AIFF) {
        // set sfInfo
        m_sfInfo.samplerate = m_sampleRate;
        m_sfInfo.channels = 2;

        if (m_encoding == ENCODING_WAVE) {
            m_sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
        } else {
            m_sfInfo.format = SF_FORMAT_AIFF | SF_FORMAT_PCM_16;
        }

        // Creates a new WAVE or AIFF file and writes header information.
#ifdef __WINDOWS__
        // Pointer valid until string changed
        LPCWSTR lpcwFilename = (LPCWSTR)m_fileName.utf16();
        m_pSndfile = sf_wchar_open(lpcwFilename, SFM_WRITE, &m_sfInfo);
#else
        m_pSndfile = sf_open(m_fileName.toLocal8Bit().constData(), SFM_WRITE, &m_sfInfo);
#endif
        if (m_pSndfile) {
            sf_command(m_pSndfile, SFC_SET_NORM_FLOAT, NULL, SF_TRUE);
            // Warning! Depending on how libsndfile is compiled autoclip may not work.
            // Ensure CPU_CLIPS_NEGATIVE and CPU_CLIPS_POSITIVE is setup properly in the build.
            sf_command(m_pSndfile, SFC_SET_CLIPPING, NULL, SF_TRUE) ;

            // Set meta data
            int ret = sf_set_string(m_pSndfile, SF_STR_TITLE, m_baTitle.constData());
            if (ret != 0) {
                qWarning("libsndfile error: %s", sf_error_number(ret));
            }

            ret = sf_set_string(m_pSndfile, SF_STR_ARTIST, m_baAuthor.constData());
            if (ret != 0) {
                qWarning("libsndfile error: %s", sf_error_number(ret));
            }

            ret = sf_set_string(m_pSndfile, SF_STR_COMMENT, m_baAlbum.constData());
            if (ret != 0) {
                qWarning("libsndfile error: %s", sf_error_number(ret));
            }
        }
    } else {
        // We can use a QFile to write compressed audio.
        if (m_pEncoder) {
            m_file.setFileName(m_fileName);
            if (!m_file.open(QIODevice::WriteOnly)) {
                return false;
            }
            if (m_file.handle() != -1) {
                m_dataStream.setDevice(&m_file);
            }
        } else {
            return false;
        }
    }

    // Return whether the file is really open.
    return fileOpen();
}

bool EngineRecord::openCueFile() {
    if (m_cueFileName.length() <= 0) {
        return false;
    }

    qDebug() << "Opening Cue File:" << m_cueFileName;
    m_cueFile.setFileName(m_cueFileName);

    // TODO(rryan): maybe we need to use the sandbox to get read/write rights on Mac OS ?!
    if (!m_cueFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Could not write Cue File:" << m_cueFileName;
        return false;
    }

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
        QFileInfo(m_fileName).fileName() //strip path
            .replace(QString("\""), QString("\\\"")), // escape doublequote
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
