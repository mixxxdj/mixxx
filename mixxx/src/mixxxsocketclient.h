//
// C++ Interface: mixxxsocketclient
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MIXXXSOCKETCLIENT_H
#define MIXXXSOCKETCLIENT_H

#include <q3socket.h>
#include <q3textstream.h>

class Track;

/**
@author Tue Haste Andersen
*/
class MixxxSocketClient : public Q3Socket
{
    Q_OBJECT
public:
    MixxxSocketClient(Track *pTrack, int sock, QObject *parent=0, const char *name=0);
    ~MixxxSocketClient();

public slots:
    void slotEndOfFileCh1(double);
    void slotEndOfFileCh2(double);
private slots:
    void readClient();

private:
    Track *m_pTrack;
    Q3TextStream ts;
};

#endif
