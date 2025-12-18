#pragma once

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QHttpServerResponse>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
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
    virtual QHttpServerResponse status() const = 0;
    virtual QHttpServerResponse control(const QJsonObject& body) const = 0;
    virtual QHttpServerResponse autoDjStatus() const = 0;
    virtual QHttpServerResponse autoDj(const QJsonObject& body) const = 0;
    virtual QHttpServerResponse playlists(const std::optional<int>& playlistId) const = 0;
    virtual QHttpServerResponse playlistCommand(const QJsonObject& body) const = 0;
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
    QHttpServerResponse status() const;
    QHttpServerResponse control(
            const QString& group,
            const QString& key,
            const std::optional<double>& value) const = delete;
    QHttpServerResponse control(const QJsonObject& body) const;
    QHttpServerResponse autoDjStatus() const;
    QHttpServerResponse autoDj(const QJsonObject& body) const;
    QHttpServerResponse playlists(const std::optional<int>& playlistId) const;
    QHttpServerResponse playlistCommand(const QJsonObject& body) const;

  private:
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
    QJsonObject appInfo() const;
    QJsonObject mixerState() const;
    QJsonObject broadcastState() const;
    QJsonObject recordingState() const;
    QJsonObject autoDjOverview() const;
    QJsonObject trackPayload(const TrackPointer& track) const;
    QList<TrackId> parseTrackIds(const QJsonArray& values, QStringList* errors) const;
    int ensureAutoDjPlaylistId(PlaylistDAO& playlistDao) const;

    PlayerManager* const m_playerManager;
    TrackCollectionManager* const m_trackCollectionManager;
    [[maybe_unused]] const UserSettingsPointer m_settings;
    int m_activePlaylistId{-1};
};

} // namespace mixxx::network::rest

#endif // MIXXX_HAS_HTTP_SERVER
