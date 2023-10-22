// axkaremote.h
// Remote control and monitoring system using local sockets
// Created 2023-10-05 by Axel Karjalainen <axel@axka.fi>
// vim:set et sw=4 ts=4 sts=4:

#pragma once

#include <qelapsedtimer.h>
#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "preferences/usersettings.h"
#include "track/track.h"
#include <QList>
#include <QHttpServer>

namespace mixxx {

class AxkaRemote : public QObject {
    Q_OBJECT
    public:
        AxkaRemote(UserSettingsPointer &pConfig);
        virtual ~AxkaRemote();

    public slots:
        void createServer();
        void handleWebSocketConnection();

    private:
        QElapsedTimer m_time;
        //QLocalServer m_server;
        QHttpServer m_server;
        UserSettingsPointer m_pConfig;
        QList<ControlProxy *> m_connectedControls;
        QList<QWebSocket *> m_clients;

        void wsBroadcast(QString key, QJsonValue json);
        void broadcastPosition(int deckNr);
        void broadcastRate(int deckNr);
        void broadcastMetadata(int deckNr);
        void broadcastValue(QString name, QString group, QString item);
        void broadcastConnectValue(QString name, QString group, QString item);
        void connectValue(QString group, QString item, std::function<void (double value)> callback);

};

} // namespace mixxx
