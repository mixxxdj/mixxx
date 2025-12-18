#pragma once

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QHttpServerResponse>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <optional>

#include "preferences/usersettings.h"

class PlayerManager;
class TrackCollectionManager;

namespace mixxx::network::rest {

class RestApiGateway : public QObject {
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
            const std::optional<double>& value) const;
    QHttpServerResponse autoDj(const QString& action) const;
    QHttpServerResponse playlists() const;
    QHttpServerResponse playlistTracks(int playlistId) const;

  private:
    QHttpServerResponse errorResponse(
            QHttpServerResponse::StatusCode code,
            const QString& message) const;
    QHttpServerResponse successResponse(const QJsonObject& payload) const;
    QJsonArray deckStatuses() const;
    QJsonObject deckSummary(int deckIndex) const;

    PlayerManager* const m_playerManager;
    TrackCollectionManager* const m_trackCollectionManager;
    [[maybe_unused]] const UserSettingsPointer m_settings;
};

} // namespace mixxx::network::rest

#endif // MIXXX_HAS_HTTP_SERVER
