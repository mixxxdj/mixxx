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

#include <sndfile.h>

#include "encoder/encodercallback.h"
#include "configobject.h"
#include "engine/sidechain/sidechainworker.h"
#include "trackinfoobject.h"

class Encoder;
class ConfigKey;
class ControlObjectThread;

class EngineRecord : public QObject, public EncoderCallback, public SideChainWorker {
    Q_OBJECT
  public:
    EngineRecord(ConfigObject<ConfigValue>* _config);
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
    void bytesRecorded(int);
    void isRecording(bool);

  private:
    int getActiveTracks();

    // Check if the metadata has changed since the previous check. We also check
    // when was the last check performed to avoid using too much CPU and as well
    // to avoid changing the metadata during scratches.
    bool metaDataHasChanged();

    void writeCueLine();

    ConfigObject<ConfigValue>* m_pConfig;
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

    ControlObjectThread* m_pRecReady;
    ControlObjectThread* m_pSamplerate;

    int m_iMetaDataLife;
    TrackPointer m_pCurrentTrack;

    QByteArray m_cueFileName;
    quint64 m_cueSamplePos;
    quint64 m_cueTrack;
    bool m_bCueIsEnabled;
};

#endif
