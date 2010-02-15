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

//#include "engineobject.h"
#include "engineabstractrecord.h"
#include "configobject.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"

#include <shout/shout.h>

#include <QObject>

#include "encoder.h"

//class ControlLogpotmeter;
//class ConfigKey;
class EncoderVorbis;
class TrackInfoObject;

class EngineShoutcast : public EngineAbstractRecord {
    Q_OBJECT
public:
    EngineShoutcast(ConfigObject<ConfigValue> *_config);
    ~EngineShoutcast();
    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
    void writePage(unsigned char *header, unsigned char *body,
                   int headerLen, int bodyLen);

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
    bool serverConnect();
    int getActiveTracks();
    bool metaDataHasChanged();
    void updateMetaData();
    TrackInfoObject *m_pMetaData;
    shout_t *m_pShout;
    shout_metadata_t *m_pShoutMetaData;
    int m_pMetaDataLife;
    long m_iShoutStatus;
    long m_iShoutFailures;
    ConfigObject<ConfigValue> *m_pConfig;
    ControlObject* recReady;
    Encoder *encoder;
    ControlObjectThreadMain* m_pUpdateShoutcastFromPrefs;
//    void (*writeFn)(unsigned char *, unsigned char *, int, int);
    ControlObjectThread* m_pCrossfader;
    ControlObjectThread* m_pVolume1;
    ControlObjectThread* m_pVolume2;
    volatile bool m_bQuit;
};

#endif
