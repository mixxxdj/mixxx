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

    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
    /**writes to shoutcast stream **/
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
    bool metaDataHasChanged();
    void updateMetaData();

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
    /** static metadata according to prefereneces **/
    bool m_custom_metadata;
    QByteArray m_baCustom_artist;
    QByteArray m_baCustom_title;
    QByteArray m_baFormat;
    /* Standard error dialog */
    void errorDialog(QString text, QString detailedError);
    /** we static metadata is used, we only need calling shout_set_metedata once */
    bool m_firstCall;
};

#endif
