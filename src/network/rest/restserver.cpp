#include "network/rest/restserver.h"

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaObject>
#include <QSemaphore>
#include <QSslSocket>
#include <QUrl>
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

QString methodToString(QHttpServerRequest::Method method) {
    switch (method) {
    case QHttpServerRequest::Method::Get:
        return QStringLiteral("GET");
    case QHttpServerRequest::Method::Put:
        return QStringLiteral("PUT");
    case QHttpServerRequest::Method::Post:
        return QStringLiteral("POST");
    case QHttpServerRequest::Method::Delete:
        return QStringLiteral("DELETE");
    case QHttpServerRequest::Method::Patch:
        return QStringLiteral("PATCH");
    default:
        return QStringLiteral("OTHER");
    }
}
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
    kLogger.info() << "Starting REST API server";
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

    kLogger.info() << "Stopping REST API server";

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
        kLogger.warning() << "REST API TLS preparation failed:" << certificateResult.error;
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

    if (certificateResult.generated) {
        kLogger.info() << "Generated REST API TLS certificate at"
                       << certificateResult.certificatePath
                       << "and key at" << certificateResult.privateKeyPath;
    } else {
        kLogger.info() << "Loaded REST API TLS certificate from"
                       << certificateResult.certificatePath
                       << "and key from" << certificateResult.privateKeyPath;
    }
#else
    Q_UNUSED(settings);
    Q_UNUSED(certificateGenerator);
    kLogger.warning() << "REST API TLS preparation failed: Qt is built without SSL support";
    result.error = QObject::tr("Qt is built without SSL support");
#endif
    return result;
}

QHttpServerResponse RestServer::tlsRequiredResponse(const QHttpServerRequest& request) const {
    const QString message = QStringLiteral("TLS is required for this route");
    logRouteError(request, QHttpServerResponse::StatusCode::Forbidden, message);
    return jsonResponse(QJsonObject{{"error", message}}, QHttpServerResponse::StatusCode::Forbidden);
}

QHttpServerResponse RestServer::unauthorizedResponse(const QHttpServerRequest& request) const {
    const QString message = QStringLiteral("Unauthorized");
    logRouteError(request, QHttpServerResponse::StatusCode::Unauthorized, message);
    return jsonResponse(QJsonObject{{"error", message}}, QHttpServerResponse::StatusCode::Unauthorized);
}

QHttpServerResponse RestServer::forbiddenResponse(
        const QHttpServerRequest& request, const QString& message) const {
    logRouteError(request, QHttpServerResponse::StatusCode::Forbidden, message);
    return jsonResponse(QJsonObject{{"error", message}}, QHttpServerResponse::StatusCode::Forbidden);
}

QHttpServerResponse RestServer::badRequestResponse(
        const QHttpServerRequest& request, const QString& message) const {
    logRouteError(request, QHttpServerResponse::StatusCode::BadRequest, message);
    return jsonResponse(QJsonObject{{"error", message}}, QHttpServerResponse::StatusCode::BadRequest);
}

QHttpServerResponse RestServer::methodNotAllowedResponse(const QHttpServerRequest& request) const {
    const QString message = QStringLiteral("Method not allowed");
    logRouteError(request, QHttpServerResponse::StatusCode::MethodNotAllowed, message);
    return jsonResponse(QJsonObject{{"error", message}}, QHttpServerResponse::StatusCode::MethodNotAllowed);
}

QHttpServerResponse RestServer::serviceUnavailableResponse(const QHttpServerRequest* request) const {
    const QString message = QStringLiteral("REST gateway unavailable");
    if (request != nullptr) {
        logRouteError(*request, QHttpServerResponse::StatusCode::ServiceUnavailable, message);
    }
    return jsonResponse(QJsonObject{{"error", message}}, QHttpServerResponse::StatusCode::ServiceUnavailable);
}

QHttpServerResponse RestServer::invokeGateway(
        const QHttpServerRequest& request,
        const std::function<QHttpServerResponse()>& action) const {
    if (m_gateway.isNull()) {
        return serviceUnavailableResponse(&request);
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

RestServer::AuthorizationResult RestServer::authorize(
        const QHttpServerRequest& request, AccessPolicy policy) const {
    AuthorizationResult result;

    const bool authDisabled = m_settings.tokens.isEmpty();
    if (authDisabled) {
        result.authorized = true;
        return result;
    }

    const QByteArray headerValue = request.value(kAuthHeader).trimmed();
    if (headerValue.isEmpty()) {
        return result;
    }

    const QByteArray bearerPrefix = QByteArrayLiteral("Bearer ");
    const QByteArray tokenValue = headerValue.startsWith(bearerPrefix)
            ? headerValue.mid(bearerPrefix.size())
            : headerValue;

    const QString requiredPermission =
            policy == AccessPolicy::Status ? QStringLiteral("read") : QStringLiteral("full");
    const QDateTime nowUtc = QDateTime::currentDateTimeUtc();

    for (const auto& token : m_settings.tokens) {
        if (token.value.isEmpty()) {
            continue;
        }
        if (token.expiresUtc.has_value() && token.expiresUtc.value() < nowUtc) {
            continue;
        }
        if (token.value.toUtf8() != tokenValue) {
            continue;
        }

        const bool hasFull = token.permission.compare(QStringLiteral("full"), Qt::CaseInsensitive) == 0;
        const bool hasRead = token.permission.compare(QStringLiteral("read"), Qt::CaseInsensitive) == 0;

        if (requiredPermission == QStringLiteral("read") && (hasRead || hasFull)) {
            result.authorized = true;
            result.usedReadOnlyToken = !hasFull;
            return result;
        }
        if (requiredPermission == QStringLiteral("full") && hasFull) {
            result.authorized = true;
            return result;
        }
        result.forbidden = true;
        return result;
    }
    return result;
}

bool RestServer::controlRouteRequiresTls(const QHttpServerRequest& request) const {
    if (!m_settings.requireTls) {
        return false;
    }

    bool isSecure = false;
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    isSecure = request.isSecure();
#else
    isSecure = request.url().scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) == 0;
#endif
    return !isSecure;
}

QString RestServer::requestDescription(const QHttpServerRequest& request) const {
    return QStringLiteral("%1 %2")
            .arg(methodToString(request.method()), request.url().path());
}

void RestServer::logRouteError(
        const QHttpServerRequest& request,
        QHttpServerResponse::StatusCode status,
        const QString& message) const {
    const QString key = QStringLiteral("%1 %2")
                                .arg(requestDescription(request),
                                        QString::number(static_cast<int>(status)));
    m_routeErrorLogger.log(key, [&](int suppressed) {
        auto stream = kLogger.warning();
        stream << "REST route error" << requestDescription(request)
               << static_cast<int>(status) << message;
        if (suppressed > 0) {
            stream << "(suppressed" << suppressed << "similar messages)";
        }
    });
}

void RestServer::registerRoutes() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(m_httpServer.get());

    const auto healthRoute = [this](const QHttpServerRequest& request) {
        const AuthorizationResult auth = authorize(request, AccessPolicy::Status);
        if (!auth.authorized) {
            return unauthorizedResponse(request);
        }
        if (request.method() != QHttpServerRequest::Method::Get) {
            return methodNotAllowedResponse(request);
        }
        return invokeGateway(request, [this]() {
            return m_gateway->health();
        });
    };
    m_httpServer->route("/health", healthRoute);
    m_httpServer->route("/api/health", healthRoute);

    const auto readyRoute = [this](const QHttpServerRequest& request) {
        const AuthorizationResult auth = authorize(request, AccessPolicy::Status);
        if (!auth.authorized) {
            return unauthorizedResponse(request);
        }
        if (request.method() != QHttpServerRequest::Method::Get) {
            return methodNotAllowedResponse(request);
        }
        return invokeGateway(request, [this]() {
            return m_gateway->ready();
        });
    };
    m_httpServer->route("/ready", readyRoute);
    m_httpServer->route("/api/ready", readyRoute);

    const auto statusRoute = [this](const QHttpServerRequest& request) {
        const AuthorizationResult auth = authorize(request, AccessPolicy::Status);
        if (!auth.authorized) {
            return unauthorizedResponse(request);
        }
        if (request.method() != QHttpServerRequest::Method::Get) {
            return methodNotAllowedResponse(request);
        }
        return invokeGateway(request, [this]() {
            return m_gateway->status();
        });
    };
    m_httpServer->route("/status", statusRoute);
    m_httpServer->route("/api/status", statusRoute);

    const auto decksRoute = [this](const QHttpServerRequest& request) {
        const AuthorizationResult auth = authorize(request, AccessPolicy::Status);
        if (!auth.authorized) {
            return unauthorizedResponse(request);
        }
        if (request.method() != QHttpServerRequest::Method::Get) {
            return methodNotAllowedResponse(request);
        }
        return invokeGateway(request, [this]() {
            return m_gateway->decks();
        });
    };
    m_httpServer->route("/decks", decksRoute);
    m_httpServer->route("/api/decks", decksRoute);

    const auto deckRoute = [this](const QHttpServerRequest& request, int deckNumber) {
        const AuthorizationResult auth = authorize(request, AccessPolicy::Status);
        if (!auth.authorized) {
            return unauthorizedResponse(request);
        }
        if (request.method() != QHttpServerRequest::Method::Get) {
            return methodNotAllowedResponse(request);
        }
        return invokeGateway(request, [this, deckNumber]() {
            return m_gateway->deck(deckNumber);
        });
    };
    m_httpServer->route("/decks/<int>", deckRoute);
    m_httpServer->route("/api/decks/<int>", deckRoute);

    const auto controlRoute = [this](const QHttpServerRequest& request) {
        const AuthorizationResult auth = authorize(request, AccessPolicy::Control);
        if (!auth.authorized) {
            if (auth.forbidden) {
                return forbiddenResponse(request, tr("Full access token required for this endpoint"));
            }
            return unauthorizedResponse(request);
        }
        if (request.method() != QHttpServerRequest::Method::Post) {
            return methodNotAllowedResponse(request);
        }
        if (controlRouteRequiresTls(request)) {
            return tlsRequiredResponse(request);
        }

        const auto document = QJsonDocument::fromJson(request.body());
        if (!document.isObject()) {
            return badRequestResponse(request, QStringLiteral("Expected JSON request body"));
        }
        const auto body = document.object();
        return invokeGateway(request, [this, body]() {
            return m_gateway->control(body);
        });
    };
    m_httpServer->route("/control", controlRoute);
    m_httpServer->route("/api/control", controlRoute);

    const auto autoDjRoute = [this](const QHttpServerRequest& request) {
        const AuthorizationResult auth = authorize(
                request,
                request.method() == QHttpServerRequest::Method::Get ? AccessPolicy::Status
                                                                    : AccessPolicy::Control);
        if (!auth.authorized) {
            if (auth.forbidden) {
                return forbiddenResponse(request, tr("Full access token required for this endpoint"));
            }
            return unauthorizedResponse(request);
        }
        if (request.method() == QHttpServerRequest::Method::Get) {
            return invokeGateway(request, [this]() {
                return m_gateway->autoDjStatus();
            });
        }
        if (request.method() != QHttpServerRequest::Method::Post) {
            return methodNotAllowedResponse(request);
        }
        if (controlRouteRequiresTls(request)) {
            return tlsRequiredResponse(request);
        }

        const auto document = QJsonDocument::fromJson(request.body());
        if (!document.isObject()) {
            return badRequestResponse(request, QStringLiteral("Expected JSON request body"));
        }
        const auto body = document.object();
        return invokeGateway(request, [this, body]() {
            return m_gateway->autoDj(body);
        });
    };
    m_httpServer->route("/autodj", autoDjRoute);
    m_httpServer->route("/api/autodj", autoDjRoute);

    const auto playlistsRoute = [this](const QHttpServerRequest& request) {
        const AuthorizationResult auth = authorize(
                request,
                request.method() == QHttpServerRequest::Method::Get ? AccessPolicy::Status
                                                                    : AccessPolicy::Control);
        if (!auth.authorized) {
            if (auth.forbidden) {
                return forbiddenResponse(request, tr("Full access token required for this endpoint"));
            }
            return unauthorizedResponse(request);
        }
        if (request.method() == QHttpServerRequest::Method::Get) {
            std::optional<int> playlistId;
            const QString playlistIdParam = request.query().queryItemValue("id");
            if (!playlistIdParam.isEmpty()) {
                bool ok = false;
                const int idValue = playlistIdParam.toInt(&ok);
                if (!ok) {
                    return badRequestResponse(request, QStringLiteral("Playlist id must be numeric"));
                }
                playlistId = idValue;
            }
            return invokeGateway(request, [this, playlistId]() {
                return m_gateway->playlists(playlistId);
            });
        }

        if (request.method() != QHttpServerRequest::Method::Post) {
            return methodNotAllowedResponse(request);
        }

        if (controlRouteRequiresTls(request)) {
            return tlsRequiredResponse(request);
        }

        const auto document = QJsonDocument::fromJson(request.body());
        if (!document.isObject()) {
            return badRequestResponse(request, QStringLiteral("Expected JSON request body"));
        }
        const auto body = document.object();
        return invokeGateway(request, [this, body]() {
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

    const bool authEnabled = !m_settings.tokens.isEmpty();
    if (authEnabled && !m_settings.useHttps) {
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
    kLogger.info() << "REST API server stopped";
}

} // namespace mixxx::network::rest

#endif // MIXXX_HAS_HTTP_SERVER
