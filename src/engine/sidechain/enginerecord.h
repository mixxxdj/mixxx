/***************************************************************************
                          enginerecord.h  -  description
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

#ifndef ENGINERECORD_H
#define ENGINERECORD_H

#include <QDataStream>
#include <QFile>

#ifdef Q_OS_WIN
//Enable unicode in libsndfile on Windows
//(sf_open uses UTF-8 otherwise)
#include <windows.h>
#define ENABLE_SNDFILE_WINDOWS_PROTOTYPES 1
#endif
#include <sndfile.h>

#include "preferences/usersettings.h"
#include "encoder/encodercallback.h"
#include "engine/sidechain/sidechainworker.h"
#include "track/track.h"

class ConfigKey;
class ControlProxy;
class Encoder;

class EngineRecord : public QObject, public EncoderCallback, public SideChainWorker {
    Q_OBJECT
  public:
    EngineRecord(UserSettingsPointer pConfig);
    virtual ~EngineRecord();

    void process(const CSAMPLE* pBuffer, const int iBufferSize);
    void shutdown() {}

    // writes compressed audio to file
    void write(unsigned char *header, unsigned char *body, int headerLen, int bodyLen);
    // creates or opens an audio file
    bool openFile();
    // closes the audio file
    void closeFile();
    void updateFromPreferences();
    bool fileOpen();
    bool openCueFile();
    void closeCueFile();

  signals:
    // emitted to notify RecordingManager
    void bytesRecorded(int bytes);

    // Emitted when recording state changes. 'recording' represents whether
    // recording is active and 'error' is true if an error occurred. Currently
    // only one error can occur: the specified file was unable to be opened for
    // writing.
    void isRecording(bool recording, bool error);
    void durationRecorded(QString duration);

  private:
    int getActiveTracks();

    // Check if the metadata has changed since the previous check. We also check
    // when was the last check performed to avoid using too much CPU and as well
    // to avoid changing the metadata during scratches.
    bool metaDataHasChanged();

    void writeCueLine();

    UserSettingsPointer m_pConfig;
    Encoder* m_pEncoder;
    QByteArray m_OGGquality;
    QByteArray m_MP3quality;
    QByteArray m_encoding;
    QString m_fileName;
    QByteArray m_baTitle;
    QByteArray m_baAuthor;
    QByteArray m_baAlbum;

    QFile m_file;
    QFile m_cueFile;
    QDataStream m_dataStream;
    SNDFILE* m_pSndfile;
    SF_INFO m_sfInfo;

    ControlProxy* m_pRecReady;
    ControlProxy* m_pSamplerate;
    quint64 m_frames;
    quint64 m_sampleRate;
    quint64 m_recordedDuration;
    QString getRecordedDurationStr();

    int m_iMetaDataLife;
    TrackPointer m_pCurrentTrack;

    QByteArray m_cueFileName;
    quint64 m_cueTrack;
    bool m_bCueIsEnabled;
};

#endif
