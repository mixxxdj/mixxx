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

    /** writes (un)compressed audio to file **/
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

    ConfigObject<ConfigValue>* m_config;
    Encoder* m_encoder;
    QByteArray m_OGGquality;
    QByteArray m_MP3quality;
    QByteArray m_Encoding;
    QString m_filename;
    QByteArray m_baTitle;
    QByteArray m_baAuthor;
    QByteArray m_baAlbum;

    QFile m_file;
    QFile m_cuefile;
    QDataStream m_datastream;
    SNDFILE *m_sndfile;
    SF_INFO m_sfInfo;

    ControlObjectThread* m_recReady;
    ControlObjectThread* m_samplerate;

    int m_iMetaDataLife;
    TrackPointer m_pCurrentTrack;

    QByteArray m_cuefilename;
    quint64 m_cuesamplepos;
    quint64 m_cuetrack;
    bool m_bCueIsEnabled;
};

#endif
