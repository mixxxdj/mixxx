//
// C++ Implementation: mixxxsocketclient
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "mixxxsocketclient.h"
#include "controlobject.h"
#include "enginebuffer.h"
#include "track.h"
#include "trackcollection.h"
#include "trackinfoobject.h"

MixxxSocketClient::MixxxSocketClient(Track *pTrack, int sock, QObject *parent, const char *name) : QSocket(parent, name)
{
    m_pTrack = pTrack;

    connect(this, SIGNAL(readyRead()), SLOT(readClient()));
    connect(this, SIGNAL(connectionClosed()), SLOT(deleteLater()));
    setSocket(sock);

    ControlObject *p;
    p = ControlObject::getControl(ConfigKey("[Channel1]","TrackEnd"));
    connect(p, SIGNAL(signalUpdateApp(double)), this, SLOT(slotEndOfFileCh1(double)));
    p = ControlObject::getControl(ConfigKey("[Channel2]","TrackEnd"));
    connect(p, SIGNAL(signalUpdateApp(double)), this, SLOT(slotEndOfFileCh2(double)));

    ts.setDevice(this);

    // Identification
    ts << "Mixxx " << VERSION << endl;
}


MixxxSocketClient::~MixxxSocketClient()
{
}

void MixxxSocketClient::readClient()
{
    while (canReadLine())
    {
        QString str = ts.readLine();

        // Split string into player, command and argument
        QString player = str.section(" ", 0, 0);
        QString command = str.section(" ", 1, 1);
        QString argument = str.section(" ", 2, 2);
        QString response = "OK";

        // Find out if the command is for player 1 or 2:
        if (player=="quit")
            emit(connectionClosed());
        else
        {
            QString group;
            if (player=="player1")
                group = "[Channel1]";
            else if (player=="player2")
                group = "[Channel2]";
            else
                response == "ERR";

            if (response=="OK")
            {
                if (command=="stop_on_eof")
                {
                    ControlObject *p = ControlObject::getControl(ConfigKey(group,"TrackEndMode"));
                    p->setValueFromApp(TRACK_END_MODE_STOP);
                }
                else if (command=="stop")
                {
                    ControlObject *p = ControlObject::getControl(ConfigKey(group,"play"));
                    p->setValueFromApp(0.);
                }
                else if (command=="start")
                {
                    ControlObject *p = ControlObject::getControl(ConfigKey(group,"play"));
                    p->setValueFromApp(1.);
                }
                else if (command=="load")
                {
                    TrackInfoObject *pTrack = m_pTrack->getTrackCollection()->getTrack(argument);
                    if (pTrack)
                    {
                        if (group=="[Channel1]")
                            m_pTrack->slotLoadPlayer1(pTrack);
                        else
                            m_pTrack->slotLoadPlayer2(pTrack);
                    }
                }
                else
                    response = "ERR";
            }
        }

        ts << response << endl;
    }
}

void MixxxSocketClient::slotEndOfFileCh1(double)
{
    qDebug("finsih");
    ts << "player1 finished" << endl;
}

void MixxxSocketClient::slotEndOfFileCh2(double)
{
    ts << "player2 finished" << endl;
}








