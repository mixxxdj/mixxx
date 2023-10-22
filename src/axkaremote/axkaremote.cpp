// axkaremote.cpp
// Remote control and monitoring system using local sockets
// Created 2023-10-05 by Axel Karjalainen <axel@axka.fi>
// vim:set et sw=4 ts=4 sts=4:

#include "axkaremote/axkaremote.h"
#include <qjsonarray.h>
#include <qjsondocument.h>
#include <qjsonobject.h>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "library/coverart.h"
#include "preferences/usersettings.h"

#include "errordialoghandler.h"
#include "mixer/playerinfo.h"
#include "util/event.h"

#include "mixer/playermanager.h"
#include <QDebug>

#include "moc_axkaremote.cpp"

namespace mixxx {

AxkaRemote::AxkaRemote(UserSettingsPointer &pConfig)
    : m_pConfig(pConfig) {
    m_time.start();
    qInfo() << "[AxkaRemote] initializing AxkaRemote...";

    createServer();

    broadcastConnectValue("xfader", "[Master]", "crossfader");

    // connect play buttons
    for (int deckNr = 0; deckNr < (int)PlayerManager::numDecks(); deckNr++) {
        QString group = PlayerManager::groupForDeck(deckNr);

        broadcastConnectValue(QStringLiteral("deck/%1/volume"), group, "volume");

        connectValue(group, "play", std::bind(&AxkaRemote::broadcastRate, this, deckNr));
        connectValue(group, "reverse", std::bind(&AxkaRemote::broadcastRate, this, deckNr));
        connectValue(group, "rate", std::bind(&AxkaRemote::broadcastRate, this, deckNr));
        connectValue(group, "rateRange", std::bind(&AxkaRemote::broadcastRate, this, deckNr));

        // TODO(axka): cast to bool
        broadcastConnectValue(QStringLiteral("deck/%1/is_playing"), group, "play");

        connectValue(group, "track_samples", std::bind(&AxkaRemote::broadcastPosition, this, deckNr));
        connectValue(group, "track_samplerate", std::bind(&AxkaRemote::broadcastPosition, this, deckNr));
        connectValue(group, "playposition", std::bind(&AxkaRemote::broadcastPosition, this, deckNr));

        // TODO: broadcast on deck (un)loaded and on metadata change
        connectValue(group, "play", std::bind(&AxkaRemote::broadcastMetadata, this, deckNr));
        // FIXME: this combined with getMetadata locking a mutex segfaults Mixxx
        // connectValue(group, "track_loaded", std::bind(&AxkaRemote::broadcastMetadata, this, deckNr));
    }
    qInfo() << "[AxkaRemote] initialized AxkaRemote!";
}

void AxkaRemote::broadcastValue(QString name, QString group, QString item) {
    ControlProxy control(ConfigKey(group, item));
    wsBroadcast(name, control.get());
}
void AxkaRemote::broadcastConnectValue(QString name, QString group, QString item) {
    qInfo() << "[AxkaRemote] broadcastConnectValue";
    auto control = ControlDoublePrivate::getControl(ConfigKey(group, item));
    DEBUG_ASSERT(control);
    control->setParent(this);
    connect(control.data(), &ControlDoublePrivate::valueChanged, [group, item, name, this] (double value, QObject * sender) {
        qInfo() << "[AxkaRemote] broadcastConnectValue changed for input" << group << item;
        wsBroadcast(name, value);
    });
    /*ControlProxy control(ConfigKey(group, item), this);
    control.connectValueChanged(this, [name, &control, this] () {
        qInfo() << "[AxkaRemote] broadcastConnectValue changed";
        wsBroadcast(name, control.get());
    });
    m_connectedControls.append(&control);*/
}
void AxkaRemote::connectValue(QString group, QString item, std::function<void (double value)> callback) {
    qInfo() << "[AxkaRemote] connectValue";
    auto control = ControlDoublePrivate::getControl(ConfigKey(group, item));
    DEBUG_ASSERT(control);
    control->setParent(this);
    connect(control.data(), &ControlDoublePrivate::valueChanged, [group, item, callback] (double value, QObject * sender) {
        qInfo() << "[AxkaRemote] connectValue changed for input" << group << item;
        callback(value);
    });
    /*ControlProxy control(ConfigKey(group, item), this);
    control.connectValueChanged(this, callback);
    m_connectedControls.append(&control);*/
}

AxkaRemote::~AxkaRemote() {
    // FIXME: doesn't do anything .. update: causes a coredump segfault
    /*for (ControlProxy *pControl : std::as_const(m_connectedControls)) {
        delete pControl;
    }*/
}

void AxkaRemote::broadcastRate(int deckNr) {
    qInfo() << "[AxkaRemote] broadcastRate";
    QString group = PlayerManager::groupForDeck(deckNr);

    bool is_reversing = ControlProxy(group, "reverse").toBool();
    double rate_multiplier = ControlProxy(group, "rate").get() * (is_reversing ? -1 : 1);
    double rate_max = ControlProxy(group, "rateRange").get();

    // Default:
    // rate_multiplier: 1
    // rate_max: 0.08
    wsBroadcast(QStringLiteral("deck/%1/rate").arg(deckNr), QJsonObject({
            {"rate_multiplier", rate_multiplier},
            {"rate_max", rate_max},
    }));
}

void AxkaRemote::broadcastPosition(int deckNr) {
    qInfo() << "[AxkaRemote] broadcastPosition";
    QString group = PlayerManager::groupForDeck(deckNr);

    // TODO: replace with Track::getDuration() ?
    double track_samples = ControlProxy(group, "track_samples").get();
    double track_samplerate = ControlProxy(group, "track_samplerate").get();
    double duration = track_samples / track_samplerate / 2;

    double playposition_mul = ControlProxy(group, "playposition").get();
    double playposition_sec = playposition_mul * duration;

    // Default:
    // duration: -1
    // playposition_mul: 0
    // playposition_sec: 0
    wsBroadcast(QStringLiteral("deck/%1/playposition").arg(deckNr), QJsonObject({
            {"duration", duration},
            {"playposition_mul", playposition_mul},
            {"playposition_sec", playposition_sec},
    }));
}

void AxkaRemote::broadcastMetadata(int deckNr) {
    qInfo() << "[AxkaRemote] broadcastMetadata";
    QString group = PlayerManager::groupForDeck(deckNr);

    PlayerInfo &playerInfo = PlayerInfo::instance();
    TrackPointer pTrack =
        playerInfo.getTrackInfo(PlayerManager::groupForDeck(deckNr));
    TrackMetadata meta = pTrack->getMetadata();

    CoverInfo::LoadedImage img = pTrack->getCoverInfoWithLocation().loadImage(pTrack);
    // TODO: use cache and don't send the image over websockets
    // TODO: check img.result (CoverInfo::LoadedImage::Result)
    // TODO: check img.image null

    // Default:
    // title: ""
    // duration: -1
    // lyrics: ""
    wsBroadcast(QStringLiteral("deck/%1/metadata").arg(deckNr), QJsonObject({
            {"title", pTrack->getTitleInfo()}, // getTitle doesn't fallback to the filename
            {"duration", pTrack->getDuration()},
            {"lyrics", meta.getTrackInfo().getLyrics()},
    }));
}

void AxkaRemote::createServer() {
    m_server.route("/", [] () {
        return "Mixxx AxkaRemote API server";
    });

    m_server.route("/ws", [](QHttpServerResponder&&) {});
    connect(&m_server, &QHttpServer::newWebSocketConnection, this, &AxkaRemote::handleWebSocketConnection);

    const auto port = m_server.listen(QHostAddress::Any, 8001);
    if (!port) {
        qWarning() << "Server failed to listen on a port.";
        return;
    }
    // TODO: if (CmdlineArgs::Instance().getDeveloper()) {
    qInfo().noquote() << QStringLiteral("[AxkaRemote]   listening on http://127.0.0.1:%1 (Press CTRL+C to quit)").arg(port);
}

static QString getIdentifier(QWebSocket *peer)
{
    return QStringLiteral("%1:%2").arg(peer->peerAddress().toString(),
                                       QString::number(peer->peerPort()));
}
void AxkaRemote::wsBroadcast(QString kind, QJsonValue value) {
    auto doc = QJsonDocument(QJsonObject {
        {
            {"kind", kind},
            {"data", value}
        }
    });
    auto message = QString(doc.toJson());
    for (QWebSocket *pClient : std::as_const(m_clients)) {
        pClient->sendTextMessage(message);
    }
}

void AxkaRemote::handleWebSocketConnection() {
    auto *pSocket= m_server.nextPendingWebSocketConnection().release();
    if (pSocket == nullptr) {
        // This should never happen when using the connection signal
        return;
    }
    qInfo() << "[AxkaRemote]" << getIdentifier(pSocket) << "connected!";
    pSocket->setParent(this);
    connect(pSocket, &QWebSocket::textMessageReceived, [this] (QString message) {
            QWebSocket *pSender = qobject_cast<QWebSocket *>(sender());
            qInfo() << "[AxkaRemote]" << getIdentifier(pSender) << "sent message:" << message;
    });
    connect(pSocket, &QWebSocket::disconnected, [this] () {
            qInfo() << "[AxkaRemote] disconnecting...";
            QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
            if (pClient) {
                qInfo() << "[AxkaRemote]" << getIdentifier(pClient) << "disconnected!";
                m_clients.removeAll(pClient);
                pClient->deleteLater();
            }
    });
    m_clients << pSocket;
    wsBroadcast("clientJoined", getIdentifier(pSocket));
}

} // namespace mixxx
