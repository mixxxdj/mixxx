#pragma once

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QHttpServerResponse>
#include <QHash>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QElapsedTimer>
#include <QMutex>
#include <optional>
#include <functional>

#include "preferences/usersettings.h"

class PlayerManager;
class TrackCollectionManager;
class PlaylistDAO;

namespace mixxx::network::rest {

class RestApiProvider : public QObject {
    Q_OBJECT

  public:
    using QObject::QObject;
    ~RestApiProvider() override = default;

    virtual QHttpServerResponse health() const = 0;
    virtual QHttpServerResponse ready() const = 0;
    virtual QHttpServerResponse status() const = 0;
    virtual QHttpServerResponse deck(int deckNumber) const = 0;
    virtual QHttpServerResponse decks() const = 0;
    virtual QHttpServerResponse control(const QJsonObject& body) const = 0;
    virtual QHttpServerResponse autoDjStatus() const = 0;
    virtual QHttpServerResponse autoDj(const QJsonObject& body) const = 0;
    virtual QHttpServerResponse playlists(const std::optional<int>& playlistId) const = 0;
    virtual QHttpServerResponse playlistCommand(const QJsonObject& body) const = 0;
    virtual QHttpServerResponse withIdempotencyCache(
            const QString& token,
            const QString& idempotencyKey,
            const QString& endpoint,
            const std::function<QHttpServerResponse()>& handler) const {
        return handler();
    }
};

class RestApiGateway : public RestApiProvider {
    Q_OBJECT

  public:
    RestApiGateway(
            PlayerManager* playerManager,
            TrackCollectionManager* trackCollectionManager,
            const UserSettingsPointer& settings,
            QObject* parent = nullptr);

    QHttpServerResponse health() const;
    QHttpServerResponse ready() const;
    QHttpServerResponse status() const;
    QHttpServerResponse deck(int deckNumber) const;
    QHttpServerResponse decks() const;
    QHttpServerResponse control(
            const QString& group,
            const QString& key,
            const std::optional<double>& value) const = delete;
    QHttpServerResponse control(const QJsonObject& body) const;
    QHttpServerResponse autoDjStatus() const;
    QHttpServerResponse autoDj(const QJsonObject& body) const;
    QHttpServerResponse playlists(const std::optional<int>& playlistId) const;
    QHttpServerResponse playlistCommand(const QJsonObject& body) const;
    QHttpServerResponse withIdempotencyCache(
            const QString& token,
            const QString& idempotencyKey,
            const QString& endpoint,
            const std::function<QHttpServerResponse()>& handler) const override;

  private:
    struct IdempotencyEntry {
        QDateTime createdUtc;
        QHttpServerResponse response;
    };

    QHttpServerResponse errorResponse(
            QHttpServerResponse::StatusCode code,
            const QString& message) const;
    QHttpServerResponse successResponse(
            const QJsonObject& payload,
            QHttpServerResponse::StatusCode statusCode =
                    QHttpServerResponse::StatusCode::Ok) const;
    QHttpServerResponse withPlaylistDao(
            const std::function<QHttpServerResponse(PlaylistDAO&)>& handler) const;
    QJsonArray deckStatuses() const;
    QJsonObject deckSummary(int deckIndex) const;
    QJsonObject readinessPayload() const;
    QJsonObject appInfo() const;
    QJsonObject systemHealth() const;
    QJsonObject mixerState() const;
    QJsonObject broadcastState() const;
    QJsonObject recordingState() const;
    QJsonObject autoDjOverview() const;
    QJsonObject trackPayload(const TrackPointer& track) const;
    QList<TrackId> parseTrackIds(const QJsonArray& values, QStringList* errors) const;
    int ensureAutoDjPlaylistId(PlaylistDAO& playlistDao) const;
    std::optional<double> cpuUsagePercent() const;
    std::optional<quint64> rssBytes() const;

    PlayerManager* const m_playerManager;
    TrackCollectionManager* const m_trackCollectionManager;
    [[maybe_unused]] const UserSettingsPointer m_settings;
    int m_activePlaylistId{-1};
    QElapsedTimer m_uptime;
    mutable QHash<QString, IdempotencyEntry> m_idempotencyCache;
    mutable QMutex m_idempotencyMutex;
};

} // namespace mixxx::network::rest

#endif // MIXXX_HAS_HTTP_SERVER
