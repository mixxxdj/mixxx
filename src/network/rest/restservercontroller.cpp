#include "network/rest/restservercontroller.h"

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QHostAddress>

#include <QCoreApplication>
#include <QtGlobal>
#include <limits>

#include "moc_restservercontroller.cpp"
#include "mixer/playermanager.h"
#include "network/rest/restapigateway.h"
#include "network/rest/restservervalidator.h"
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
          m_httpServer(std::make_unique<RestServer>(m_gateway.get())),
          m_httpsServer(std::make_unique<RestServer>(m_gateway.get())) {
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

RestServerController::ListenerConfiguration RestServerController::loadSettings() const {
    const RestServerSettings::Values values = m_settingsStore.get();

    auto buildSettings = [&values](int port, bool useHttps) {
        RestServer::Settings settings;
        const bool listenerEnabled = useHttps ? values.useHttps : values.enableHttp;
        settings.enabled = values.enabled && listenerEnabled;
        settings.host = values.host;

        QHostAddress address;
        settings.hostValid = address.setAddress(values.host);
        settings.address = address;
        settings.port = port;
        settings.portValid = port > 0 && port <= std::numeric_limits<quint16>::max();
        settings.useHttps = useHttps;
        settings.autoGenerateCertificate = useHttps && values.autoGenerateCert;
        settings.requireTls = values.requireTls;
        settings.allowUnauthenticated = values.allowUnauthenticated;
        settings.certificatePath = values.certificatePath;
        settings.privateKeyPath = values.privateKeyPath;
        settings.maxRequestBytes = values.maxRequestBytes;
        const QStringList allowlistEntries =
                values.corsAllowlist.split(',', Qt::SkipEmptyParts);
        for (const auto& entry : allowlistEntries) {
            const QString trimmed = entry.trimmed();
            if (!trimmed.isEmpty()) {
                settings.corsAllowlist.append(trimmed);
            }
        }
        settings.tokens.clear();
        for (const auto& token : values.tokens) {
            RestServer::Token restToken;
            restToken.value = token.value;
            restToken.description = token.description;
            restToken.scopes = token.scopes;
            restToken.createdUtc = token.createdUtc;
            restToken.expiresUtc = token.expiresUtc;
            settings.tokens.append(restToken);
        }
        settings.streamEnabled = values.streamEnabled;
        settings.streamIntervalMs = values.streamIntervalMs;
        settings.streamMaxClients = values.streamMaxClients;
        return settings;
    };

    ListenerConfiguration configuration;
    configuration.enabled = values.enabled;
    configuration.enableHttp = values.enableHttp;
    configuration.enableHttps = values.useHttps;
    configuration.httpSettings = buildSettings(values.httpPort, false);
    configuration.httpsSettings = buildSettings(values.httpsPort, true);
    return configuration;
}

void RestServerController::applySettings(const ListenerConfiguration& settings) {
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
        status.tlsActive = running && m_activeHttpsSettings.useHttps;
        status.certificateGenerated = m_lastHttpsTlsConfiguration.has_value() &&
                m_lastHttpsTlsConfiguration->generated && status.tlsActive;
        status.lastError = message;
        status.tlsError = tlsMessage;
        publishStatus(status);
    };

    if (settings.httpSettings != m_activeHttpSettings || settings.httpsSettings != m_activeHttpsSettings) {
        m_loggedStartFailure = false;
    }

    const bool httpWasRunning = m_httpServer && m_httpServer->isRunning();
    const bool httpsWasRunning = m_httpsServer && m_httpsServer->isRunning();
    const RestServer::Settings previousHttpSettings = m_activeHttpSettings;
    const RestServer::Settings previousHttpsSettings = m_activeHttpsSettings;
    const std::optional<RestServer::TlsResult> previousTlsConfiguration = m_lastHttpsTlsConfiguration;

    if (!settings.enabled || (!settings.enableHttp && !settings.enableHttps)) {
        if (httpWasRunning) {
            kLogger.info() << "Stopping REST API HTTP server because it is disabled";
            m_httpServer->stop();
        }
        if (httpsWasRunning) {
            kLogger.info() << "Stopping REST API HTTPS server because it is disabled";
            m_httpsServer->stop();
        }
        m_loggedStartFailure = false;
        m_activeHttpSettings = RestServer::Settings{};
        m_activeHttpsSettings = RestServer::Settings{};
        m_lastHttpsTlsConfiguration.reset();
        publishStatus(RestServerSettings::Status{});
        return;
    }

    if (!m_httpServer || !m_httpsServer) {
        kLogger.warning() << "REST API server missing; cannot start";
        publishStartFailure(tr("REST API server is unavailable"), QString(), httpWasRunning || httpsWasRunning);
        return;
    }

    if (settings.enableHttp && settings.enableHttps &&
            settings.httpSettings.port == settings.httpsSettings.port) {
        const QString error = tr("HTTP and HTTPS ports must differ");
        if (!m_loggedStartFailure) {
            kLogger.warning() << error;
            m_loggedStartFailure = true;
        }
        publishStartFailure(error, QString(), httpWasRunning || httpsWasRunning);
        return;
    }

    auto validateServer = [&](const RestServer::Settings& requestedSettings,
                               const RestServer::Settings& activeSettings,
                               bool serverRunning,
                               CertificateGenerator* certificateGenerator)
            -> std::optional<RestServerValidationResult> {
        RestServerValidator validator(activeSettings, serverRunning, certificateGenerator);
        const RestServerValidationResult result = validator.validate(requestedSettings);
        if (!result.success) {
            if (!m_loggedStartFailure) {
                kLogger.warning() << "Rejecting REST API configuration:" << result.error
                                  << result.tlsError;
                m_loggedStartFailure = true;
            }
            publishStartFailure(result.error, result.tlsError, serverRunning || httpWasRunning || httpsWasRunning);
            return std::nullopt;
        }
        return result;
    };

    std::optional<RestServerValidationResult> httpValidation;
    std::optional<RestServerValidationResult> httpsValidation;
    if (settings.enableHttp) {
        httpValidation = validateServer(settings.httpSettings, m_activeHttpSettings, httpWasRunning, nullptr);
        if (!httpValidation.has_value()) {
            return;
        }
    }
    if (settings.enableHttps) {
        httpsValidation = validateServer(settings.httpsSettings, m_activeHttpsSettings, httpsWasRunning, &m_certificateGenerator);
        if (!httpsValidation.has_value()) {
            return;
        }
    }

    bool httpRunning = false;
    bool httpsRunning = false;
    QString lastError;
    QString tlsError;

    auto attemptRestartPrevious = [&](RestServer* server,
                                      const RestServer::Settings& previousSettings,
                                      const std::optional<RestServer::TlsResult>& previousTls,
                                      QString* errorOut) {
        if (!server || !previousSettings.enabled) {
            return false;
        }
        QString restartError;
        if (server->start(previousSettings, previousTls, &restartError)) {
            if (errorOut) {
                *errorOut = QString();
            }
            return true;
        }
        if (errorOut) {
            *errorOut = restartError;
        }
        return false;
    };

    auto updateStatus = [&](bool runningHttp, bool runningHttps, bool generatedCertificate) {
        RestServerSettings::Status status;
        status.running = runningHttp || runningHttps;
        status.tlsActive = runningHttps;
        status.certificateGenerated = generatedCertificate && runningHttps;
        status.lastError = lastError;
        status.tlsError = tlsError;
        publishStatus(status);
    };

    if (!settings.enableHttp) {
        if (httpWasRunning) {
            m_httpServer->stop();
        }
        m_activeHttpSettings = RestServer::Settings{};
    }

    if (!settings.enableHttps) {
        if (httpsWasRunning) {
            m_httpsServer->stop();
        }
        m_activeHttpsSettings = RestServer::Settings{};
        m_lastHttpsTlsConfiguration.reset();
    }

    if (settings.enableHttp) {
        RestServer::Settings httpSettingsToApply = httpValidation->settings;
        if (httpWasRunning && httpSettingsToApply == m_activeHttpSettings) {
            httpRunning = true;
        } else {
            if (httpWasRunning) {
                m_httpServer->stop();
            }
            QString startError;
            if (m_httpServer->start(httpSettingsToApply, std::nullopt, &startError)) {
                httpRunning = true;
                m_activeHttpSettings = httpSettingsToApply;
            } else {
                lastError = startError.isEmpty() ? tr("Failed to start REST API server") : startError;
                if (!m_loggedStartFailure) {
                    kLogger.warning() << "Failed to start REST API HTTP server:" << startError;
                    m_loggedStartFailure = true;
                }
                QString restartError;
                if (httpWasRunning && attemptRestartPrevious(m_httpServer.get(), previousHttpSettings, std::nullopt, &restartError)) {
                    httpRunning = true;
                    m_activeHttpSettings = previousHttpSettings;
                } else if (!restartError.isEmpty()) {
                    lastError = restartError;
                }
            }
        }
    }

    if (settings.enableHttps) {
        RestServer::Settings httpsSettingsToApply = httpsValidation->settings;
        std::optional<RestServer::TlsResult> tlsConfiguration = httpsValidation->tlsConfiguration;
        if (httpsWasRunning && httpsSettingsToApply == m_activeHttpsSettings) {
            httpsRunning = true;
        } else {
            if (httpsWasRunning) {
                m_httpsServer->stop();
            }
            QString startError;
            if (m_httpsServer->start(httpsSettingsToApply, tlsConfiguration, &startError)) {
                httpsRunning = true;
                m_activeHttpsSettings = httpsSettingsToApply;
                m_lastHttpsTlsConfiguration = tlsConfiguration;
                if (httpsSettingsToApply.autoGenerateCertificate) {
                    RestServerSettings::Values storedValues = m_settingsStore.get();
                    storedValues.certificatePath = httpsSettingsToApply.certificatePath;
                    storedValues.privateKeyPath = httpsSettingsToApply.privateKeyPath;
                    m_settingsStore.set(storedValues);
                }
            } else {
                lastError = startError.isEmpty() ? tr("Failed to start REST API server") : startError;
                tlsError = httpsValidation->tlsError;
                if (!m_loggedStartFailure) {
                    kLogger.warning() << "Failed to start REST API HTTPS server:" << startError;
                    m_loggedStartFailure = true;
                }
                QString restartError;
                if (httpsWasRunning && attemptRestartPrevious(m_httpsServer.get(), previousHttpsSettings, previousTlsConfiguration, &restartError)) {
                    httpsRunning = true;
                    m_activeHttpsSettings = previousHttpsSettings;
                    m_lastHttpsTlsConfiguration = previousTlsConfiguration;
                } else if (!restartError.isEmpty()) {
                    lastError = restartError;
                }
            }
        }
    }

    const bool generatedCertificate = m_lastHttpsTlsConfiguration.has_value() &&
            m_lastHttpsTlsConfiguration->generated;
    updateStatus(httpRunning, httpsRunning, generatedCertificate);
}

void RestServerController::start() {
    refreshFromSettings();
    m_reloadTimer.start();
}

void RestServerController::shutdown() {
    m_reloadTimer.stop();
    if (m_httpServer) {
        m_httpServer->stop();
    }
    if (m_httpsServer) {
        m_httpsServer->stop();
    }
}

void RestServerController::refreshFromSettings() {
    applySettings(loadSettings());
}

} // namespace mixxx::network::rest

#endif // MIXXX_HAS_HTTP_SERVER
