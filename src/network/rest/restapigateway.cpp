#include "network/rest/restapigateway.h"

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QDateTime>
#include <QFile>
#include <QJsonDocument>
#include <QJsonValue>
#include <QMutexLocker>
#include <QThread>
#ifdef Q_OS_LINUX
#include <unistd.h>
#endif

#include "moc_restapigateway.cpp"
#include "control/controlproxy.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackschema.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "mixer/basetrackplayer.h"
#include "mixer/playermanager.h"
#include "broadcast/defs_broadcast.h"
#include "util/versionstore.h"
#include "track/track.h"
#include "track/trackid.h"
#include "util/logger.h"
#include "util/thread_affinity.h"

namespace {
constexpr auto kApplicationJsonMimeType = "application/json";
}

namespace mixxx::network::rest {

namespace {
[[maybe_unused]] const Logger kLogger("mixxx::network::rest::RestApiGateway");
constexpr qint64 kIdempotencyCacheTtlMs = 10 * 1000;
constexpr int kMaxIdempotencyCacheEntries = 512;
constexpr int kMaxIdempotencyKeyLength = 128;
} // namespace

RestApiGateway::RestApiGateway(
        PlayerManager* playerManager,
        TrackCollectionManager* trackCollectionManager,
        const UserSettingsPointer& settings,
        QObject* parent)
        : RestApiProvider(parent),
          m_playerManager(playerManager),
          m_trackCollectionManager(trackCollectionManager),
          m_settings(settings) {
    DEBUG_ASSERT(m_playerManager);
    DEBUG_ASSERT(m_trackCollectionManager);
    m_uptime.start();
}

QHttpServerResponse RestApiGateway::errorResponse(
        QHttpServerResponse::StatusCode code,
        const QString& message) const {
    return QHttpServerResponse(
            kApplicationJsonMimeType,
            QJsonDocument(QJsonObject{
                    {"error", message},
            })
                    .toJson(QJsonDocument::Compact),
            code);
}

QHttpServerResponse RestApiGateway::successResponse(
        const QJsonObject& payload,
        QHttpServerResponse::StatusCode statusCode) const {
    return QHttpServerResponse(
            kApplicationJsonMimeType,
            QJsonDocument(payload).toJson(QJsonDocument::Compact),
            statusCode);
}

QHttpServerResponse RestApiGateway::health() const {
    const QJsonObject readiness = readinessPayload();
    return successResponse(QJsonObject{
            {"status", "ok"},
            {"ready", readiness.value("ready")},
            {"issues", readiness.value("issues")},
            {"system", systemHealth()},
            {"uptime_ms", static_cast<qint64>(m_uptime.elapsed())},
            {"timestamp", QDateTime::currentDateTimeUtc().toString(Qt::ISODate)},
    });
}

QHttpServerResponse RestApiGateway::ready() const {
    return successResponse(readinessPayload());
}

QHttpServerResponse RestApiGateway::withIdempotencyCache(
        const QString& token,
        const QString& idempotencyKey,
        const QString& endpoint,
        const std::function<QHttpServerResponse()>& handler) const {
    if (idempotencyKey.isEmpty()) {
        return handler();
    }
    if (idempotencyKey.size() > kMaxIdempotencyKeyLength) {
        return errorResponse(
                QHttpServerResponse::StatusCode::BadRequest,
                tr("Idempotency-Key exceeds maximum length of %1 characters")
                        .arg(kMaxIdempotencyKeyLength));
    }

    const QDateTime nowUtc = QDateTime::currentDateTimeUtc();
    const QString cacheKey = QStringLiteral("%1\n%2\n%3")
                                     .arg(token, idempotencyKey, endpoint);
    const auto responseFromCache = [&](const IdempotencyEntry& entry) {
        QHttpServerResponse response(entry.mimeType, entry.body, entry.statusCode);
        if (!entry.headers.isEmpty()) {
            response.setHeaders(entry.headers);
        }
        return response;
    };
    {
        QMutexLocker locker(&m_idempotencyMutex);
        for (auto it = m_idempotencyCache.begin(); it != m_idempotencyCache.end();) {
            if (it->createdUtc.msecsTo(nowUtc) > kIdempotencyCacheTtlMs) {
                it = m_idempotencyCache.erase(it);
            } else {
                ++it;
            }
        }
        for (auto it = m_idempotencyOrder.begin(); it != m_idempotencyOrder.end();) {
            if (!m_idempotencyCache.contains(*it)) {
                it = m_idempotencyOrder.erase(it);
            } else {
                ++it;
            }
        }
        const auto cached = m_idempotencyCache.constFind(cacheKey);
        if (cached != m_idempotencyCache.constEnd()) {
            return responseFromCache(*cached);
        }
    }

    QHttpServerResponse response = handler();
    const IdempotencyEntry entry{
            nowUtc,
            response.statusCode(),
            response.data(),
            response.mimeType(),
            response.headers(),
    };
    {
        QMutexLocker locker(&m_idempotencyMutex);
        const auto cached = m_idempotencyCache.constFind(cacheKey);
        if (cached != m_idempotencyCache.constEnd()) {
            return responseFromCache(*cached);
        }
        m_idempotencyCache.insert(cacheKey, entry);
        m_idempotencyOrder.append(cacheKey);
        while (m_idempotencyCache.size() > kMaxIdempotencyCacheEntries &&
                !m_idempotencyOrder.isEmpty()) {
            const QString oldestKey = m_idempotencyOrder.takeFirst();
            m_idempotencyCache.remove(oldestKey);
        }
    }
    return response;
}

QJsonObject RestApiGateway::deckSummary(int deckIndex) const {
    QJsonObject deck;
    const QString group = PlayerManager::groupForDeck(deckIndex);
    deck.insert("group", group);
    ControlProxy playControl(group, QStringLiteral("play"), nullptr);
    ControlProxy trackLoaded(group, QStringLiteral("track_loaded"), nullptr);
    ControlProxy playPosition(group, QStringLiteral("playposition"), nullptr);
    ControlProxy rate(group, QStringLiteral("rate"), nullptr);
    ControlProxy gain(group, QStringLiteral("gain"), nullptr);
    ControlProxy volume(group, QStringLiteral("volume"), nullptr);
    ControlProxy syncEnabled(group, QStringLiteral("sync_enabled"), nullptr);
    ControlProxy keylock(group, QStringLiteral("keylock"), nullptr);
    ControlProxy loopEnabled(group, QStringLiteral("loop_enabled"), nullptr);
    deck.insert("playing", playControl.toBool());
    deck.insert("track_loaded", trackLoaded.toBool());
    deck.insert("position", playPosition.get());
    deck.insert("rate", rate.get());
    deck.insert("gain", gain.get());
    deck.insert("volume", volume.get());
    deck.insert("sync", syncEnabled.toBool());
    deck.insert("keylock", keylock.toBool());
    deck.insert("loop_enabled", loopEnabled.toBool());

    BaseTrackPlayer* const player = m_playerManager->getPlayer(group);
    if (player != nullptr) {
        if (const TrackPointer track = player->getLoadedTrack()) {
            deck.insert("track", trackPayload(track));
            deck.insert("duration", track->getDuration());
        }
    }

    return deck;
}

QJsonArray RestApiGateway::deckStatuses() const {
    QJsonArray decks;
    const auto totalDecks = m_playerManager->numberOfDecks();
    for (unsigned int i = 0; i < totalDecks; ++i) {
        decks.append(deckSummary(static_cast<int>(i)));
    }
    return decks;
}

QHttpServerResponse RestApiGateway::status() const {
    return successResponse(statusPayload());
}

QJsonObject RestApiGateway::statusPayload() const {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    const QJsonObject readiness = readinessPayload();
    return QJsonObject{
            {"app", appInfo()},
            {"ready", readiness},
            {"system", systemHealth()},
            {"decks", deckStatuses()},
            {"mixer", mixerState()},
            {"broadcast", broadcastState()},
            {"recording", recordingState()},
            {"autodj", autoDjOverview()},
            {"uptime_ms", static_cast<qint64>(m_uptime.elapsed())},
            {"timestamp", QDateTime::currentDateTimeUtc().toString(Qt::ISODate)},
    };
}

QHttpServerResponse RestApiGateway::deck(int deckNumber) const {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    if (deckNumber <= 0) {
        return errorResponse(
                QHttpServerResponse::StatusCode::BadRequest,
                tr("Deck number must be positive"));
    }

    const int deckIndex = deckNumber - 1;
    const auto totalDecks = m_playerManager->numberOfDecks();
    if (deckIndex < 0 || deckIndex >= static_cast<int>(totalDecks)) {
        return errorResponse(
                QHttpServerResponse::StatusCode::NotFound,
                tr("Deck %1 does not exist").arg(deckNumber));
    }

    return successResponse(QJsonObject{{"deck", deckSummary(deckIndex)}});
}

QHttpServerResponse RestApiGateway::decks() const {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    return successResponse(QJsonObject{{"decks", deckStatuses()}});
}

QHttpServerResponse RestApiGateway::control(const QJsonObject& body) const {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    struct ControlResult {
        QHttpServerResponse::StatusCode status;
        QJsonObject payload;
    };

    auto errorResult = [&](QHttpServerResponse::StatusCode code, const QString& message) {
        return ControlResult{code, QJsonObject{{"error", message}}};
    };

    auto successResult = [&](const QJsonObject& payload,
                             QHttpServerResponse::StatusCode statusCode =
                                     QHttpServerResponse::StatusCode::Ok) {
        return ControlResult{statusCode, payload};
    };

    auto applyControl = [&](const QJsonObject& commandBody) {
        const QString command = commandBody.value("command").toString();
        QString group = commandBody.value("group").toString();
        if (group.isEmpty()) {
            const auto deckValue = commandBody.value("deck");
            if (deckValue.isDouble()) {
                const int deckIndex = deckValue.toInt();
                if (deckIndex > 0) {
                    group = PlayerManager::groupForDeck(deckIndex - 1);
                }
            }
        }
        if (group.isEmpty()) {
            return errorResult(
                    QHttpServerResponse::StatusCode::BadRequest,
                    tr("Missing control group"));
        }

        const QString key = commandBody.value("key").toString();
        if (command.isEmpty() && key.isEmpty()) {
            return errorResult(
                    QHttpServerResponse::StatusCode::BadRequest,
                    tr("Missing command or key"));
        }

        auto applyValue = [&](ControlProxy& control, std::optional<double> value = std::nullopt) {
            control.set(value.value_or(1.0));
            return control.get();
        };

        auto toggleControl = [&](ControlProxy& control) {
            const double nextValue = control.toBool() ? 0.0 : 1.0;
            control.set(nextValue);
            return nextValue;
        };

        auto finalizeResponse = [&](const QString& responseKey,
                                    const QString& responseGroup,
                                    double responseValue) {
            return successResult(QJsonObject{
                    {"group", responseGroup},
                    {"key", responseKey},
                    {"value", responseValue},
            });
        };

        if (!command.isEmpty()) {
            if (command == QStringLiteral("play") || command == QStringLiteral("pause") ||
                    command == QStringLiteral("toggle")) {
                ControlProxy control(group, QStringLiteral("play"), nullptr);
                if (!control.valid()) {
                    return errorResult(
                            QHttpServerResponse::StatusCode::BadRequest,
                            tr("Unknown control %1 play").arg(group));
                }
                if (command == QStringLiteral("toggle")) {
                    return finalizeResponse("play", group, toggleControl(control));
                }
                const double value = command == QStringLiteral("play") ? 1.0 : 0.0;
                return finalizeResponse("play", group, applyValue(control, value));
            }
            if (command == QStringLiteral("seek")) {
                const auto positionValue = commandBody.value("position");
                if (!positionValue.isDouble()) {
                    return errorResult(
                            QHttpServerResponse::StatusCode::BadRequest,
                            tr("Seek position must be numeric"));
                }
                ControlProxy control(group, QStringLiteral("playposition"), nullptr);
                if (!control.valid()) {
                    return errorResult(
                            QHttpServerResponse::StatusCode::BadRequest,
                            tr("Unknown control %1 playposition").arg(group));
                }
                return finalizeResponse(
                        "playposition",
                        group,
                        applyValue(control, positionValue.toDouble()));
            }
            if (command == QStringLiteral("gain")) {
                const auto gainValue = commandBody.value("value");
                if (!gainValue.isDouble()) {
                    return errorResult(
                            QHttpServerResponse::StatusCode::BadRequest,
                            tr("Gain must be numeric"));
                }
                ControlProxy control(group, QStringLiteral("gain"), nullptr);
                if (!control.valid()) {
                    return errorResult(
                            QHttpServerResponse::StatusCode::BadRequest,
                            tr("Unknown control %1 gain").arg(group));
                }
                return finalizeResponse("gain", group, applyValue(control, gainValue.toDouble()));
            }

            return errorResult(
                    QHttpServerResponse::StatusCode::BadRequest,
                    tr("Unsupported control command: %1").arg(command));
        }

        ControlProxy control(group, key, nullptr);
        if (!control.valid()) {
            return errorResult(
                    QHttpServerResponse::StatusCode::BadRequest,
                    tr("Unknown control %1 %2").arg(group, key));
        }

        const auto valueVariant = commandBody.value("value");
        std::optional<double> controlValue;
        if (!valueVariant.isUndefined()) {
            if (!valueVariant.isDouble()) {
                return errorResult(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("Control value must be numeric"));
            }
            controlValue = valueVariant.toDouble();
        }

        const double resultValue = applyValue(control, controlValue);
        return finalizeResponse(key, group, resultValue);
    };

    if (body.contains("commands")) {
        const auto commandsValue = body.value("commands");
        if (!commandsValue.isArray()) {
            return errorResponse(
                    QHttpServerResponse::StatusCode::BadRequest,
                    tr("Commands must be an array"));
        }

        const QJsonArray commands = commandsValue.toArray();
        if (commands.isEmpty()) {
            return errorResponse(
                    QHttpServerResponse::StatusCode::BadRequest,
                    tr("No commands provided"));
        }

        QJsonArray results;
        bool sawSuccess = false;
        bool sawError = false;
        QHttpServerResponse::StatusCode firstErrorStatus = QHttpServerResponse::StatusCode::Ok;

        for (const QJsonValue& commandValue : commands) {
            if (!commandValue.isObject()) {
                auto invalidResult = errorResult(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("Command entry must be an object"));
                QJsonObject entry = invalidResult.payload;
                entry.insert("status", static_cast<int>(invalidResult.status));
                results.append(entry);
                sawError = true;
                if (firstErrorStatus == QHttpServerResponse::StatusCode::Ok) {
                    firstErrorStatus = invalidResult.status;
                } else if (firstErrorStatus != invalidResult.status) {
                    firstErrorStatus = QHttpServerResponse::StatusCode::MultiStatus;
                }
                continue;
            }

            const auto result = applyControl(commandValue.toObject());
            QJsonObject entry = result.payload;
            entry.insert("status", static_cast<int>(result.status));
            results.append(entry);

            if (result.status == QHttpServerResponse::StatusCode::Ok) {
                sawSuccess = true;
            } else {
                sawError = true;
                if (firstErrorStatus == QHttpServerResponse::StatusCode::Ok) {
                    firstErrorStatus = result.status;
                } else if (firstErrorStatus != result.status) {
                    firstErrorStatus = QHttpServerResponse::StatusCode::MultiStatus;
                }
            }
        }

        QHttpServerResponse::StatusCode aggregateStatus = QHttpServerResponse::StatusCode::Ok;
        if (sawSuccess && sawError) {
            aggregateStatus = QHttpServerResponse::StatusCode::MultiStatus;
        } else if (sawError) {
            aggregateStatus = firstErrorStatus;
            if (aggregateStatus == QHttpServerResponse::StatusCode::Ok) {
                aggregateStatus = QHttpServerResponse::StatusCode::MultiStatus;
            }
        }

        return successResponse(QJsonObject{{"results", results}}, aggregateStatus);
    }

    const auto singleResult = applyControl(body);
    if (singleResult.status == QHttpServerResponse::StatusCode::Ok) {
        return successResponse(singleResult.payload);
    }
    return errorResponse(singleResult.status, singleResult.payload.value("error").toString());
}

QHttpServerResponse RestApiGateway::autoDj(const QJsonObject& body) const {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    const QString action = body.value("action").toString();
    if (action.isEmpty()) {
        return errorResponse(
                QHttpServerResponse::StatusCode::BadRequest,
                tr("Missing AutoDJ action"));
    }

    const ConfigKey enabledKey(QStringLiteral("[AutoDJ]"), QStringLiteral("enabled"));
    const ConfigKey skipKey(QStringLiteral("[AutoDJ]"), QStringLiteral("skip_next"));
    const ConfigKey fadeKey(QStringLiteral("[AutoDJ]"), QStringLiteral("fade_now"));
    const ConfigKey shuffleKey(QStringLiteral("[AutoDJ]"), QStringLiteral("shuffle_playlist"));
    const ConfigKey addRandomKey(QStringLiteral("[AutoDJ]"), QStringLiteral("add_random_track"));

    ControlProxy enabled(enabledKey, nullptr);
    ControlProxy skip(skipKey, nullptr);
    ControlProxy fade(fadeKey, nullptr);
    ControlProxy shuffle(shuffleKey, nullptr);
    ControlProxy addRandom(addRandomKey, nullptr);

    auto resultWithEnabled = [&](const QString& performedAction) {
        return successResponse(QJsonObject{
                {"action", performedAction},
                {"enabled", enabled.toBool()},
        });
    };

    if (action == QStringLiteral("enable")) {
        enabled.set(1.0);
    } else if (action == QStringLiteral("disable")) {
        enabled.set(0.0);
    } else if (action == QStringLiteral("skip")) {
        skip.set(1.0);
    } else if (action == QStringLiteral("fade")) {
        fade.set(1.0);
    } else if (action == QStringLiteral("shuffle")) {
        shuffle.set(1.0);
    } else if (action == QStringLiteral("add_random")) {
        addRandom.set(1.0);
    } else if (action == QStringLiteral("clear") || action == QStringLiteral("add") ||
            action == QStringLiteral("move")) {
        return withPlaylistDao([&](PlaylistDAO& playlistDao) {
            const int autoDjPlaylistId = ensureAutoDjPlaylistId(playlistDao);
            if (autoDjPlaylistId == kInvalidPlaylistId) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::ServiceUnavailable,
                        tr("Unable to locate AutoDJ playlist"));
            }

            if (action == QStringLiteral("clear")) {
                playlistDao.clearAutoDJQueue();
                return resultWithEnabled(action);
            }

            if (action == QStringLiteral("add")) {
                const auto tracksValue = body.value("track_ids");
                if (!tracksValue.isArray()) {
                    return errorResponse(
                            QHttpServerResponse::StatusCode::BadRequest,
                            tr("track_ids must be an array"));
                }
                const auto tracksArray = tracksValue.toArray();
                for (int index = 0; index < tracksArray.size(); ++index) {
                    const auto& value = tracksArray.at(index);
                    if (!value.isDouble() && !value.isString()) {
                        return errorResponse(
                                QHttpServerResponse::StatusCode::BadRequest,
                                tr("track_ids must contain numbers or strings"));
                    }
                }
                QStringList parseErrors;
                const QList<TrackId> trackIds = parseTrackIds(tracksArray, &parseErrors);
                if (!parseErrors.isEmpty()) {
                    return errorResponse(
                            QHttpServerResponse::StatusCode::BadRequest,
                            parseErrors.join(QLatin1String("; ")));
                }
                if (trackIds.isEmpty()) {
                    return errorResponse(
                            QHttpServerResponse::StatusCode::BadRequest,
                            tr("No track ids provided"));
                }

                const QString position =
                        body.value("position").toString(QStringLiteral("bottom")).toLower();
                PlaylistDAO::AutoDJSendLoc sendLoc = PlaylistDAO::AutoDJSendLoc::BOTTOM;
                if (position == QStringLiteral("top")) {
                    sendLoc = PlaylistDAO::AutoDJSendLoc::TOP;
                } else if (position == QStringLiteral("replace")) {
                    sendLoc = PlaylistDAO::AutoDJSendLoc::REPLACE;
                }

                playlistDao.addTracksToAutoDJQueue(trackIds, sendLoc);
                return resultWithEnabled(action);
            }

            const auto fromValue = body.value("from");
            if (!fromValue.isDouble()) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("Move command is missing from position"));
            }
            const auto toValue = body.value("to");
            if (!toValue.isDouble()) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("Move command is missing destination position"));
            }
            const int from = fromValue.toInt();
            const int to = toValue.toInt();

            const int queueSize = playlistDao.getAutoDJTrackIds().size();
            if (queueSize == 0) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("AutoDJ queue is empty"));
            }
            const int maxPosition = queueSize - 1;
            if (from < 0 || from > maxPosition) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("AutoDJ from position %1 is out of range (valid 0-%2)")
                                .arg(from)
                                .arg(maxPosition));
            }
            if (to < 0 || to > maxPosition) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("AutoDJ destination position %1 is out of range (valid 0-%2)")
                                .arg(to)
                                .arg(maxPosition));
            }

            playlistDao.moveTrack(autoDjPlaylistId, from, to);
            return resultWithEnabled(action);
        });
    } else {
        return errorResponse(
                QHttpServerResponse::StatusCode::BadRequest,
                tr("Unsupported AutoDJ action: %1").arg(action));
    }

    return resultWithEnabled(action);
}

QHttpServerResponse RestApiGateway::playlists(const std::optional<int>& playlistId) const {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    return withPlaylistDao([&](PlaylistDAO& playlistDao) {
        if (playlistId.has_value()) {
            if (playlistId.value() < 0 || playlistDao.isHidden(playlistId.value())) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::NotFound,
                        tr("Playlist %1 does not exist").arg(playlistId.value()));
            }

            QJsonArray tracks;
            const QList<TrackId> trackIds =
                    playlistDao.getTrackIdsInPlaylistOrder(playlistId.value());
            for (const TrackId& trackId : trackIds) {
                QJsonObject entry;
                entry.insert("id", trackId.toString());
                const TrackPointer track = m_trackCollectionManager->getTrackById(trackId);
                if (track) {
                    entry.insert("track", trackPayload(track));
                    entry.insert("duration", track->getDuration());
                }
                tracks.append(entry);
            }
            return successResponse(QJsonObject{
                    {"playlist_id", playlistId.value()},
                    {"tracks", tracks},
            });
        }

        const auto playlists = playlistDao.getPlaylists(PlaylistDAO::PLHT_NOT_HIDDEN);
        QJsonArray payload;
        for (const auto& playlist : playlists) {
            QJsonObject entry;
            entry.insert("id", playlist.first);
            entry.insert("name", playlist.second);
            entry.insert("locked", playlistDao.isPlaylistLocked(playlist.first));
            entry.insert("track_count", playlistDao.tracksInPlaylist(playlist.first));
            payload.append(entry);
        }
        return successResponse(QJsonObject{
                {"active_playlist_id", m_activePlaylistId},
                {"playlists", payload},
        });
    });
}

QHttpServerResponse RestApiGateway::playlistCommand(const QJsonObject& body) const {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    const QString action = body.value("action").toString();
    if (action.isEmpty()) {
        return errorResponse(
                QHttpServerResponse::StatusCode::BadRequest,
                tr("Missing playlist action"));
    }

    return withPlaylistDao([&](PlaylistDAO& playlistDao) {
        auto validateTargetPlaylist = [&](int playlistId) -> std::optional<QString> {
            if (playlistId < 0 || playlistDao.isHidden(playlistId)) {
                return tr("Playlist %1 does not exist").arg(playlistId);
            }
            if (playlistDao.isPlaylistLocked(playlistId)) {
                return tr("Playlist %1 is locked").arg(playlistId);
            }
            return std::nullopt;
        };

        if (action == QStringLiteral("create")) {
            QString name = body.value("name").toString();
            if (name.isEmpty()) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("Playlist name is required"));
            }
            const int id = playlistDao.createUniquePlaylist(&name);
            if (id == kInvalidPlaylistId) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::InternalServerError,
                        tr("Failed to create playlist"));
            }
            return successResponse(
                    QJsonObject{{"id", id}, {"name", name}},
                    QHttpServerResponse::StatusCode::Created);
        }

        const int playlistId =
                body.value("playlist_id").toInt(m_activePlaylistId == kInvalidPlaylistId ? -1
                                                                                        : m_activePlaylistId);

        if (playlistId < 0) {
            return errorResponse(
                    QHttpServerResponse::StatusCode::BadRequest,
                    tr("Playlist id is required"));
        }

        if (const auto validationError = validateTargetPlaylist(playlistId)) {
            return errorResponse(QHttpServerResponse::StatusCode::BadRequest, *validationError);
        }

        if (action == QStringLiteral("delete")) {
            playlistDao.deletePlaylist(playlistId);
            if (m_activePlaylistId == playlistId) {
                m_activePlaylistId = -1;
            }
            return successResponse(QJsonObject{{"deleted", playlistId}});
        }

        if (action == QStringLiteral("rename")) {
            const QString name = body.value("name").toString();
            if (name.isEmpty()) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("Playlist name is required"));
            }
            playlistDao.renamePlaylist(playlistId, name);
            return successResponse(QJsonObject{{"playlist_id", playlistId}, {"name", name}});
        }

        if (action == QStringLiteral("send_to_autodj")) {
            QString locString = body.value("position").toString(QStringLiteral("bottom")).toLower();
            PlaylistDAO::AutoDJSendLoc loc = PlaylistDAO::AutoDJSendLoc::BOTTOM;
            if (locString == QStringLiteral("top")) {
                loc = PlaylistDAO::AutoDJSendLoc::TOP;
            } else if (locString == QStringLiteral("replace")) {
                loc = PlaylistDAO::AutoDJSendLoc::REPLACE;
            }
            playlistDao.addPlaylistToAutoDJQueue(playlistId, loc);
            return successResponse(QJsonObject{
                    {"playlist_id", playlistId},
                    {"position", locString},
            });
        }

        if (action == QStringLiteral("set_active")) {
            m_activePlaylistId = playlistId;
            return successResponse(QJsonObject{{"active_playlist_id", m_activePlaylistId}});
        }

        if (action == QStringLiteral("add")) {
            const auto tracksValue = body.value("track_ids");
            if (!tracksValue.isArray()) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("track_ids must be an array"));
            }
            const auto tracksArray = tracksValue.toArray();
            for (int index = 0; index < tracksArray.size(); ++index) {
                const auto& value = tracksArray.at(index);
                if (!value.isDouble() && !value.isString()) {
                    return errorResponse(
                            QHttpServerResponse::StatusCode::BadRequest,
                            tr("track_ids must contain numbers or strings"));
                }
            }
            QStringList parseErrors;
            const QList<TrackId> trackIds = parseTrackIds(tracksArray, &parseErrors);
            if (!parseErrors.isEmpty()) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        parseErrors.join(QLatin1String("; ")));
            }

            if (trackIds.isEmpty()) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("No track ids provided"));
            }

            const auto positionValue = body.value("position");
            if (positionValue.isUndefined()) {
                playlistDao.appendTracksToPlaylist(trackIds, playlistId);
            } else {
                if (!positionValue.isDouble()) {
                    return errorResponse(
                            QHttpServerResponse::StatusCode::BadRequest,
                            tr("Playlist position must be numeric"));
                }
                const int position = positionValue.toInt();
                const int trackCount = playlistDao.tracksInPlaylist(playlistId);
                if (position < 0 || position > trackCount) {
                    return errorResponse(
                            QHttpServerResponse::StatusCode::BadRequest,
                            tr("Playlist position %1 is out of range (valid 0-%2)")
                                    .arg(position)
                                    .arg(trackCount));
                }
                playlistDao.insertTracksIntoPlaylist(trackIds, playlistId, position);
            }
            return successResponse(QJsonObject{{"playlist_id", playlistId}});
        }

        if (action == QStringLiteral("remove")) {
            QList<int> positions;
            const auto positionsValue = body.value("positions");
            if (!positionsValue.isArray()) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("positions must be an array"));
            }
            const auto positionsArray = positionsValue.toArray();
            for (int index = 0; index < positionsArray.size(); ++index) {
                if (!positionsArray.at(index).isDouble()) {
                    return errorResponse(
                            QHttpServerResponse::StatusCode::BadRequest,
                            tr("positions must contain numbers"));
                }
            }
            positions.reserve(positionsArray.size());
            for (const auto& value : positionsArray) {
                if (!value.isDouble()) {
                    return errorResponse(
                            QHttpServerResponse::StatusCode::BadRequest,
                            tr("Playlist positions must be numeric"));
                }
                const int position = value.toInt();
                positions.append(position);
            }
            if (positions.isEmpty()) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("No playlist positions provided to remove"));
            }
            playlistDao.removeTracksFromPlaylist(playlistId, positions);
            return successResponse(QJsonObject{
                    {"playlist_id", playlistId},
                    {"removed", positionsArray},
            });
        }

        if (action == QStringLiteral("reorder")) {
            const auto fromValue = body.value("from");
            if (!fromValue.isDouble()) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("Reorder requires a from position"));
            }
            const auto toValue = body.value("to");
            if (!toValue.isDouble()) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("Reorder requires a destination position"));
            }
            const int from = fromValue.toInt();
            const int to = toValue.toInt();
            const int trackCount = playlistDao.tracksInPlaylist(playlistId);
            if (trackCount == 0) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("Playlist is empty"));
            }
            const int maxPosition = trackCount - 1;
            if (from < 0 || from > maxPosition) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("Reorder from position %1 is out of range (valid 0-%2)")
                                .arg(from)
                                .arg(maxPosition));
            }
            if (to < 0 || to > maxPosition) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("Reorder destination position %1 is out of range (valid 0-%2)")
                                .arg(to)
                                .arg(maxPosition));
            }
            playlistDao.moveTrack(playlistId, from, to);
            return successResponse(QJsonObject{
                    {"playlist_id", playlistId},
                    {"from", from},
                    {"to", to},
            });
        }

        return errorResponse(
                QHttpServerResponse::StatusCode::BadRequest,
                tr("Unsupported playlist action: %1").arg(action));
    });
}

QHttpServerResponse RestApiGateway::autoDjStatus() const {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    return withPlaylistDao([&](PlaylistDAO& playlistDao) {
        const int autoDjPlaylistId = playlistDao.getPlaylistIdFromName(AUTODJ_TABLE);
        const QList<TrackId> trackIds = autoDjPlaylistId == kInvalidPlaylistId
                ? QList<TrackId>{}
                : playlistDao.getAutoDJTrackIds();

        QJsonArray tracks;
        const int trackCount = trackIds.size();
        const int sampleLimit = qMin(trackCount, 10);
        for (int index = 0; index < sampleLimit; ++index) {
            const TrackPointer track = m_trackCollectionManager->getTrackById(trackIds[index]);
            QJsonObject entry;
            entry.insert("id", trackIds[index].toString());
            if (track) {
                entry.insert("track", trackPayload(track));
                entry.insert("duration", track->getDuration());
            }
            tracks.append(entry);
        }

        return successResponse(QJsonObject{
                {"enabled",
                        ControlProxy(QStringLiteral("[AutoDJ]"),
                                QStringLiteral("enabled"),
                                nullptr)
                                .toBool()},
                {"playlist_id", autoDjPlaylistId},
                {"queue_size", trackCount},
                {"tracks", tracks},
        });
    });
}

QJsonObject RestApiGateway::readinessPayload() const {
    QJsonArray issues;
    bool ready = true;

    if (!m_playerManager || m_playerManager->numberOfDecks() == 0) {
        issues.append(tr("No decks available"));
        ready = false;
    }

    auto* const collection = m_trackCollectionManager->internalCollection();
    if (collection == nullptr) {
        issues.append(tr("Track collection is not available"));
        ready = false;
    }

    return QJsonObject{
            {"ready", ready},
            {"issues", issues},
    };
}

QJsonObject RestApiGateway::appInfo() const {
    QJsonObject info;
    info.insert("name", VersionStore::applicationName());
    info.insert("version", VersionStore::version());
    info.insert("git", VersionStore::gitVersion());
    info.insert("platform", VersionStore::platform());
    return info;
}

QJsonObject RestApiGateway::mixerState() const {
    QJsonObject mixer;
    ControlProxy crossfader(QStringLiteral("[Master]"), QStringLiteral("crossfader"), nullptr);
    ControlProxy gain(QStringLiteral("[Master]"), QStringLiteral("gain"), nullptr);
    ControlProxy balance(QStringLiteral("[Master]"), QStringLiteral("balance"), nullptr);
    mixer.insert("crossfader", crossfader.get());
    mixer.insert("gain", gain.get());
    mixer.insert("balance", balance.get());
    return mixer;
}

QJsonObject RestApiGateway::broadcastState() const {
    QJsonObject broadcast;
    ControlProxy status(QStringLiteral(BROADCAST_PREF_KEY), QStringLiteral("status"), nullptr);
    ControlProxy enabled(QStringLiteral(BROADCAST_PREF_KEY), QStringLiteral("enabled"), nullptr);
    broadcast.insert("enabled", enabled.toBool());
    broadcast.insert("status", status.get());
    return broadcast;
}

QJsonObject RestApiGateway::recordingState() const {
    QJsonObject recording;
    ControlProxy status(QStringLiteral("[Recording]"), QStringLiteral("status"), nullptr);
    ControlProxy toggle(QStringLiteral("[Recording]"), QStringLiteral("toggle_recording"), nullptr);
    recording.insert("status", status.get());
    recording.insert("toggle", toggle.toBool());
    return recording;
}

QJsonObject RestApiGateway::autoDjOverview() const {
    ControlProxy enabled(QStringLiteral("[AutoDJ]"), QStringLiteral("enabled"), nullptr);
    QJsonObject autodj;
    autodj.insert("enabled", enabled.toBool());
    auto* const collection = m_trackCollectionManager->internalCollection();
    if (collection != nullptr) {
        PlaylistDAO& playlistDao = collection->getPlaylistDAO();
        const int autoDjPlaylistId = playlistDao.getPlaylistIdFromName(AUTODJ_TABLE);
        if (autoDjPlaylistId != kInvalidPlaylistId) {
            autodj.insert("playlist_id", autoDjPlaylistId);
            autodj.insert("queue_size", playlistDao.getAutoDJTrackIds().size());
        } else {
            autodj.insert("queue_size", 0);
        }
    } else {
        autodj.insert("queue_size", 0);
    }
    return autodj;
}

QJsonObject RestApiGateway::systemHealth() const {
    QJsonObject system;
    system.insert("logical_cores", QThread::idealThreadCount());

    if (const auto cpu = cpuUsagePercent()) {
        system.insert("cpu_usage_percent", *cpu);
    } else {
        system.insert("cpu_usage_percent", QJsonValue());
    }

    if (const auto rss = rssBytes()) {
        system.insert("rss_bytes", static_cast<qint64>(*rss));
    } else {
        system.insert("rss_bytes", QJsonValue());
    }

    return system;
}

QJsonObject RestApiGateway::trackPayload(const TrackPointer& track) const {
    QJsonObject payload;
    payload.insert("id", track->getId().toString());
    payload.insert("title", track->getTitle());
    payload.insert("artist", track->getArtist());
    payload.insert("album", track->getAlbum());
    payload.insert("duration", track->getDuration());
    payload.insert("bpm", track->getBpm());
    return payload;
}

QHttpServerResponse RestApiGateway::withPlaylistDao(
        const std::function<QHttpServerResponse(PlaylistDAO&)>& handler) const {
    auto* const collection = m_trackCollectionManager->internalCollection();
    if (collection == nullptr) {
        return errorResponse(
                QHttpServerResponse::StatusCode::ServiceUnavailable,
                tr("Track collection is not available"));
    }
    PlaylistDAO& playlistDao = collection->getPlaylistDAO();
    return handler(playlistDao);
}

QList<TrackId> RestApiGateway::parseTrackIds(
        const QJsonArray& values, QStringList* errors) const {
    QList<TrackId> trackIds;
    trackIds.reserve(values.size());
    int index = 0;
    for (const auto& value : values) {
        const TrackId trackId = TrackId(value.toVariant());
        if (!trackId.isValid()) {
            errors->append(tr("Invalid track id at index %1").arg(index));
        } else {
            trackIds.append(trackId);
        }
        ++index;
    }
    return trackIds;
}

int RestApiGateway::ensureAutoDjPlaylistId(PlaylistDAO& playlistDao) const {
    int playlistId = playlistDao.getPlaylistIdFromName(AUTODJ_TABLE);
    if (playlistId == kInvalidPlaylistId) {
        playlistId = playlistDao.createPlaylist(AUTODJ_TABLE, PlaylistDAO::PLHT_AUTO_DJ);
    }
    return playlistId;
}

std::optional<double> RestApiGateway::cpuUsagePercent() const {
#ifdef Q_OS_LINUX
    auto readCpuTotals = []() -> std::optional<quint64> {
        QFile file(QStringLiteral("/proc/stat"));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return std::nullopt;
        }
        const QByteArray line = file.readLine();
        const QList<QByteArray> parts = line.split(' ');
        quint64 total = 0;
        for (const auto& part : parts.mid(1)) {
            if (part.isEmpty()) {
                continue;
            }
            bool ok = false;
            const quint64 value = part.toULongLong(&ok);
            if (ok) {
                total += value;
            }
        }
        return total;
    };

    auto readProcessTime = []() -> std::optional<quint64> {
        QFile file(QStringLiteral("/proc/self/stat"));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return std::nullopt;
        }
        const QList<QByteArray> parts = file.readAll().split(' ');
        // utime (14) + stime (15)
        if (parts.size() < 16) {
            return std::nullopt;
        }
        bool ok1 = false;
        bool ok2 = false;
        const quint64 utime = parts.at(13).toULongLong(&ok1);
        const quint64 stime = parts.at(14).toULongLong(&ok2);
        if (!ok1 || !ok2) {
            return std::nullopt;
        }
        return utime + stime;
    };

    static quint64 s_prevProcess = 0;
    static quint64 s_prevTotal = 0;
    const auto process = readProcessTime();
    const auto total = readCpuTotals();
    if (!process.has_value() || !total.has_value()) {
        return std::nullopt;
    }
    if (s_prevProcess == 0 || s_prevTotal == 0) {
        s_prevProcess = *process;
        s_prevTotal = *total;
        return std::nullopt;
    }

    const quint64 deltaProc = *process - s_prevProcess;
    const quint64 deltaTotal = *total - s_prevTotal;
    s_prevProcess = *process;
    s_prevTotal = *total;
    if (deltaTotal == 0) {
        return std::nullopt;
    }

    const int cores = QThread::idealThreadCount();
    const double usage = (static_cast<double>(deltaProc) / static_cast<double>(deltaTotal)) * cores * 100.0;
    return usage;
#else
    return std::nullopt;
#endif
}

std::optional<quint64> RestApiGateway::rssBytes() const {
#ifdef Q_OS_LINUX
    QFile file(QStringLiteral("/proc/self/statm"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return std::nullopt;
    }
    const QList<QByteArray> parts = file.readAll().split(' ');
    if (parts.size() < 2) {
        return std::nullopt;
    }
    bool ok = false;
    const quint64 rssPages = parts.at(1).toULongLong(&ok);
    if (!ok) {
        return std::nullopt;
    }
    const long pageSize = sysconf(_SC_PAGESIZE);
    if (pageSize <= 0) {
        return std::nullopt;
    }
    return rssPages * static_cast<quint64>(pageSize);
#else
    return std::nullopt;
#endif
}

} // namespace mixxx::network::rest

#endif // MIXXX_HAS_HTTP_SERVER
