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

#include <qsocket.h>
#include <qtextstream.h>

class Track;

/**
@author Tue Haste Andersen
*/
class MixxxSocketClient : public QSocket
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
    QTextStream ts;
};

#endif
