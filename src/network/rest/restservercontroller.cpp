#include "network/rest/restservercontroller.h"

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QHostAddress>
#include <QtGlobal>

#include <QCoreApplication>

#include "moc_restservercontroller.cpp"
#include "mixer/playermanager.h"
#include "network/rest/restapigateway.h"
#include "preferences/configobject.h"
#include "util/logger.h"

namespace {
constexpr quint16 kDefaultRestPort = 8989;
const ConfigKey kEnabledKey(QStringLiteral("[Rest]"), QStringLiteral("enabled"));
const ConfigKey kHostKey(QStringLiteral("[Rest]"), QStringLiteral("host"));
const ConfigKey kPortKey(QStringLiteral("[Rest]"), QStringLiteral("port"));
const ConfigKey kTlsEnabledKey(QStringLiteral("[Rest]"), QStringLiteral("tls_enabled"));
const ConfigKey kCertificatePathKey(QStringLiteral("[Rest]"), QStringLiteral("tls_certificate_path"));
const ConfigKey kPrivateKeyPathKey(QStringLiteral("[Rest]"), QStringLiteral("tls_private_key_path"));
const ConfigKey kAuthTokenKey(QStringLiteral("[Rest]"), QStringLiteral("token"));
constexpr int kRefreshIntervalMs = 2000;
} // namespace

namespace mixxx::network::rest {

namespace {
const Logger kLogger("mixxx::network::rest::RestServerController");
} // namespace

RestServerController::RestServerController(
        const UserSettingsPointer& settings,
        PlayerManager* playerManager,
        TrackCollectionManager* trackCollectionManager,
        QObject* parent)
        : QObject(parent),
          m_settings(settings),
          m_playerManager(playerManager),
          m_trackCollectionManager(trackCollectionManager),
          m_gateway(std::make_unique<RestApiGateway>(
                  m_playerManager,
                  m_trackCollectionManager,
                  m_settings)),
          m_server(std::make_unique<RestServer>(m_gateway.get())) {
    DEBUG_ASSERT(m_settings);
    DEBUG_ASSERT(m_playerManager);
    DEBUG_ASSERT(m_trackCollectionManager);

    m_reloadTimer.setInterval(kRefreshIntervalMs);
    m_reloadTimer.setSingleShot(false);
    connect(&m_reloadTimer, &QTimer::timeout, this, &RestServerController::refreshFromSettings);
}

RestServerController::~RestServerController() {
    shutdown();
}

RestServer::Settings RestServerController::loadSettings() const {
    RestServer::Settings settings;
    if (m_settings) {
        settings.enabled = m_settings->getValue<bool>(kEnabledKey, false);
        const QString hostValue = m_settings->getValue<QString>(
                kHostKey, QHostAddress(QHostAddress::LocalHost).toString());
        QHostAddress address;
        if (!address.setAddress(hostValue)) {
            kLogger.warning()
                    << "Invalid REST API host address configured:"
                    << hostValue
                    << "falling back to localhost";
            address = QHostAddress::LocalHost;
        }
        settings.address = address;
        const int configuredPort = m_settings->getValue<int>(kPortKey, kDefaultRestPort);
        settings.port = qBound(1, configuredPort, 65535);
        settings.tlsEnabled = m_settings->getValue<bool>(kTlsEnabledKey, false);
        settings.certificatePath = m_settings->getValue<QString>(kCertificatePathKey);
        settings.privateKeyPath = m_settings->getValue<QString>(kPrivateKeyPathKey);
        settings.authToken = m_settings->getValue<QString>(kAuthTokenKey);
    }
    return settings;
}

void RestServerController::applySettings(const RestServer::Settings& settings) {
    if (settings != m_activeSettings) {
        m_loggedStartFailure = false;
    }

    if (!settings.enabled) {
        if (m_server && m_server->isRunning()) {
            kLogger.info() << "Stopping REST API server because it is disabled";
            m_server->stop();
        }
        m_loggedStartFailure = false;
        m_activeSettings = settings;
        return;
    }

    if (settings == m_activeSettings && m_server && m_server->isRunning()) {
        return;
    }

    if (!m_server) {
        kLogger.warning() << "REST API server missing; cannot start";
        return;
    }

    if (m_server->start(settings)) {
        m_activeSettings = settings;
        m_loggedStartFailure = false;
    } else {
        if (!m_loggedStartFailure) {
            kLogger.warning() << "Failed to start REST API server";
            m_loggedStartFailure = true;
        }
    }
}

void RestServerController::start() {
    refreshFromSettings();
    m_reloadTimer.start();
}

void RestServerController::shutdown() {
    m_reloadTimer.stop();
    if (m_server) {
        m_server->stop();
    }
}

void RestServerController::refreshFromSettings() {
    applySettings(loadSettings());
}

} // namespace mixxx::network::rest

#endif // MIXXX_HAS_HTTP_SERVER
