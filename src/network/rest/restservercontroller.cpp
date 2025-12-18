#include "network/rest/restservercontroller.h"

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QHostAddress>
#include <QtGlobal>

#include <QCoreApplication>

#include "moc_restservercontroller.cpp"
#include "mixer/playermanager.h"
#include "network/rest/restapigateway.h"
#include "util/logger.h"

namespace {
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
          m_settingsStore(m_settings),
          m_certificateGenerator(m_settings->getSettingsPath()),
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
    const RestServerSettings::Values values = m_settingsStore.get();
    settings.enabled = values.enabled;

    QHostAddress address;
    if (!address.setAddress(values.host)) {
        kLogger.warning()
                << "Invalid REST API host address configured:"
                << values.host
                << "falling back to localhost";
        address = QHostAddress::LocalHost;
    }
    settings.address = address;
    settings.port = qBound(1, values.port, 65535);
    settings.useHttps = values.useHttps;
    settings.autoGenerateCertificate = values.autoGenerateCert;
    settings.requireTls = values.requireTls;
    settings.certificatePath = values.certificatePath;
    settings.privateKeyPath = values.privateKeyPath;
    settings.authToken = values.authToken;
    return settings;
}

void RestServerController::applySettings(const RestServer::Settings& settings) {
    auto publishStatus = [this](const RestServerSettings::Status& status) {
        if (status.running == m_status.running &&
                status.tlsActive == m_status.tlsActive &&
                status.certificateGenerated == m_status.certificateGenerated &&
                status.lastError == m_status.lastError &&
                status.tlsError == m_status.tlsError) {
            return;
        }
        m_status = status;
        m_settingsStore.setStatus(status);
    };

    auto publishStartFailure = [&](const QString& message, const QString& tlsMessage, bool running) {
        RestServerSettings::Status status;
        status.running = running;
        status.tlsActive = running && m_activeSettings.useHttps;
        status.certificateGenerated = m_lastTlsConfiguration.has_value() &&
                m_lastTlsConfiguration->generated && status.tlsActive;
        status.lastError = message;
        status.tlsError = tlsMessage;
        publishStatus(status);
    };

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
        m_lastTlsConfiguration.reset();
        publishStatus(RestServerSettings::Status{});
        return;
    }

    if (!m_server) {
        kLogger.warning() << "REST API server missing; cannot start";
        publishStartFailure(tr("REST API server is unavailable"), QString(), false);
        return;
    }

    const bool wasRunning = m_server->isRunning();
    const RestServer::Settings previousSettings = m_activeSettings;
    const std::optional<RestServer::TlsResult> previousTlsConfiguration = m_lastTlsConfiguration;

    RestServer::Settings settingsToApply = settings;
    std::optional<RestServer::TlsResult> tlsConfiguration;
    QString tlsError;
    if (settingsToApply.useHttps) {
        const RestServer::TlsResult tlsResult = RestServer::prepareTlsConfiguration(
                settingsToApply, &m_certificateGenerator);
        if (!tlsResult.success) {
            if (!m_loggedStartFailure) {
                kLogger.warning() << "Failed to prepare REST API TLS configuration:" << tlsResult.error;
                m_loggedStartFailure = true;
            }
            publishStartFailure(tr("TLS configuration failed"), tlsResult.error, wasRunning);
            return;
        }
        tlsConfiguration = tlsResult;
        settingsToApply.certificatePath = tlsResult.certificatePath;
        settingsToApply.privateKeyPath = tlsResult.privateKeyPath;
    }

    if (settingsToApply == m_activeSettings && wasRunning) {
        RestServerSettings::Status status;
        status.running = true;
        status.tlsActive = settingsToApply.useHttps;
        status.certificateGenerated = tlsConfiguration.has_value() && tlsConfiguration->generated;
        publishStatus(status);
        return;
    }

    if (wasRunning) {
        m_server->stop();
    }

    QString startError;
    if (m_server->start(settingsToApply, tlsConfiguration, &startError)) {
        m_activeSettings = settingsToApply;
        m_lastTlsConfiguration = tlsConfiguration;
        m_loggedStartFailure = false;

        if (settingsToApply.autoGenerateCertificate) {
            RestServerSettings::Values storedValues = m_settingsStore.get();
            storedValues.certificatePath = settingsToApply.certificatePath;
            storedValues.privateKeyPath = settingsToApply.privateKeyPath;
            m_settingsStore.set(storedValues);
        }

        RestServerSettings::Status status;
        status.running = true;
        status.tlsActive = settingsToApply.useHttps;
        status.certificateGenerated = tlsConfiguration.has_value() && tlsConfiguration->generated;
        publishStatus(status);
        return;
    }

    if (!m_loggedStartFailure) {
        kLogger.warning() << "Failed to start REST API server:" << startError;
        m_loggedStartFailure = true;
    }
    publishStartFailure(
            startError.isEmpty() ? tr("Failed to start REST API server") : startError,
            tlsError,
            wasRunning);

    if (wasRunning && previousSettings.enabled) {
        QString restartError;
        if (m_server->start(previousSettings, previousTlsConfiguration, &restartError)) {
            m_activeSettings = previousSettings;
            m_lastTlsConfiguration = previousTlsConfiguration;
            RestServerSettings::Status status;
            status.running = true;
            status.tlsActive = previousSettings.useHttps;
            status.certificateGenerated = previousTlsConfiguration.has_value() &&
                    previousTlsConfiguration->generated;
            publishStatus(status);
        } else {
            publishStartFailure(
                    restartError.isEmpty()
                            ? tr("Failed to restart REST API with previous settings")
                            : restartError,
                    QString(),
                    false);
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
