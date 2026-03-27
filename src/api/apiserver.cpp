#include "api/apiserver.h"

#include <httplib.h>

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <chrono>
#include <thread>

#include "control/controlproxy.h"
#include "mixer/basetrackplayer.h"
#include "mixer/playermanager.h"
#include "track/track.h"

namespace {
const char* kApiTag = "[API]";

QJsonObject jsonOk(const QJsonObject& extra = {}) {
    QJsonObject obj{{"ok", true}};
    for (auto it = extra.begin(); it != extra.end(); ++it) {
        obj[it.key()] = it.value();
    }
    return obj;
}

QJsonObject jsonError(const QString& msg) {
    return QJsonObject{{"ok", false}, {"error", msg}};
}

std::string toJson(const QJsonObject& obj) {
    return QJsonDocument(obj).toJson(QJsonDocument::Compact).toStdString();
}

QJsonObject parseBody(const std::string& body) {
    return QJsonDocument::fromJson(QByteArray::fromStdString(body)).object();
}

} // anonymous namespace

ApiServer::ApiServer(int port,
        const QString& bindAddress,
        PlayerManager* pPlayerManager,
        QObject* parent)
        : QObject(parent),
          m_port(port),
          m_bindAddress(bindAddress),
          m_pPlayerManager(pPlayerManager) {
}

ApiServer::~ApiServer() {
    stop();
}

QString ApiServer::deckGroup(int deck) {
    return QString("[Channel%1]").arg(deck);
}

double ApiServer::getControl(const QString& group, const QString& key) {
    ControlProxy proxy(ConfigKey(group, key));
    return proxy.get();
}

void ApiServer::setControl(const QString& group, const QString& key, double value) {
    ControlProxy proxy(ConfigKey(group, key));
    proxy.set(value);
}

QJsonObject ApiServer::getTrackInfo(int deck) {
    QJsonObject result;
    if (!m_pPlayerManager) {
        return result;
    }

    // PlayerManager and Track objects live on the main thread.
    // Use BlockingQueuedConnection to safely read from the API thread.
    QMetaObject::invokeMethod(
            qApp,
            [this, deck, &result]() {
                QString group = deckGroup(deck);
                BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(group);
                if (!pPlayer) {
                    return;
                }
                TrackPointer pTrack = pPlayer->getLoadedTrack();
                if (!pTrack) {
                    return;
                }

                result["title"] = pTrack->getTitle();
                result["artist"] = pTrack->getArtist();
                result["album"] = pTrack->getAlbum();
                result["album_artist"] = pTrack->getAlbumArtist();
                result["genre"] = pTrack->getGenre();
                result["composer"] = pTrack->getComposer();
                result["comment"] = pTrack->getComment();
                result["year"] = pTrack->getYear();
                result["track_number"] = pTrack->getTrackNumber();
                result["bpm"] = pTrack->getBpm();
                result["key"] = pTrack->getKeyText();
                result["duration"] = pTrack->getDuration();
                result["duration_text"] = pTrack->getDurationTextSeconds();
                result["bitrate"] = pTrack->getBitrate();
                result["sample_rate"] = static_cast<int>(pTrack->getSampleRate().value());
                result["channels"] = static_cast<int>(pTrack->getChannels().value());
                result["file_path"] = pTrack->getLocation();
                result["file_type"] = pTrack->getType();
                result["rating"] = pTrack->getRating();
                result["times_played"] = pTrack->getTimesPlayed();
                result["bpm_locked"] = pTrack->isBpmLocked();

                // Beat grid info
                auto pBeats = pTrack->getBeats();
                if (pBeats) {
                    QJsonObject beatsObj;
                    beatsObj["has_beats"] = true;
                    beatsObj["constant_tempo"] = pBeats->hasConstantTempo();

                    auto firstBeat = pBeats->firstBeat();
                    if (firstBeat.isValid()) {
                        beatsObj["first_beat_frame"] = firstBeat.value();
                    }

                    result["beats"] = beatsObj;
                } else {
                    QJsonObject beatsObj;
                    beatsObj["has_beats"] = false;
                    result["beats"] = beatsObj;
                }

                // Waveform summary — downsampled to ~200 points
                auto pWaveform = pTrack->getWaveformSummary();
                if (pWaveform) {
                    QJsonObject waveObj;
                    waveObj["has_waveform"] = true;
                    waveObj["data_size"] = pWaveform->getDataSize();
                    waveObj["texture_size"] = pWaveform->getTextureSize();

                    int dataSize = pWaveform->getDataSize();
                    if (dataSize > 0) {
                        int targetPoints = 200;
                        int step = std::max(1, dataSize / targetPoints);
                        QJsonArray lowArr, midArr, highArr;
                        for (int i = 0; i < dataSize; i += step) {
                            lowArr.append(static_cast<int>(pWaveform->getLow(i)));
                            midArr.append(static_cast<int>(pWaveform->getMid(i)));
                            highArr.append(static_cast<int>(pWaveform->getHigh(i)));
                        }
                        waveObj["low"] = lowArr;
                        waveObj["mid"] = midArr;
                        waveObj["high"] = highArr;
                    }

                    result["waveform_summary"] = waveObj;
                } else {
                    QJsonObject waveObj;
                    waveObj["has_waveform"] = false;
                    result["waveform_summary"] = waveObj;
                }

                // Cue points
                auto cues = pTrack->getCuePoints();
                if (!cues.isEmpty()) {
                    QJsonArray cueArr;
                    for (const auto& cue : cues) {
                        QJsonObject cueObj;
                        cueObj["type"] = static_cast<int>(cue->getType());
                        cueObj["hotcue_index"] = cue->getHotCue();
                        auto startPos = cue->getPosition();
                        if (startPos.isValid()) {
                            cueObj["position_frames"] = startPos.value();
                        }
                        auto endPos = cue->getEndPosition();
                        if (endPos.isValid()) {
                            cueObj["end_position_frames"] = endPos.value();
                        }
                        cueObj["label"] = cue->getLabel();
                        auto color = cue->getColor();
                        cueObj["color"] = static_cast<double>(static_cast<QRgb>(color));
                        cueArr.append(cueObj);
                    }
                    result["cue_points"] = cueArr;
                }
            },
            Qt::BlockingQueuedConnection);

    return result;
}

void ApiServer::start() {
    if (m_running) {
        return;
    }

    m_thread = std::make_unique<QThread>();
    m_thread->setObjectName("ApiServerThread");
    QThread* thread = m_thread.get();
    connect(thread, &QThread::started, this, &ApiServer::run);
    moveToThread(thread);
    thread->start();
    qInfo() << kApiTag << "Starting on" << m_bindAddress << "port" << m_port;
}

void ApiServer::stop() {
    m_running = false;
    if (m_thread && m_thread->isRunning()) {
        m_thread->quit();
        m_thread->wait(5000);
    }
}

void ApiServer::run() {
    httplib::Server svr;
    m_running = true;

    // CORS — allow cross-origin requests from local tools and web UIs
    svr.set_default_headers({
            {"Access-Control-Allow-Origin", "*"},
            {"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
            {"Access-Control-Allow-Headers", "Content-Type"},
    });
    svr.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        res.status = 204;
    });

    // ════════════════════════════════════════════════════════════════
    // LIVE — Real-time audio state (designed for 10Hz polling)
    // ════════════════════════════════════════════════════════════════

    svr.Get("/api/live", [this](const httplib::Request&, httplib::Response& res) {
        auto deckLive = [this](int d) -> QJsonObject {
            QString g = deckGroup(d);
            return QJsonObject{
                    {"playing", getControl(g, "play") > 0.5},
                    {"bpm", getControl(g, "bpm")},
                    {"beat_active", getControl(g, "beat_active") > 0.5},
                    {"beat_distance", getControl(g, "beat_distance")},
                    {"playposition", getControl(g, "playposition")},
                    {"volume", getControl(g, "volume")},
                    {"vu_left", getControl(g, "VuMeterL")},
                    {"vu_right", getControl(g, "VuMeterR")},
                    {"peak_indicator", getControl(g, "PeakIndicator") > 0.5},
            };
        };

        QJsonObject live{
                {"timestamp", static_cast<qint64>(QDateTime::currentMSecsSinceEpoch())},
                {"crossfader", getControl("[Master]", "crossfader")},
                {"master_vu_left", getControl("[Master]", "VuMeterL")},
                {"master_vu_right", getControl("[Master]", "VuMeterR")},
                {"deck1", deckLive(1)},
                {"deck2", deckLive(2)},
        };
        res.set_content(toJson(live), "application/json");
    });

    // ════════════════════════════════════════════════════════════════
    // STATUS — Full mixer and deck state snapshot
    // ════════════════════════════════════════════════════════════════

    svr.Get("/api/status", [this](const httplib::Request&, httplib::Response& res) {
        auto deck = [this](int d) -> QJsonObject {
            QString g = deckGroup(d);
            double dur = getControl(g, "duration");
            double pos = getControl(g, "playposition");
            double posSeconds = pos * dur;
            double remaining = dur - posSeconds;

            return QJsonObject{
                    {"deck", d},
                    {"playing", getControl(g, "play") > 0.5},
                    {"volume", getControl(g, "volume")},
                    {"bpm", getControl(g, "bpm")},
                    {"visual_bpm", getControl(g, "visual_bpm")},
                    {"file_bpm", getControl(g, "file_bpm")},
                    {"position", pos},
                    {"position_seconds", posSeconds},
                    {"remaining_seconds", remaining},
                    {"duration", dur},
                    {"track_loaded", getControl(g, "track_loaded") > 0.5},
                    {"sync_enabled", getControl(g, "sync_enabled") > 0.5},
                    {"rate", getControl(g, "rate")},
                    {"key", getControl(g, "key")},
                    {"visual_key", getControl(g, "visual_key")},
                    {"beat_active", getControl(g, "beat_active") > 0.5},
                    {"loop_enabled", getControl(g, "loop_enabled") > 0.5},
                    {"loop_start_position", getControl(g, "loop_start_position")},
                    {"loop_end_position", getControl(g, "loop_end_position")},
                    {"track_color", getControl(g, "track_color")},
                    {"eq_hi", getControl(QString("[EqualizerRack1_%1_Effect1]").arg(g), "parameter3")},
                    {"eq_mid", getControl(QString("[EqualizerRack1_%1_Effect1]").arg(g), "parameter2")},
                    {"eq_lo", getControl(QString("[EqualizerRack1_%1_Effect1]").arg(g), "parameter1")},
            };
        };
        QJsonObject status{
                {"engine", "running"},
                {"crossfader", getControl("[Master]", "crossfader")},
                {"master_volume", getControl("[Master]", "volume")},
                {"headphone_volume", getControl("[Master]", "headVolume")},
                {"deck1", deck(1)},
                {"deck2", deck(2)},
        };
        res.set_content(toJson(status), "application/json");
    });

    // Detailed deck info — transport + analysis state
    svr.Get("/api/deck/(\\d+)", [this](const httplib::Request& req, httplib::Response& res) {
        int d = std::stoi(req.matches[1]);
        QString g = deckGroup(d);
        double dur = getControl(g, "duration");
        double pos = getControl(g, "playposition");

        QJsonObject info{
                {"deck", d},
                {"playing", getControl(g, "play") > 0.5},
                {"track_loaded", getControl(g, "track_loaded") > 0.5},
                {"duration", dur},
                {"position", pos},
                {"position_seconds", pos * dur},
                {"remaining_seconds", dur - (pos * dur)},
                {"bpm", getControl(g, "bpm")},
                {"visual_bpm", getControl(g, "visual_bpm")},
                {"file_bpm", getControl(g, "file_bpm")},
                {"key", getControl(g, "key")},
                {"visual_key", getControl(g, "visual_key")},
                {"volume", getControl(g, "volume")},
                {"rate", getControl(g, "rate")},
                {"sync_enabled", getControl(g, "sync_enabled") > 0.5},
                {"beat_active", getControl(g, "beat_active") > 0.5},
                {"loop_enabled", getControl(g, "loop_enabled") > 0.5},
                {"pfl", getControl(g, "pfl") > 0.5},
                {"orientation", getControl(g, "orientation")},
                {"waveform_zoom", getControl(g, "waveform_zoom")},
        };
        res.set_content(toJson(info), "application/json");
    });

    // ════════════════════════════════════════════════════════════════
    // TRACK INFO — metadata, beats, waveform, cues from Track object
    // ════════════════════════════════════════════════════════════════

    svr.Get("/api/deck/(\\d+)/track_info",
            [this](const httplib::Request& req, httplib::Response& res) {
                int d = std::stoi(req.matches[1]);
                QString g = deckGroup(d);

                if (getControl(g, "track_loaded") < 0.5) {
                    res.set_content(
                            toJson(jsonError("No track loaded on deck " +
                                    QString::number(d))),
                            "application/json");
                    return;
                }

                QJsonObject info = getTrackInfo(d);
                if (info.isEmpty()) {
                    res.set_content(
                            toJson(jsonError("Could not read track info from deck " +
                                    QString::number(d))),
                            "application/json");
                    return;
                }

                info["deck"] = d;
                res.set_content(toJson(info), "application/json");
            });

    // ════════════════════════════════════════════════════════════════
    // GENERIC CONTROL — read/write any Mixxx control by group+key
    // ════════════════════════════════════════════════════════════════

    svr.Get("/api/control", [this](const httplib::Request& req, httplib::Response& res) {
        auto group = QString::fromStdString(req.get_param_value("group"));
        auto key = QString::fromStdString(req.get_param_value("key"));
        res.set_content(
                toJson(QJsonObject{
                        {"group", group},
                        {"key", key},
                        {"value", getControl(group, key)},
                }),
                "application/json");
    });

    svr.Post("/api/control", [this](const httplib::Request& req, httplib::Response& res) {
        auto obj = parseBody(req.body);
        QString group = obj["group"].toString();
        QString key = obj["key"].toString();
        if (group.isEmpty() || key.isEmpty()) {
            res.set_content(
                    toJson(jsonError("group and key are required")),
                    "application/json");
            return;
        }
        setControl(group, key, obj["value"].toDouble());
        res.set_content(toJson(jsonOk()), "application/json");
    });

    // ════════════════════════════════════════════════════════════════
    // DECK CONTROLS
    // ════════════════════════════════════════════════════════════════

    svr.Post("/api/load", [this](const httplib::Request& req, httplib::Response& res) {
        auto obj = parseBody(req.body);
        int deck = obj["deck"].toInt(1);
        QString path = obj["track"].toString();

        if (m_pPlayerManager && !path.isEmpty()) {
            QMetaObject::invokeMethod(
                    m_pPlayerManager,
                    [pm = m_pPlayerManager, path, deck]() {
                        pm->slotLoadToDeck(path, deck);
                    },
                    Qt::QueuedConnection);
            qInfo() << kApiTag << "Loading" << path << "to deck" << deck;
            QThread::msleep(500);
            res.set_content(
                    toJson(jsonOk({{"deck", deck}, {"track", path}})),
                    "application/json");
        } else {
            res.set_content(
                    toJson(jsonError("PlayerManager not available or empty path")),
                    "application/json");
        }
    });

    svr.Post("/api/play", [this](const httplib::Request& req, httplib::Response& res) {
        int deck = parseBody(req.body)["deck"].toInt(1);
        setControl(deckGroup(deck), "play", 1.0);
        res.set_content(toJson(jsonOk({{"deck", deck}})), "application/json");
    });

    svr.Post("/api/pause", [this](const httplib::Request& req, httplib::Response& res) {
        int deck = parseBody(req.body)["deck"].toInt(1);
        setControl(deckGroup(deck), "play", 0.0);
        res.set_content(toJson(jsonOk({{"deck", deck}})), "application/json");
    });

    svr.Post("/api/stop", [this](const httplib::Request& req, httplib::Response& res) {
        int deck = parseBody(req.body)["deck"].toInt(1);
        setControl(deckGroup(deck), "play", 0.0);
        setControl(deckGroup(deck), "playposition", 0.0);
        res.set_content(toJson(jsonOk({{"deck", deck}})), "application/json");
    });

    svr.Post("/api/eject", [this](const httplib::Request& req, httplib::Response& res) {
        int deck = parseBody(req.body)["deck"].toInt(1);
        setControl(deckGroup(deck), "eject", 1.0);
        res.set_content(toJson(jsonOk({{"deck", deck}})), "application/json");
    });

    svr.Post("/api/seek", [this](const httplib::Request& req, httplib::Response& res) {
        auto obj = parseBody(req.body);
        int deck = obj["deck"].toInt(1);
        double position = obj["position"].toDouble(0.0); // 0.0 to 1.0
        setControl(deckGroup(deck), "playposition", position);
        res.set_content(
                toJson(jsonOk({{"deck", deck}, {"position", position}})),
                "application/json");
    });

    svr.Post("/api/volume", [this](const httplib::Request& req, httplib::Response& res) {
        auto obj = parseBody(req.body);
        int deck = obj["deck"].toInt(1);
        double level = obj["level"].toDouble(1.0);
        setControl(deckGroup(deck), "volume", level);
        res.set_content(toJson(jsonOk()), "application/json");
    });

    // ════════════════════════════════════════════════════════════════
    // SYNC & BPM
    // ════════════════════════════════════════════════════════════════

    svr.Post("/api/sync", [this](const httplib::Request& req, httplib::Response& res) {
        int deck = parseBody(req.body)["deck"].toInt(1);
        setControl(deckGroup(deck), "sync_enabled", 1.0);
        res.set_content(toJson(jsonOk({{"deck", deck}})), "application/json");
    });

    svr.Post("/api/sync_off", [this](const httplib::Request& req, httplib::Response& res) {
        int deck = parseBody(req.body)["deck"].toInt(1);
        setControl(deckGroup(deck), "sync_enabled", 0.0);
        res.set_content(toJson(jsonOk({{"deck", deck}})), "application/json");
    });

    svr.Post("/api/rate", [this](const httplib::Request& req, httplib::Response& res) {
        auto obj = parseBody(req.body);
        int deck = obj["deck"].toInt(1);
        double rate = obj["rate"].toDouble(0.0); // -1.0 to 1.0
        setControl(deckGroup(deck), "rate", rate);
        res.set_content(toJson(jsonOk()), "application/json");
    });

    // ════════════════════════════════════════════════════════════════
    // MIXER
    // ════════════════════════════════════════════════════════════════

    svr.Post("/api/crossfade", [this](const httplib::Request& req, httplib::Response& res) {
        double pos = parseBody(req.body)["position"].toDouble(0.5);
        // Map 0-1 (API range) to -1 to +1 (Mixxx internal range)
        setControl("[Master]", "crossfader", (pos * 2.0) - 1.0);
        res.set_content(toJson(jsonOk({{"position", pos}})), "application/json");
    });

    svr.Post("/api/master", [this](const httplib::Request& req, httplib::Response& res) {
        double level = parseBody(req.body)["level"].toDouble(1.0);
        setControl("[Master]", "volume", level);
        res.set_content(toJson(jsonOk()), "application/json");
    });

    // ════════════════════════════════════════════════════════════════
    // EQ (per deck)
    // ════════════════════════════════════════════════════════════════

    svr.Post("/api/eq", [this](const httplib::Request& req, httplib::Response& res) {
        auto obj = parseBody(req.body);
        int deck = obj["deck"].toInt(1);
        QString eqGroup = QString("[EqualizerRack1_[Channel%1]_Effect1]").arg(deck);
        if (obj.contains("hi")) {
            setControl(eqGroup, "parameter3", obj["hi"].toDouble());
        }
        if (obj.contains("mid")) {
            setControl(eqGroup, "parameter2", obj["mid"].toDouble());
        }
        if (obj.contains("lo")) {
            setControl(eqGroup, "parameter1", obj["lo"].toDouble());
        }
        res.set_content(toJson(jsonOk()), "application/json");
    });

    // ════════════════════════════════════════════════════════════════
    // LOOPS & CUES
    // ════════════════════════════════════════════════════════════════

    svr.Post("/api/loop", [this](const httplib::Request& req, httplib::Response& res) {
        auto obj = parseBody(req.body);
        int deck = obj["deck"].toInt(1);
        QString g = deckGroup(deck);
        QString action = obj["action"].toString("toggle");

        if (action == "in") {
            setControl(g, "loop_in", 1.0);
        } else if (action == "out") {
            setControl(g, "loop_out", 1.0);
        } else if (action == "toggle") {
            setControl(g, "reloop_toggle", 1.0);
        } else if (action == "halve") {
            setControl(g, "loop_halve", 1.0);
        } else if (action == "double") {
            setControl(g, "loop_double", 1.0);
        }

        res.set_content(toJson(jsonOk({{"action", action}})), "application/json");
    });

    svr.Post("/api/hotcue", [this](const httplib::Request& req, httplib::Response& res) {
        auto obj = parseBody(req.body);
        int deck = obj["deck"].toInt(1);
        int num = obj["num"].toInt(1);                       // 1-8
        QString action = obj["action"].toString("activate"); // activate, set, clear
        QString g = deckGroup(deck);
        QString key = QString("hotcue_%1_%2").arg(num).arg(action);
        setControl(g, key, 1.0);
        res.set_content(toJson(jsonOk()), "application/json");
    });

    // ════════════════════════════════════════════════════════════════
    // LIBRARY NAVIGATION
    // ════════════════════════════════════════════════════════════════

    svr.Post("/api/library", [this](const httplib::Request& req, httplib::Response& res) {
        auto obj = parseBody(req.body);
        QString action = obj["action"].toString();
        if (action == "down") {
            setControl("[Library]", "MoveDown", 1.0);
        } else if (action == "up") {
            setControl("[Library]", "MoveUp", 1.0);
        } else if (action == "left") {
            setControl("[Library]", "MoveLeft", 1.0);
        } else if (action == "right") {
            setControl("[Library]", "MoveRight", 1.0);
        } else if (action == "focus_forward") {
            setControl("[Library]", "MoveFocusForward", 1.0);
        } else if (action == "focus_back") {
            setControl("[Library]", "MoveFocusBackward", 1.0);
        } else if (action == "select") {
            setControl("[Library]", "GoToItem", 1.0);
        }
        res.set_content(toJson(jsonOk({{"action", action}})), "application/json");
    });

    // ════════════════════════════════════════════════════════════════
    // EFFECTS
    // ════════════════════════════════════════════════════════════════

    svr.Post("/api/effect", [this](const httplib::Request& req, httplib::Response& res) {
        auto obj = parseBody(req.body);
        int deck = obj["deck"].toInt(1);
        int unit = obj["unit"].toInt(1); // Effect unit 1-4
        QString action = obj["action"].toString("toggle");
        QString g = QString("[EffectRack1_EffectUnit%1]").arg(unit);

        if (action == "toggle" || action == "on") {
            setControl(g, QString("group_[Channel%1]_enable").arg(deck), 1.0);
        } else if (action == "off") {
            setControl(g, QString("group_[Channel%1]_enable").arg(deck), 0.0);
        }

        if (obj.contains("mix")) {
            setControl(g, "mix", obj["mix"].toDouble());
        }
        if (obj.contains("super")) {
            setControl(g, "super1", obj["super"].toDouble());
        }

        res.set_content(toJson(jsonOk()), "application/json");
    });

    // ════════════════════════════════════════════════════════════════
    // AUTO DJ
    // ════════════════════════════════════════════════════════════════

    svr.Post("/api/autodj", [this](const httplib::Request& req, httplib::Response& res) {
        auto obj = parseBody(req.body);
        QString action = obj["action"].toString("toggle");
        if (action == "enable" || action == "toggle") {
            setControl("[AutoDJ]", "enabled", 1.0);
        } else if (action == "disable") {
            setControl("[AutoDJ]", "enabled", 0.0);
        } else if (action == "skip") {
            setControl("[AutoDJ]", "skip_next", 1.0);
        } else if (action == "fade_now") {
            setControl("[AutoDJ]", "fade_now", 1.0);
        }
        res.set_content(toJson(jsonOk({{"action", action}})), "application/json");
    });

    // ════════════════════════════════════════════════════════════════
    // SMOOTH TRANSITION (server-side crossfade with S-curve easing)
    // ════════════════════════════════════════════════════════════════

    svr.Post("/api/transition", [this](const httplib::Request& req, httplib::Response& res) {
        auto obj = parseBody(req.body);
        int toDeck = obj["deck"].toInt(2);
        double duration = obj["duration"].toDouble(60.0);

        // Capture m_running by reference to allow clean shutdown
        std::thread([this, toDeck, duration]() {
            int fps = 20;
            int steps = static_cast<int>(duration * fps);
            double cfStart = getControl("[Master]", "crossfader");
            double cfEnd = (toDeck == 2) ? 1.0 : -1.0;

            for (int i = 0; i <= steps && m_running; i++) {
                double r = static_cast<double>(i) / steps;
                double ease = r * r * (3.0 - 2.0 * r); // smoothstep
                double cf = cfStart + (cfEnd - cfStart) * ease;
                setControl("[Master]", "crossfader", cf);
                std::this_thread::sleep_for(std::chrono::milliseconds(1000 / fps));
            }
            if (m_running) {
                setControl("[Master]", "crossfader", cfEnd);
                qInfo() << kApiTag << "Transition to deck" << toDeck
                        << "complete (" << duration << "s)";
            }
        }).detach();

        res.set_content(
                toJson(jsonOk({
                        {"deck", toDeck},
                        {"duration", duration},
                        {"msg", "Transition started"},
                })),
                "application/json");
    });

    // ════════════════════════════════════════════════════════════════
    // QUICK EFFECT / FILTER
    // ════════════════════════════════════════════════════════════════

    svr.Post("/api/filter", [this](const httplib::Request& req, httplib::Response& res) {
        auto obj = parseBody(req.body);
        int deck = obj["deck"].toInt(1);
        double value = obj["value"].toDouble(0.5); // 0.0 to 1.0
        setControl(
                QString("[QuickEffectRack1_[Channel%1]]").arg(deck),
                "super1",
                value);
        res.set_content(toJson(jsonOk()), "application/json");
    });

    // ════════════════════════════════════════════════════════════════
    // HEADPHONE CUE (PFL)
    // ════════════════════════════════════════════════════════════════

    svr.Post("/api/pfl", [this](const httplib::Request& req, httplib::Response& res) {
        auto obj = parseBody(req.body);
        int deck = obj["deck"].toInt(1);
        double val = obj["enabled"].toBool(true) ? 1.0 : 0.0;
        setControl(deckGroup(deck), "pfl", val);
        res.set_content(toJson(jsonOk()), "application/json");
    });

    // ════════════════════════════════════════════════════════════════
    // LIST TRACKS — files in ~/Music directory
    // ════════════════════════════════════════════════════════════════

    svr.Get("/api/tracks", [](const httplib::Request&, httplib::Response& res) {
        QJsonArray tracks;
        QDir musicDir(QDir::homePath() + "/Music");
        QStringList filters;
        filters << "*.mp3" << "*.wav" << "*.flac" << "*.m4a" << "*.ogg";
        auto files = musicDir.entryInfoList(filters, QDir::Files, QDir::Name);
        for (const auto& fi : files) {
            tracks.append(QJsonObject{
                    {"name", fi.baseName()},
                    {"filename", fi.fileName()},
                    {"path", fi.absoluteFilePath()},
                    {"size_mb", static_cast<double>(fi.size()) / (1024.0 * 1024.0)},
            });
        }
        QJsonObject result{{"tracks", tracks}, {"count", tracks.size()}};
        res.set_content(
                QJsonDocument(result).toJson(QJsonDocument::Compact).toStdString(),
                "application/json");
    });

    // ════════════════════════════════════════════════════════════════

    qInfo() << kApiTag << "HTTP API listening on"
            << m_bindAddress << "port" << m_port;
    svr.listen(m_bindAddress.toStdString(), m_port);
    qInfo() << kApiTag << "HTTP API stopped";
}

#include "moc_apiserver.cpp"
