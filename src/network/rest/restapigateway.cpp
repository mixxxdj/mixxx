#include "network/rest/restapigateway.h"

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QJsonDocument>
#include <QJsonValue>

#include "moc_restapigateway.cpp"
#include "control/controlproxy.h"
#include "library/dao/playlistdao.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "mixer/basetrackplayer.h"
#include "mixer/playermanager.h"
#include "preferences/configobject.h"
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

QHttpServerResponse RestApiGateway::successResponse(const QJsonObject& payload) const {
    return QHttpServerResponse(
            QHttpServerResponse::StatusCode::Ok,
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
    deck.insert("playing", playControl.toBool());
    deck.insert("track_loaded", trackLoaded.toBool());

    BaseTrackPlayer* const player = m_playerManager->getDeck(deckIndex + 1);
    if (player != nullptr) {
        if (const TrackPointer track = player->getLoadedTrack()) {
            deck.insert("title", track->getTitle());
            deck.insert("artist", track->getArtist());
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
            {"decks", deckStatuses()},
            {"autodj_enabled",
                    ControlProxy(QStringLiteral("[AutoDJ]"),
                            QStringLiteral("enabled"),
                            nullptr)
                            .toBool()},
    };
    return successResponse(payload);
}

QHttpServerResponse RestApiGateway::control(
        const QString& group,
        const QString& key,
        const std::optional<double>& value) const {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    ControlProxy control(group, key, nullptr);
    if (!control.valid()) {
        return errorResponse(
                QHttpServerResponse::StatusCode::BadRequest,
                tr("Unknown control %1 %2").arg(group, key));
    }

    control.set(value.value_or(1.0));
    return successResponse(QJsonObject{
            {"group", group},
            {"key", key},
            {"value", control.get()},
    });
}

QHttpServerResponse RestApiGateway::autoDj(const QString& action) const {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

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
    } else {
        return errorResponse(
                QHttpServerResponse::StatusCode::BadRequest,
                tr("Unsupported AutoDJ action: %1").arg(action));
    }

    return successResponse(QJsonObject{
            {"action", action},
            {"enabled", enabled.toBool()},
    });
}

QHttpServerResponse RestApiGateway::playlists() const {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    auto* const collection = m_trackCollectionManager->internalCollection();
    if (collection == nullptr) {
        return errorResponse(
                QHttpServerResponse::StatusCode::ServiceUnavailable,
                tr("Track collection is not available"));
    }

    PlaylistDAO& playlistDao = collection->getPlaylistDAO();
    const auto playlists = playlistDao.getPlaylists(PlaylistDAO::PLHT_NOT_HIDDEN);
    QJsonArray payload;
    payload.reserve(playlists.size());
    for (const auto& playlist : playlists) {
        QJsonObject entry;
        entry.insert("id", playlist.first);
        entry.insert("name", playlist.second);
        entry.insert("tracks", playlistDao.tracksInPlaylist(playlist.first));
        payload.append(entry);
    }
    return successResponse(QJsonObject{{"playlists", payload}});
}

QHttpServerResponse RestApiGateway::playlistTracks(int playlistId) const {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    if (playlistId < 0) {
        return errorResponse(
                QHttpServerResponse::StatusCode::BadRequest,
                tr("Playlist id must be positive"));
    }

    auto* const collection = m_trackCollectionManager->internalCollection();
    if (collection == nullptr) {
        return errorResponse(
                QHttpServerResponse::StatusCode::ServiceUnavailable,
                tr("Track collection is not available"));
    }

    PlaylistDAO& playlistDao = collection->getPlaylistDAO();
    if (playlistDao.isHidden(playlistId)) {
        return errorResponse(
                QHttpServerResponse::StatusCode::NotFound,
                tr("Playlist %1 does not exist").arg(playlistId));
    }

    const QList<TrackId> trackIds = playlistDao.getTrackIdsInPlaylistOrder(playlistId);
    QJsonArray tracks;
    tracks.reserve(trackIds.size());
    for (const TrackId& trackId : trackIds) {
        QJsonObject entry;
        entry.insert("id", trackId.toString());
        const TrackPointer track = m_trackCollectionManager->getTrackById(trackId);
        if (track) {
            entry.insert("title", track->getTitle());
            entry.insert("artist", track->getArtist());
            entry.insert("duration", track->getDuration());
        }
        tracks.append(entry);
    }

    return successResponse(QJsonObject{
            {"playlist_id", playlistId},
            {"tracks", tracks},
    });
}

} // namespace mixxx::network::rest

#endif // MIXXX_HAS_HTTP_SERVER
