#include "network/rest/restapigateway.h"

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QJsonDocument>
#include <QJsonValue>

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
} // namespace

RestApiGateway::RestApiGateway(
        PlayerManager* playerManager,
        TrackCollectionManager* trackCollectionManager,
        const UserSettingsPointer& settings,
        QObject* parent)
        : QObject(parent),
          m_playerManager(playerManager),
          m_trackCollectionManager(trackCollectionManager),
          m_settings(settings) {
    DEBUG_ASSERT(m_playerManager);
    DEBUG_ASSERT(m_trackCollectionManager);
}

QHttpServerResponse RestApiGateway::errorResponse(
        QHttpServerResponse::StatusCode code,
        const QString& message) const {
    return QHttpServerResponse(
            code,
            QJsonDocument(QJsonObject{
                    {"error", message},
            })
                    .toJson(QJsonDocument::Compact),
            kApplicationJsonMimeType);
}

QHttpServerResponse RestApiGateway::successResponse(
        const QJsonObject& payload,
        QHttpServerResponse::StatusCode statusCode) const {
    return QHttpServerResponse(
            statusCode,
            QJsonDocument(payload).toJson(QJsonDocument::Compact),
            kApplicationJsonMimeType);
}

QHttpServerResponse RestApiGateway::health() const {
    return successResponse(QJsonObject{{"status", "ok"}});
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
    deck.insert("playing", playControl.toBool());
    deck.insert("track_loaded", trackLoaded.toBool());
    deck.insert("position", playPosition.get());
    deck.insert("rate", rate.get());
    deck.insert("gain", gain.get());
    deck.insert("volume", volume.get());

    BaseTrackPlayer* const player = m_playerManager->getDeck(deckIndex + 1);
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
    decks.reserve(static_cast<int>(totalDecks));
    for (unsigned int i = 0; i < totalDecks; ++i) {
        decks.append(deckSummary(static_cast<int>(i)));
    }
    return decks;
}

QHttpServerResponse RestApiGateway::status() const {
    const QJsonObject payload{
            {"app", appInfo()},
            {"decks", deckStatuses()},
            {"mixer", mixerState()},
            {"broadcast", broadcastState()},
            {"recording", recordingState()},
            {"autodj", autoDjOverview()},
    };
    return successResponse(payload);
}

QHttpServerResponse RestApiGateway::control(const QJsonObject& body) const {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    const QString command = body.value("command").toString();
    QString group = body.value("group").toString();
    if (group.isEmpty()) {
        bool ok = false;
        const int deckIndex = body.value("deck").toInt(&ok);
        if (ok && deckIndex > 0) {
            group = PlayerManager::groupForDeck(deckIndex - 1);
        }
    }
    if (group.isEmpty()) {
        return errorResponse(
                QHttpServerResponse::StatusCode::BadRequest,
                tr("Missing control group"));
    }

    auto createControl = [&](const QString& controlKey) -> std::optional<ControlProxy> {
        ControlProxy control(group, controlKey, nullptr);
        if (!control.valid()) {
            return std::nullopt;
        }
        return control;
    };

    const QString key = body.value("key").toString();
    if (command.isEmpty() && key.isEmpty()) {
        return errorResponse(
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

    auto ensureControl = [&](const QString& controlKey) -> std::optional<ControlProxy> {
        if (auto controlProxy = createControl(controlKey)) {
            return controlProxy;
        }
        return std::nullopt;
    };

    auto finalizeResponse = [&](const QString& responseKey,
                                const QString& responseGroup,
                                double responseValue) {
        return successResponse(QJsonObject{
                {"group", responseGroup},
                {"key", responseKey},
                {"value", responseValue},
        });
    };

    if (!command.isEmpty()) {
        if (command == QStringLiteral("play") || command == QStringLiteral("pause") ||
                command == QStringLiteral("toggle")) {
            auto control = ensureControl(QStringLiteral("play"));
            if (!control.has_value()) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("Unknown control %1 play").arg(group));
            }
            if (command == QStringLiteral("toggle")) {
                return finalizeResponse("play", group, toggleControl(control.value()));
            }
            const double value = command == QStringLiteral("play") ? 1.0 : 0.0;
            return finalizeResponse("play", group, applyValue(control.value(), value));
        }
        if (command == QStringLiteral("seek")) {
            bool ok = false;
            const double position = body.value("position").toDouble(&ok);
            if (!ok) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("Seek position must be numeric"));
            }
            auto control = ensureControl(QStringLiteral("playposition"));
            if (!control.has_value()) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("Unknown control %1 playposition").arg(group));
            }
            return finalizeResponse("playposition", group, applyValue(control.value(), position));
        }
        if (command == QStringLiteral("gain")) {
            bool ok = false;
            const double gain = body.value("value").toDouble(&ok);
            if (!ok) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("Gain must be numeric"));
            }
            auto control = ensureControl(QStringLiteral("gain"));
            if (!control.has_value()) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("Unknown control %1 gain").arg(group));
            }
            return finalizeResponse("gain", group, applyValue(control.value(), gain));
        }

        return errorResponse(
                QHttpServerResponse::StatusCode::BadRequest,
                tr("Unsupported control command: %1").arg(command));
    }

    ControlProxy control(group, key, nullptr);
    if (!control.valid()) {
        return errorResponse(
                QHttpServerResponse::StatusCode::BadRequest,
                tr("Unknown control %1 %2").arg(group, key));
    }

    const auto valueVariant = body.value("value");
    const std::optional<double> controlValue = valueVariant.isUndefined()
            ? std::optional<double>{}
            : std::optional<double>{valueVariant.toDouble()};

    const double resultValue = applyValue(control, controlValue);
    return finalizeResponse(key, group, resultValue);
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
                const auto tracksValue = body.value("track_ids").toArray();
                QStringList parseErrors;
                const QList<TrackId> trackIds = parseTrackIds(tracksValue, &parseErrors);
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

            bool ok = false;
            const int from = body.value("from").toInt(&ok);
            if (!ok) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("Move command is missing from position"));
            }
            ok = false;
            const int to = body.value("to").toInt(&ok);
            if (!ok) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("Move command is missing destination position"));
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
            tracks.reserve(trackIds.size());
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
        payload.reserve(playlists.size());
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

        if (action == QStringLiteral("set_active")) {
            m_activePlaylistId = playlistId;
            return successResponse(QJsonObject{{"active_playlist_id", m_activePlaylistId}});
        }

        if (action == QStringLiteral("add")) {
            const auto tracksValue = body.value("track_ids").toArray();
            QStringList parseErrors;
            const QList<TrackId> trackIds = parseTrackIds(tracksValue, &parseErrors);
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
                int position = positionValue.toInt();
                playlistDao.insertTracksIntoPlaylist(trackIds, playlistId, position);
            }
            return successResponse(QJsonObject{{"playlist_id", playlistId}});
        }

        if (action == QStringLiteral("remove")) {
            QList<int> positions;
            const auto positionsValue = body.value("positions").toArray();
            positions.reserve(positionsValue.size());
            for (const auto& value : positionsValue) {
                positions.append(value.toInt());
            }
            if (positions.isEmpty()) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("No playlist positions provided to remove"));
            }
            playlistDao.removeTracksFromPlaylist(playlistId, positions);
            return successResponse(QJsonObject{
                    {"playlist_id", playlistId},
                    {"removed", positionsValue},
            });
        }

        if (action == QStringLiteral("reorder")) {
            bool ok = false;
            const int from = body.value("from").toInt(&ok);
            if (!ok) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("Reorder requires a from position"));
            }
            ok = false;
            const int to = body.value("to").toInt(&ok);
            if (!ok) {
                return errorResponse(
                        QHttpServerResponse::StatusCode::BadRequest,
                        tr("Reorder requires a destination position"));
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

} // namespace mixxx::network::rest

#endif // MIXXX_HAS_HTTP_SERVER
