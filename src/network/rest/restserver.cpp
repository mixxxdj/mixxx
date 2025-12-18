#include "network/rest/restserver.h"

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaObject>
#include <QSemaphore>
#include <QSslSocket>
#include <QUrlQuery>
#include <utility>

#include "moc_restserver.cpp"
#include "network/rest/restapigateway.h"
#include "network/rest/certificategenerator.h"
#include "util/logger.h"
#include "util/thread_affinity.h"

namespace mixxx::network::rest {

namespace {
constexpr auto kAuthHeader = "Authorization";
} // namespace

const Logger RestServer::kLogger("mixxx::network::rest::RestServer");

RestServer::RestServer(RestApiProvider* gateway, QObject* parent)
        : QObject(parent),
          m_gateway(gateway),
          m_isRunning(false),
          m_tlsActive(false),
          m_listeningPort(0) {
    DEBUG_ASSERT(m_gateway);
}

RestServer::~RestServer() {
    stop();
}

bool RestServer::start(
        const Settings& settings,
        const std::optional<TlsResult>& tlsConfiguration,
        QString* error) {
    if (m_thread.isRunning()) {
        stop();
    }
    m_lastError.clear();

    if (m_gateway.isNull()) {
        kLogger.warning() << "REST API gateway is not available; aborting startup";
        if (error) {
            *error = tr("REST API gateway is not available");
        }
        return false;
    }

    m_settings = settings;
    m_tlsConfiguration = tlsConfiguration;
    m_tlsActive = false;

    m_threadObject = std::make_unique<QObject>();
    m_threadObject->moveToThread(&m_thread);
    m_thread.start();

    bool success = false;
    QSemaphore semaphore;
    QMetaObject::invokeMethod(m_threadObject.get(), [this, &success, &semaphore]() {
        success = startOnThread();
        semaphore.release();
    });
    semaphore.acquire();

    if (!success) {
        stop();
        if (error) {
            *error = m_lastError.isEmpty() ? tr("Failed to start REST server") : m_lastError;
        }
        return false;
    }

    m_isRunning = true;
    emit started(m_listeningPort);
    return true;
}

void RestServer::stop() {
    if (!m_thread.isRunning()) {
        return;
    }

    QSemaphore semaphore;
    QMetaObject::invokeMethod(m_threadObject.get(), [this, &semaphore]() {
        stopOnThread();
        semaphore.release();
    });
    semaphore.acquire();

    m_thread.quit();
    m_thread.wait();

    m_threadObject.reset();
    m_tlsConfiguration.reset();
    m_isRunning = false;
    m_tlsActive = false;
    m_listeningPort = 0;
    emit stopped();
}

QHttpServerResponse RestServer::jsonResponse(
        const QJsonObject& body,
        QHttpServerResponse::StatusCode status) const {
    return QHttpServerResponse(
            status,
            QJsonDocument(body).toJson(QJsonDocument::Compact),
            "application/json");
}

RestServer::TlsResult RestServer::prepareTlsConfiguration(
        const Settings& settings, CertificateGenerator* certificateGenerator) {
    TlsResult result;
    if (!settings.useHttps) {
        result.success = true;
        return result;
    }

#if QT_CONFIG(ssl)
    if (certificateGenerator == nullptr) {
        result.error = QObject::tr("Certificate generator is unavailable");
        return result;
    }
    const CertificateGenerator::Result certificateResult = certificateGenerator->loadOrGenerate(
            settings.certificatePath,
            settings.privateKeyPath,
            settings.autoGenerateCertificate);
    result.certificatePath = certificateResult.certificatePath;
    result.privateKeyPath = certificateResult.privateKeyPath;
    if (!certificateResult.success) {
        result.error = certificateResult.error;
        return result;
    }

    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setLocalCertificate(certificateResult.certificate);
    sslConfig.setPrivateKey(certificateResult.privateKey);
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);

    result.configuration = sslConfig;
    result.generated = certificateResult.generated;
    result.success = true;
#else
    Q_UNUSED(settings);
    Q_UNUSED(certificateGenerator);
    result.error = QObject::tr("Qt is built without SSL support");
#endif
    return result;
}

QHttpServerResponse RestServer::tlsRequiredResponse() const {
    return jsonResponse(
            QJsonObject{{"error", "TLS is required for this route"}},
            QHttpServerResponse::StatusCode::Forbidden);
}

QHttpServerResponse RestServer::unauthorizedResponse() const {
    return jsonResponse(
            QJsonObject{{"error", "Unauthorized"}},
            QHttpServerResponse::StatusCode::Unauthorized);
}

QHttpServerResponse RestServer::badRequestResponse(const QString& message) const {
    return jsonResponse(
            QJsonObject{{"error", message}},
            QHttpServerResponse::StatusCode::BadRequest);
}

QHttpServerResponse RestServer::methodNotAllowedResponse() const {
    return jsonResponse(
            QJsonObject{{"error", "Method not allowed"}},
            QHttpServerResponse::StatusCode::MethodNotAllowed);
}

QHttpServerResponse RestServer::serviceUnavailableResponse() const {
    return jsonResponse(
            QJsonObject{{"error", "REST gateway unavailable"}},
            QHttpServerResponse::StatusCode::ServiceUnavailable);
}

QHttpServerResponse RestServer::invokeGateway(
        const std::function<QHttpServerResponse()>& action) const {
    if (m_gateway.isNull()) {
        return serviceUnavailableResponse();
    }

    QHttpServerResponse response(QHttpServerResponse::StatusCode::InternalServerError);
    QSemaphore semaphore;
    QMetaObject::invokeMethod(
            m_gateway,
            [&]() {
                response = action();
                semaphore.release();
            },
            Qt::QueuedConnection);
    semaphore.acquire();
    return response;
}

bool RestServer::applyTlsConfiguration() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(m_httpServer.get());

#if QT_CONFIG(ssl)
    if (!m_tlsConfiguration.has_value()) {
        kLogger.warning() << "TLS is enabled but no TLS configuration was provided";
        return false;
    }
    m_httpServer->sslSetup(m_tlsConfiguration->configuration);
    m_tlsActive = true;
    return true;
#else
    Q_UNUSED(m_settings);
    kLogger.warning() << "TLS requested for REST API but Qt is built without SSL support";
    return false;
#endif
}

bool RestServer::checkAuthorization(const QHttpServerRequest& request) const {
    if (m_settings.authToken.isEmpty()) {
        return true;
    }
    const auto headerValue = request.value(kAuthHeader);
    if (headerValue.isEmpty()) {
        return false;
    }
    const QByteArray expected = "Bearer " + m_settings.authToken.toUtf8();
    return headerValue.trimmed() == expected;
}

bool RestServer::controlRouteRequiresTls(const QHttpServerRequest& /*request*/) const {
    return m_settings.requireTls && !m_tlsActive;
}

void RestServer::registerRoutes() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(m_httpServer.get());

    const auto healthRoute = [this](const QHttpServerRequest& request) {
        if (!checkAuthorization(request)) {
            return unauthorizedResponse();
        }
        if (request.method() != QHttpServerRequest::Method::Get) {
            return methodNotAllowedResponse();
        }
        return invokeGateway([this]() {
            return m_gateway->health();
        });
    };
    m_httpServer->route("/health", healthRoute);
    m_httpServer->route("/api/health", healthRoute);

    const auto statusRoute = [this](const QHttpServerRequest& request) {
        if (!checkAuthorization(request)) {
            return unauthorizedResponse();
        }
        if (request.method() != QHttpServerRequest::Method::Get) {
            return methodNotAllowedResponse();
        }
        return invokeGateway([this]() {
            return m_gateway->status();
        });
    };
    m_httpServer->route("/status", statusRoute);
    m_httpServer->route("/api/status", statusRoute);

    const auto controlRoute = [this](const QHttpServerRequest& request) {
        if (!checkAuthorization(request)) {
            return unauthorizedResponse();
        }
        if (request.method() != QHttpServerRequest::Method::Post) {
            return methodNotAllowedResponse();
        }
        if (controlRouteRequiresTls(request)) {
            return tlsRequiredResponse();
        }

        const auto document = QJsonDocument::fromJson(request.body());
        if (!document.isObject()) {
            return badRequestResponse(QStringLiteral("Expected JSON request body"));
        }
        const auto body = document.object();
        return invokeGateway([this, body]() {
            return m_gateway->control(body);
        });
    };
    m_httpServer->route("/control", controlRoute);
    m_httpServer->route("/api/control", controlRoute);

    const auto autoDjRoute = [this](const QHttpServerRequest& request) {
        if (!checkAuthorization(request)) {
            return unauthorizedResponse();
        }
        if (request.method() == QHttpServerRequest::Method::Get) {
            return invokeGateway([this]() {
                return m_gateway->autoDjStatus();
            });
        }
        if (request.method() != QHttpServerRequest::Method::Post) {
            return methodNotAllowedResponse();
        }
        if (controlRouteRequiresTls(request)) {
            return tlsRequiredResponse();
        }

        const auto document = QJsonDocument::fromJson(request.body());
        if (!document.isObject()) {
            return badRequestResponse(QStringLiteral("Expected JSON request body"));
        }
        const auto body = document.object();
        return invokeGateway([this, body]() {
            return m_gateway->autoDj(body);
        });
    };
    m_httpServer->route("/autodj", autoDjRoute);
    m_httpServer->route("/api/autodj", autoDjRoute);

    const auto playlistsRoute = [this](const QHttpServerRequest& request) {
        if (!checkAuthorization(request)) {
            return unauthorizedResponse();
        }
        if (request.method() == QHttpServerRequest::Method::Get) {
            std::optional<int> playlistId;
            const QString playlistIdParam = request.query().queryItemValue("id");
            if (!playlistIdParam.isEmpty()) {
                bool ok = false;
                const int idValue = playlistIdParam.toInt(&ok);
                if (!ok) {
                    return badRequestResponse(QStringLiteral("Playlist id must be numeric"));
                }
                playlistId = idValue;
            }
            return invokeGateway([this, playlistId]() {
                return m_gateway->playlists(playlistId);
            });
        }

        if (request.method() != QHttpServerRequest::Method::Post) {
            return methodNotAllowedResponse();
        }

        if (controlRouteRequiresTls(request)) {
            return tlsRequiredResponse();
        }

        const auto document = QJsonDocument::fromJson(request.body());
        if (!document.isObject()) {
            return badRequestResponse(QStringLiteral("Expected JSON request body"));
        }
        const auto body = document.object();
        return invokeGateway([this, body]() {
            return m_gateway->playlistCommand(body);
        });
    };
    m_httpServer->route("/playlists", playlistsRoute);
    m_httpServer->route("/api/playlists", playlistsRoute);
}

bool RestServer::startOnThread() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(m_threadObject.get());

    m_httpServer = std::make_unique<QHttpServer>();
    registerRoutes();

    if (!m_settings.authToken.isEmpty() && !m_settings.useHttps) {
        kLogger.warning() << "REST API authentication is enabled without TLS";
    }

    if (m_settings.requireTls && !m_settings.useHttps) {
        kLogger.warning() << "REST API control routes require TLS but HTTPS is disabled";
    }

    if (m_settings.useHttps && !applyTlsConfiguration()) {
        m_lastError = tr("Failed to configure TLS for REST API");
        return false;
    }

    DEBUG_ASSERT(m_settings.portValid);
    const auto port = m_httpServer->listen(
            m_settings.address, static_cast<quint16>(m_settings.port));
    if (port == 0) {
        kLogger.warning()
                << "Failed to start REST API listener on" << m_settings.address << m_settings.port;
        m_lastError = tr("Failed to bind REST API listener");
        return false;
    }

    m_listeningPort = port;
    kLogger.info()
            << "REST API listening on" << m_settings.address.toString() << m_listeningPort;
    return true;
}

void RestServer::stopOnThread() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(m_threadObject.get());
    if (m_httpServer) {
        m_httpServer->close();
        m_httpServer.reset();
    }
}

} // namespace mixxx::network::rest

#endif // MIXXX_HAS_HTTP_SERVER
