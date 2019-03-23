/***************************************************************************
                          EngineOscClient.cpp  -  class to record the mix
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

#include "engine/sidechain/engineoscclient.h"

#include "preferences/usersettings.h"
#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "oscclient/defs_oscclient.h"

#include "errordialoghandler.h"
#include "mixer/playerinfo.h"
#include "util/event.h"

#include "mixer/playermanager.h"
#include <QDebug>

EngineOscClient::EngineOscClient(UserSettingsPointer& pConfig) :
    m_pConfig(pConfig),
    prefUpdate(ConfigKey("[Preferences]", "updated")){

    connectServer();

    ControlProxy* xfader = new ControlProxy(ConfigKey("[Master]", "crossfader"), this);
    xfader->connectValueChanged(this, &EngineOscClient::maybeSendState);
    connectedControls.append(xfader);

    //connect play buttons
    for(int deckNr = 0; deckNr < (int)PlayerManager::numDecks(); deckNr++){
        ControlProxy* play = new ControlProxy(ConfigKey(PlayerManager::groupForDeck(deckNr), "play"), this);
        play->connectValueChanged(this, &EngineOscClient::maybeSendState);
        connectedControls.append(play);

        ControlProxy* volume = new ControlProxy(ConfigKey(PlayerManager::groupForDeck(deckNr), "volume"), this);
        volume->connectValueChanged(this, &EngineOscClient::maybeSendState);
        connectedControls.append(volume);
    }

    //connect to settings changes
    connect(&prefUpdate,SIGNAL(valueChanged(double)),this,SLOT(connectServer()));
}

EngineOscClient::~EngineOscClient() {
}

void EngineOscClient::process(const CSAMPLE* pBuffer, const int iBufferSize) {
    if(time.elapsed() < 10)
        return;
    sendState();
}

void EngineOscClient::sendState()
{
    PlayerInfo& playerInfo = PlayerInfo::instance();
    int numDecks = (int)PlayerManager::numDecks();
    lo_send(serverAdress, "/mixxx/numDecks","i", numDecks);

    for(int deckNr = 0; deckNr < numDecks; deckNr++){
        lo_send(serverAdress, "/mixxx/deck/playing" ,"ii", deckNr,(int)playerInfo.isDeckPlaying(deckNr));
        lo_send(serverAdress, "/mixxx/deck/volume" ,"if",deckNr, playerInfo.getDeckVolume(deckNr));

        //speed
        ControlProxy rate(ConfigKey(PlayerManager::groupForDeck(deckNr), "rate"));
        ControlProxy rateRange(ConfigKey(PlayerManager::groupForDeck(deckNr), "rateRange"));
        ControlProxy rev(ConfigKey(PlayerManager::groupForDeck(deckNr), "reverse"));
        float speed = 1 + float(rate.get()) * float(rateRange.get());
        if(rev.get())
            speed *= -1;
        lo_send(serverAdress, "/mixxx/deck/speed" ,"if", deckNr, speed);
        
        ControlProxy posRel(ConfigKey(PlayerManager::groupForDeck(deckNr), "playposition"));
        lo_send(serverAdress, "/mixxx/deck/pos" ,"if", deckNr, float(posRel.get()));

        ControlProxy dur(ConfigKey(PlayerManager::groupForDeck(deckNr), "duration"));
        lo_send(serverAdress, "/mixxx/deck/duration" ,"if", deckNr, float(dur.get()));


        QString title = "";
        TrackPointer pTrack = playerInfo.getTrackInfo(PlayerManager::groupForDeck(deckNr));
        if(pTrack){
            title = pTrack->getTitle();
        }
        lo_send(serverAdress, "/mixxx/deck/title","is", deckNr , title.toUtf8().data());
    }
    time.restart();
}

void EngineOscClient::maybeSendState()
{
    if(time.elapsed() < 10)
        return;
    sendState();
}

void EngineOscClient::connectServer()
{
    QString server = m_pConfig->getValueString(ConfigKey(OSC_CLIENT_PREF_KEY, "Server"));
    QString port = m_pConfig->getValueString(ConfigKey(OSC_CLIENT_PREF_KEY, "Port"));
    serverAdress = lo_address_new(server.toLatin1().data(), port.toLatin1().data());
}
