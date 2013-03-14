/***************************************************************************
                          engineshoutcast.h  -  description
                             -------------------
    copyright            : (C) 2007 by John Sully
                           (C) 2007 by Albert Santoni
                           (C) 2007 by Wesley Stessens
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENGINESHOUTCAST_H
#define ENGINESHOUTCAST_H

#include <QObject>
#include <QMutex>
#include <QMessageBox>
#include <QTextCodec>

#include <shout/shout.h>

#include "engineabstractrecord.h"
#include "configobject.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "trackinfoobject.h"
#include "errordialoghandler.h"
#include "recording/encoder.h"

#define SHOUTCAST_DISCONNECTED 0
#define SHOUTCAST_CONNECTING 1
#define SHOUTCAST_CONNECTED 2

class EncoderVorbis;

class EngineShoutcast : public EngineAbstractRecord {
    Q_OBJECT
  public:
    EngineShoutcast(ConfigObject<ConfigValue> *_config);
    virtual ~EngineShoutcast();

    // This is called by the Engine implementation for each sample. Encode and
    // send the stream, as well as check for metadata changes.
    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);

    // Called by the encoder in method 'encodebuffer()' to flush the stream to
    // the server.
    void write(unsigned char *header, unsigned char *body,
               int headerLen, int bodyLen);
    /** connects to server **/
    bool serverConnect();
    bool serverDisconnect();
    bool isConnected();
    void shutdown() {
        m_bQuit = true;
    }
  public slots:
    /** Update the libshout struct with info from Mixxx's shoutcast preferences.*/
    void updateFromPreferences();
    //    static void wrapper2writePage();
    //private slots:
    //    void writePage(unsigned char *header, unsigned char *body,
    //                   int headerLen, int bodyLen, int count);
  private:
    int getActiveTracks();
    // Check if the metadata has changed since the previous check.  We also
    // check when was the last check performed to avoid using too much CPU and
    // as well to avoid changing the metadata during scratches.
    bool metaDataHasChanged();
    // Update shoutcast metadata. This does not work for OGG/Vorbis and Icecast,
    // since the actual OGG/Vorbis stream contains the metadata.
    void updateMetaData();
    // Common error dialog creation code for run-time exceptions. Notify user
    // when connected or disconnected and so on
    void errorDialog(QString text, QString detailedError);
    void infoDialog(QString text, QString detailedError);

    QByteArray encodeString(const QString& string);
    QTextCodec* m_pTextCodec;
    TrackPointer m_pMetaData;
    shout_t *m_pShout;
    shout_metadata_t *m_pShoutMetaData;
    int m_iMetaDataLife;
    long m_iShoutStatus;
    long m_iShoutFailures;
    ConfigObject<ConfigValue> *m_pConfig;
    Encoder *m_encoder;
    ControlObject* m_pShoutcastNeedUpdateFromPrefs;
    ControlObjectThreadMain* m_pUpdateShoutcastFromPrefs;
    ControlObjectThread* m_pMasterSamplerate;
    ControlObjectThread* m_pShoutcastStatus;
    volatile bool m_bQuit;
    QMutex m_shoutMutex;
    // static metadata according to prefereneces
    bool m_custom_metadata;
    QByteArray m_baCustomSong;

    // when static metadata is used, we only need calling shout_set_metedata
    // once
    bool m_firstCall;

    bool m_format_is_mp3;
    bool m_format_is_ov;
    bool m_protocol_is_icecast1;
    bool m_protocol_is_icecast2;
    bool m_protocol_is_shoutcast;
    bool m_ogg_dynamic_update;
};

#endif
