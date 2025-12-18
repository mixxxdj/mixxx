#pragma once

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QObject>
#include <QTimer>
#include <memory>
#include <optional>

#include "network/rest/restapigateway.h"
#include "network/rest/restserver.h"
#include "network/rest/certificategenerator.h"
#include "preferences/restserversettings.h"
#include "preferences/usersettings.h"

class PlayerManager;
class TrackCollectionManager;

namespace mixxx::network::rest {

class RestServerController : public QObject {
    Q_OBJECT

  public:
    RestServerController(
            const UserSettingsPointer& settings,
            PlayerManager* playerManager,
            TrackCollectionManager* trackCollectionManager,
            QObject* parent = nullptr);
    ~RestServerController() override;

    void start();

  public slots:
    void shutdown();

  private slots:
    void refreshFromSettings();

  private:
    RestServer::Settings loadSettings() const;
    void applySettings(const RestServer::Settings& settings);

    const UserSettingsPointer m_settings;
    PlayerManager* const m_playerManager;
    TrackCollectionManager* const m_trackCollectionManager;
    RestServerSettings m_settingsStore;
    CertificateGenerator m_certificateGenerator;
    std::unique_ptr<RestApiGateway> m_gateway;
    std::unique_ptr<RestServer> m_server;
    std::optional<RestServer::TlsResult> m_lastTlsConfiguration;
    RestServer::Settings m_activeSettings;
    RestServerSettings::Status m_status;
    QTimer m_reloadTimer;
    bool m_loggedStartFailure{false};
};

} // namespace mixxx::network::rest

#endif // MIXXX_HAS_HTTP_SERVER
