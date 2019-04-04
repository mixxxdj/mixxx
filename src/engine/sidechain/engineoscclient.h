/***************************************************************************
                          EngineOscClient.h  -  description
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

#ifndef EngineOscClient_H
#define EngineOscClient_H


#include "preferences/usersettings.h"
#include "engine/sidechain/sidechainworker.h"
#include "track/track.h"
#include <QTime>
#include <QList>
#include "lo/lo.h"
#include "control/controlobject.h"
#include "control/controlproxy.h"

class ConfigKey;
class Encoder;

class EngineOscClient : public QObject, public SideChainWorker {
    Q_OBJECT
  public:
    EngineOscClient(UserSettingsPointer& pConfig);
    virtual ~EngineOscClient();

public slots:
    void sendState();
    void maybeSendState();
    void connectServer();


    //interface SideChainWorker
    void process(const CSAMPLE* pBuffer, const int iBufferSize);
    void shutdown() {}


  private:
    QTime time;
    lo_address serverAdress;
    UserSettingsPointer m_pConfig;
    QList<ControlProxy*> connectedControls;
    ControlProxy prefUpdate;

};

#endif
