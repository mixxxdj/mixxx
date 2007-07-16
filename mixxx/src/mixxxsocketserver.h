//
// C++ Interface: mixxxsocketserver
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MIXXXSOCKETSERVER_H
#define MIXXXSOCKETSERVER_H

#include <q3serversocket.h>

class Track;

/**
@author Tue Haste Andersen
*/
class MixxxSocketServer : public Q3ServerSocket
{
    Q_OBJECT
public:
    MixxxSocketServer(Track *pTrack, QObject *parent=0, const char *name=0);
    ~MixxxSocketServer();
    void newConnection(int socket);

private:
    Track *m_pTrack;

};

#endif
