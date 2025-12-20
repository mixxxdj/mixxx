#include "network/rest/restserver.h"

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QCoreApplication>
#include <QFile>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMetaObject>
#include <QSemaphore>
#include <QSslSocket>
#include <QUuid>
#include <QUrl>
#include <QUrlQuery>
#include <utility>

#include "moc_restserver.cpp"
#include "network/rest/restapigateway.h"
#include "network/rest/certificategenerator.h"
#include "network/rest/restscopes.h"
#include "util/logger.h"
#include "util/thread_affinity.h"

namespace mixxx::network::rest {

namespace {
constexpr auto kAuthHeader = "Authorization";
constexpr auto kContentTypeHeader = "Content-Type";
constexpr auto kEventStreamContentType = "text/event-stream";
constexpr auto kIdempotencyKeyHeader = "Idempotency-Key";
constexpr auto kRequestIdHeader = "X-Request-Id";
constexpr auto kCorsOriginHeader = "Origin";
constexpr auto kCorsAllowOriginHeader = "Access-Control-Allow-Origin";
constexpr auto kCorsAllowMethodsHeader = "Access-Control-Allow-Methods";
constexpr auto kCorsAllowHeadersHeader = "Access-Control-Allow-Headers";
constexpr auto kCorsRequestHeadersHeader = "Access-Control-Request-Headers";
constexpr auto kCacheControlHeader = "Cache-Control";
constexpr auto kConnectionHeader = "Connection";
constexpr auto kNoCacheValue = "no-cache";
constexpr auto kKeepAliveValue = "keep-alive";
constexpr auto kAccelBufferingHeader = "X-Accel-Buffering";
constexpr auto kAccelBufferingDisabled = "no";

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

bool isJsonContentType(const QHttpServerRequest& request) {
    const QByteArray headerValue =
            request.headers().value(QByteArrayLiteral(kContentTypeHeader)).trimmed();
    if (headerValue.isEmpty()) {
        return false;
    }
    const int separatorIndex = headerValue.indexOf(';');
    const QByteArray mediaType = (separatorIndex >= 0 ? headerValue.left(separatorIndex)
                                                      : headerValue)
                                         .trimmed()
                                         .toLower();
    return mediaType == QByteArrayLiteral("application/json");
}

QByteArray formatSseEvent(const QJsonObject& payload) {
    const QByteArray body = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    QByteArray event;
    event.reserve(body.size() + 32);
    event.append("event: status\n");
    event.append("data: ");
    event.append(body);
    event.append("\n\n");
    return event;
}
} // namespace

const Logger RestServer::kLogger("mixxx::network::rest::RestServer");
const Logger RestServer::kAuditLogger("mixxx::network::rest::RestServer::Audit");

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
        const QHttpServerRequest& request,
        const QJsonObject& body,
        QHttpServerResponse::StatusCode status,
        const QString& requestId) const {
    QHttpServerResponse response(
            status,
            QJsonDocument(body).toJson(QJsonDocument::Compact),
            "application/json");
    if (!requestId.isEmpty()) {
        response.setHeader(QByteArrayLiteral(kRequestIdHeader), requestId.toUtf8());
    }
    addCorsHeaders(&response, request, false);
    return response;
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
    const QString requestId = requestIdFor(request);
    logRouteError(request, QHttpServerResponse::StatusCode::Forbidden, message, requestId);
    return jsonResponse(request, QJsonObject{{"error", message}},
            QHttpServerResponse::StatusCode::Forbidden,
            requestId);
}

QHttpServerResponse RestServer::unauthorizedResponse(const QHttpServerRequest& request) const {
    const QString message = QStringLiteral("Unauthorized");
    const QString requestId = requestIdFor(request);
    logRouteError(request, QHttpServerResponse::StatusCode::Unauthorized, message, requestId);
    return jsonResponse(request, QJsonObject{{"error", message}},
            QHttpServerResponse::StatusCode::Unauthorized,
            requestId);
}

QHttpServerResponse RestServer::forbiddenResponse(
        const QHttpServerRequest& request, const QString& message) const {
    const QString requestId = requestIdFor(request);
    logRouteError(request, QHttpServerResponse::StatusCode::Forbidden, message, requestId);
    return jsonResponse(request, QJsonObject{{"error", message}},
            QHttpServerResponse::StatusCode::Forbidden,
            requestId);
}

QHttpServerResponse RestServer::badRequestResponse(
        const QHttpServerRequest& request, const QString& message) const {
    const QString requestId = requestIdFor(request);
    logRouteError(request, QHttpServerResponse::StatusCode::BadRequest, message, requestId);
    return jsonResponse(request, QJsonObject{{"error", message}},
            QHttpServerResponse::StatusCode::BadRequest,
            requestId);
}

QHttpServerResponse RestServer::unsupportedMediaTypeResponse(
        const QHttpServerRequest& request, const QString& message) const {
    const QString requestId = requestIdFor(request);
    logRouteError(request, QHttpServerResponse::StatusCode::UnsupportedMediaType, message, requestId);
    return jsonResponse(request, QJsonObject{{"error", message}},
            QHttpServerResponse::StatusCode::UnsupportedMediaType,
            requestId);
}

QHttpServerResponse RestServer::payloadTooLargeResponse(const QHttpServerRequest& request) const {
    const QString message =
            tr("Request payload exceeds the maximum size of %1 bytes").arg(m_settings.maxRequestBytes);
    const QString requestId = requestIdFor(request);
    logRouteError(request, QHttpServerResponse::StatusCode::PayloadTooLarge, message, requestId);
    return jsonResponse(request, QJsonObject{{"error", message}},
            QHttpServerResponse::StatusCode::PayloadTooLarge,
            requestId);
}

QHttpServerResponse RestServer::methodNotAllowedResponse(const QHttpServerRequest& request) const {
    const QString message = QStringLiteral("Method not allowed");
    const QString requestId = requestIdFor(request);
    logRouteError(request, QHttpServerResponse::StatusCode::MethodNotAllowed, message, requestId);
    return jsonResponse(request, QJsonObject{{"error", message}},
            QHttpServerResponse::StatusCode::MethodNotAllowed,
            requestId);
}

QHttpServerResponse RestServer::serviceUnavailableResponse(const QHttpServerRequest* request) const {
    const QString message = QStringLiteral("REST gateway unavailable");
    if (request != nullptr) {
        const QString requestId = requestIdFor(*request);
        logRouteError(*request, QHttpServerResponse::StatusCode::ServiceUnavailable, message, requestId);
        return jsonResponse(*request, QJsonObject{{"error", message}},
                QHttpServerResponse::StatusCode::ServiceUnavailable,
                requestId);
    }
    return QHttpServerResponse(
            QHttpServerResponse::StatusCode::ServiceUnavailable,
            QJsonDocument(QJsonObject{{"error", message}}).toJson(QJsonDocument::Compact),
            "application/json");
}

QHttpServerResponse RestServer::invokeGateway(
        const QHttpServerRequest& request,
        const std::function<QHttpServerResponse()>& action,
        const QString& requestIdOverride) const {
    if (m_gateway.isNull()) {
        return serviceUnavailableResponse(&request);
    }

    const QString requestId =
            requestIdOverride.isEmpty() ? requestIdFor(request) : requestIdOverride;
    kLogger.info() << "REST audit"
                   << requestDescription(request)
                   << "requestId" << requestId;

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
    if (!requestId.isEmpty()) {
        response.setHeader(QByteArrayLiteral(kRequestIdHeader), requestId.toUtf8());
    }
    addCorsHeaders(&response, request, false);
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
        const QHttpServerRequest& request,
        const QStringList& requiredScopes) const {
    AuthorizationResult result;

    if (m_settings.tokens.isEmpty()) {
        result.authorized = m_settings.allowUnauthenticated;
        return result;
    }

    const QString requestId = requestIdFor(request);
    const QByteArray headerValue = request.value(kAuthHeader).trimmed();
    if (headerValue.isEmpty()) {
        const QString key = QStringLiteral("%1 missing-authorization")
                                    .arg(requestDescription(request));
        m_authFailureLogger.log(key, [&](int suppressed) {
            auto stream = kAuditLogger.warning();
            stream << "REST auth failure" << requestDescription(request)
                   << "scopes" << requiredScopes.join(QStringLiteral(","))
                   << "reason"
                   << QStringLiteral("missing authorization header")
                   << "requestId" << requestId;
            if (suppressed > 0) {
                stream << "(suppressed" << suppressed << "similar messages)";
            }
        });
        return result;
    }

    const QByteArray bearerPrefix = QByteArrayLiteral("Bearer ");
    const QByteArray tokenValue = headerValue.startsWith(bearerPrefix)
            ? headerValue.mid(bearerPrefix.size())
            : headerValue;
    if (tokenValue.isEmpty()) {
        const QString key = QStringLiteral("%1 empty-token").arg(requestDescription(request));
        m_authFailureLogger.log(key, [&](int suppressed) {
            auto stream = kAuditLogger.warning();
            stream << "REST auth failure" << requestDescription(request)
                   << "scopes" << requiredScopes.join(QStringLiteral(","))
                   << "reason" << QStringLiteral("empty token")
                   << "requestId" << requestId;
            if (suppressed > 0) {
                stream << "(suppressed" << suppressed << "similar messages)";
            }
        });
        return result;
    }

    const QDateTime nowUtc = QDateTime::currentDateTimeUtc();

    for (const auto& token : m_settings.tokens) {
        if (token.value.isEmpty()) {
            continue;
        }
        if (token.value.toUtf8() != tokenValue) {
            continue;
        }
        const QString tokenDescription =
                token.description.isEmpty() ? QStringLiteral("<unnamed>") : token.description;
        if (token.expiresUtc.has_value() && token.expiresUtc.value() < nowUtc) {
            const QString key =
                    QStringLiteral("%1 expired-token").arg(requestDescription(request));
            m_authFailureLogger.log(key, [&](int suppressed) {
                auto stream = kAuditLogger.warning();
                stream << "REST auth failure" << requestDescription(request)
                       << "scopes" << requiredScopes.join(QStringLiteral(","))
                       << "reason" << QStringLiteral("token expired")
                       << "token" << tokenDescription
                       << "requestId" << requestId;
                if (suppressed > 0) {
                    stream << "(suppressed" << suppressed << "similar messages)";
                }
            });
            return result;
        }

        const QStringList tokenScopes = token.scopes;
        bool hasAllScopes = true;
        QStringList missingScopes;
        for (const auto& required : requiredScopes) {
            if (!tokenScopes.contains(required, Qt::CaseInsensitive)) {
                hasAllScopes = false;
                missingScopes.append(required);
            }
        }

        if (hasAllScopes) {
            result.authorized = true;
            result.tokenValue = token.value;
            result.tokenDescription = tokenDescription;
            return result;
        }
        result.forbidden = true;
        result.missingScopes = missingScopes;
        const QString key =
                QStringLiteral("%1 insufficient-permissions").arg(requestDescription(request));
        m_authFailureLogger.log(key, [&](int suppressed) {
            auto stream = kAuditLogger.warning();
            stream << "REST auth failure" << requestDescription(request)
                   << "scopes" << requiredScopes.join(QStringLiteral(","))
                   << "reason" << QStringLiteral("insufficient scopes")
                   << "missing" << missingScopes.join(QStringLiteral(","))
                   << "token" << tokenDescription
                   << "requestId" << requestId;
            if (suppressed > 0) {
                stream << "(suppressed" << suppressed << "similar messages)";
            }
        });
        return result;
    }
    const QString key = QStringLiteral("%1 unknown-token").arg(requestDescription(request));
    m_authFailureLogger.log(key, [&](int suppressed) {
        auto stream = kAuditLogger.warning();
        stream << "REST auth failure" << requestDescription(request)
               << "scopes" << requiredScopes.join(QStringLiteral(","))
               << "reason" << QStringLiteral("unknown token")
               << "requestId" << requestId;
        if (suppressed > 0) {
            stream << "(suppressed" << suppressed << "similar messages)";
        }
    });
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

QString RestServer::requestIdFor(const QHttpServerRequest& request) const {
    const QByteArray headerValue =
            request.headers().value(QByteArrayLiteral(kRequestIdHeader)).trimmed();
    if (!headerValue.isEmpty()) {
        return QString::fromUtf8(headerValue);
    }
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void RestServer::logRouteError(
        const QHttpServerRequest& request,
        QHttpServerResponse::StatusCode status,
        const QString& message,
        const QString& requestId) const {
    const QString key = QStringLiteral("%1 %2")
                                .arg(requestDescription(request),
                                        QString::number(static_cast<int>(status)));
    m_routeErrorLogger.log(key, [&](int suppressed) {
        auto stream = kLogger.warning();
        stream << "REST route error" << requestDescription(request)
               << static_cast<int>(status) << message << "requestId" << requestId;
        if (suppressed > 0) {
            stream << "(suppressed" << suppressed << "similar messages)";
        }
    });
}

void RestServer::registerRoutes() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(m_httpServer.get());

    struct RouteScopes {
        QStringList read;
        QStringList write;
    };

    const QHash<QString, RouteScopes> routeScopes{
            {QStringLiteral("/api/v1/schema"),
                    {QStringList{scopes::kStatusRead}, QStringList{scopes::kStatusRead}}},
            {QStringLiteral("/api/v1/health"),
                    {QStringList{scopes::kStatusRead}, QStringList{scopes::kStatusRead}}},
            {QStringLiteral("/api/v1/ready"),
                    {QStringList{scopes::kStatusRead}, QStringList{scopes::kStatusRead}}},
            {QStringLiteral("/api/v1/status"),
                    {QStringList{scopes::kStatusRead}, QStringList{scopes::kStatusRead}}},
            {QStringLiteral("/api/v1/stream/status"),
                    {QStringList{scopes::kStatusRead}, QStringList{scopes::kStatusRead}}},
            {QStringLiteral("/api/v1/decks"),
                    {QStringList{scopes::kDecksRead}, QStringList{scopes::kDecksRead}}},
            {QStringLiteral("/api/v1/autodj"),
                    {QStringList{scopes::kAutoDjRead}, QStringList{scopes::kAutoDjWrite}}},
            {QStringLiteral("/api/v1/control"),
                    {QStringList{scopes::kControlWrite}, QStringList{scopes::kControlWrite}}},
            {QStringLiteral("/api/v1/playlists"),
                    {QStringList{scopes::kPlaylistsRead}, QStringList{scopes::kPlaylistsWrite}}},
    };

    const auto toJsonArray = [](const QStringList& values) {
        QJsonArray array;
        array.reserve(values.size());
        for (const auto& value : values) {
            array.append(value);
        }
        return array;
    };
    const auto makeEndpoint = [&toJsonArray](const QString& path,
                                             const QStringList& methods,
                                             const QStringList& readScopes,
                                             const QStringList& writeScopes = {}) {
        QJsonObject scopes;
        if (!readScopes.isEmpty()) {
            scopes.insert(QStringLiteral("read"), toJsonArray(readScopes));
        }
        if (!writeScopes.isEmpty()) {
            scopes.insert(QStringLiteral("write"), toJsonArray(writeScopes));
        }
        return QJsonObject{
                {QStringLiteral("path"), path},
                {QStringLiteral("methods"), toJsonArray(methods)},
                {QStringLiteral("scopes"), scopes},
        };
    };
    const QJsonArray schemaEndpoints{
            makeEndpoint(QStringLiteral("/api/v1/health"),
                    QStringList{QStringLiteral("GET")},
                    QStringList{scopes::kStatusRead}),
            makeEndpoint(QStringLiteral("/api/v1/status"),
                    QStringList{QStringLiteral("GET")},
                    QStringList{scopes::kStatusRead}),
            makeEndpoint(QStringLiteral("/api/v1/control"),
                    QStringList{QStringLiteral("POST")},
                    QStringList{},
                    QStringList{scopes::kControlWrite}),
            makeEndpoint(QStringLiteral("/api/v1/autodj"),
                    QStringList{QStringLiteral("GET"), QStringLiteral("POST")},
                    QStringList{scopes::kAutoDjRead},
                    QStringList{scopes::kAutoDjWrite}),
            makeEndpoint(QStringLiteral("/api/v1/playlists"),
                    QStringList{QStringLiteral("GET"), QStringLiteral("POST")},
                    QStringList{scopes::kPlaylistsRead},
                    QStringList{scopes::kPlaylistsWrite}),
    };
    const QJsonObject schemaPayload{
            {QStringLiteral("version"), QStringLiteral("v1")},
            {QStringLiteral("base_path"), QStringLiteral("/api/v1")},
            {QStringLiteral("links"),
                    QJsonObject{
                            {QStringLiteral("health"), QStringLiteral("/api/v1/health")},
                            {QStringLiteral("status"), QStringLiteral("/api/v1/status")},
                            {QStringLiteral("control"), QStringLiteral("/api/v1/control")},
                            {QStringLiteral("autodj"), QStringLiteral("/api/v1/autodj")},
                            {QStringLiteral("playlists"), QStringLiteral("/api/v1/playlists")},
                    }},
            {QStringLiteral("endpoints"), schemaEndpoints},
            {QStringLiteral("actions"),
                    QJsonObject{
                            {QStringLiteral("autodj"),
                                    QJsonArray{
                                            QStringLiteral("enable"),
                                            QStringLiteral("disable"),
                                            QStringLiteral("skip"),
                                            QStringLiteral("fade"),
                                            QStringLiteral("shuffle"),
                                            QStringLiteral("add_random"),
                                            QStringLiteral("clear"),
                                            QStringLiteral("add"),
                                            QStringLiteral("move"),
                                    }},
                            {QStringLiteral("playlists"),
                                    QJsonArray{
                                            QStringLiteral("create"),
                                            QStringLiteral("delete"),
                                            QStringLiteral("rename"),
                                            QStringLiteral("set_active"),
                                            QStringLiteral("add"),
                                            QStringLiteral("remove"),
                                            QStringLiteral("reorder"),
                                            QStringLiteral("send_to_autodj"),
                                    }},
                    }},
            {QStringLiteral("control_payloads"),
                    QJsonArray{
                            QStringLiteral("command"),
                            QStringLiteral("commands"),
                            QStringLiteral("direct_control"),
                    }},
    };

    const QString deckDetailPrefix = QStringLiteral("/api/v1/decks/");
    const auto requiredScopesFor = [&routeScopes, &deckDetailPrefix](
                                           const QHttpServerRequest& request) {
        const QString path = request.url().path();
        const QString key = path.startsWith(deckDetailPrefix)
                ? QStringLiteral("/api/v1/decks")
                : path;
        const RouteScopes scopes = routeScopes.value(key);
        if (request.method() == QHttpServerRequest::Method::Get) {
            return scopes.read;
        }
        return scopes.write.isEmpty() ? scopes.read : scopes.write;
    };
    const auto forbiddenMessage = [](const QStringList& missingScopes) {
        return QObject::tr("Token missing required scopes: %1")
                .arg(missingScopes.join(QStringLiteral(", ")));
    };
    const auto authorizeRequest = [this, &requiredScopesFor](
                                          const QHttpServerRequest& request) {
        return authorize(request, requiredScopesFor(request));
    };

    const auto optionsRoute = [this](const QHttpServerRequest& request) {
        QHttpServerResponse response(QHttpServerResponse::StatusCode::NoContent);
        addCorsHeaders(&response, request, true);
        return response;
    };
    m_httpServer->route("/*", QHttpServerRequest::Method::Options, optionsRoute);

    const auto schemaRoute = [this, schemaPayload](const QHttpServerRequest& request) {
        const AuthorizationResult auth = authorizeRequest(request);
        if (!auth.authorized) {
            if (auth.forbidden) {
                return forbiddenResponse(request, forbiddenMessage(auth.missingScopes));
            }
            return unauthorizedResponse(request);
        }
        if (request.method() != QHttpServerRequest::Method::Get) {
            return methodNotAllowedResponse(request);
        }
        return jsonResponse(
                request,
                schemaPayload,
                QHttpServerResponse::StatusCode::Ok,
                requestIdFor(request));
    };
    m_httpServer->route("/api/v1/schema", schemaRoute);

    const auto requestTooLarge = [this](const QHttpServerRequest& request) {
        return m_settings.maxRequestBytes > 0 &&
                request.body().size() > m_settings.maxRequestBytes;
    };
    const auto idempotencyKeyFor = [](const QHttpServerRequest& request) {
        return QString::fromUtf8(request.value(kIdempotencyKeyHeader).trimmed());
    };

    const auto healthRoute = [this](const QHttpServerRequest& request) {
        const AuthorizationResult auth = authorizeRequest(request);
        if (!auth.authorized) {
            if (auth.forbidden) {
                return forbiddenResponse(request, forbiddenMessage(auth.missingScopes));
            }
            return unauthorizedResponse(request);
        }
        if (request.method() != QHttpServerRequest::Method::Get) {
            return methodNotAllowedResponse(request);
        }
        return invokeGateway(request, [this]() {
            return m_gateway->health();
        });
    };
    m_httpServer->route("/api/v1/health", healthRoute);

    const auto readyRoute = [this](const QHttpServerRequest& request) {
        const AuthorizationResult auth = authorizeRequest(request);
        if (!auth.authorized) {
            if (auth.forbidden) {
                return forbiddenResponse(request, forbiddenMessage(auth.missingScopes));
            }
            return unauthorizedResponse(request);
        }
        if (request.method() != QHttpServerRequest::Method::Get) {
            return methodNotAllowedResponse(request);
        }
        return invokeGateway(request, [this]() {
            return m_gateway->ready();
        });
    };
    m_httpServer->route("/api/v1/ready", readyRoute);

    const auto statusRoute = [this](const QHttpServerRequest& request) {
        const AuthorizationResult auth = authorizeRequest(request);
        if (!auth.authorized) {
            if (auth.forbidden) {
                return forbiddenResponse(request, forbiddenMessage(auth.missingScopes));
            }
            return unauthorizedResponse(request);
        }
        if (request.method() != QHttpServerRequest::Method::Get) {
            return methodNotAllowedResponse(request);
        }
        return invokeGateway(request, [this]() {
            return m_gateway->status();
        });
    };
    m_httpServer->route("/api/v1/status", statusRoute);

    const auto statusStreamRoute = [this](
                                           const QHttpServerRequest& request,
                                           QHttpServerResponder&& responder) {
        if (!m_settings.streamEnabled) {
            responder.write(serviceUnavailableResponse(&request));
            return;
        }
        const AuthorizationResult auth = authorizeRequest(request);
        if (!auth.authorized) {
            if (auth.forbidden) {
                responder.write(forbiddenResponse(request, forbiddenMessage(auth.missingScopes)));
                return;
            }
            responder.write(unauthorizedResponse(request));
            return;
        }
        if (request.method() != QHttpServerRequest::Method::Get) {
            responder.write(methodNotAllowedResponse(request));
            return;
        }
        addStatusStreamClient(request, std::move(responder));
    };
    m_httpServer->route("/api/v1/stream/status", statusStreamRoute);

    const auto decksRoute = [this](const QHttpServerRequest& request) {
        const AuthorizationResult auth = authorizeRequest(request);
        if (!auth.authorized) {
            if (auth.forbidden) {
                return forbiddenResponse(request, forbiddenMessage(auth.missingScopes));
            }
            return unauthorizedResponse(request);
        }
        if (request.method() != QHttpServerRequest::Method::Get) {
            return methodNotAllowedResponse(request);
        }
        return invokeGateway(request, [this]() {
            return m_gateway->decks();
        });
    };
    m_httpServer->route("/api/v1/decks", decksRoute);

    const auto deckRoute = [this](const QHttpServerRequest& request, int deckNumber) {
        const AuthorizationResult auth = authorizeRequest(request);
        if (!auth.authorized) {
            if (auth.forbidden) {
                return forbiddenResponse(request, forbiddenMessage(auth.missingScopes));
            }
            return unauthorizedResponse(request);
        }
        if (request.method() != QHttpServerRequest::Method::Get) {
            return methodNotAllowedResponse(request);
        }
        return invokeGateway(request, [this, deckNumber]() {
            return m_gateway->deck(deckNumber);
        });
    };
    m_httpServer->route("/api/v1/decks/<int>", deckRoute);

    const auto controlRoute = [this](const QHttpServerRequest& request) {
        const AuthorizationResult auth = authorizeRequest(request);
        if (!auth.authorized) {
            if (auth.forbidden) {
                return forbiddenResponse(request, forbiddenMessage(auth.missingScopes));
            }
            return unauthorizedResponse(request);
        }
        if (request.method() != QHttpServerRequest::Method::Post) {
            return methodNotAllowedResponse(request);
        }
        if (controlRouteRequiresTls(request)) {
            return tlsRequiredResponse(request);
        }

        if (!isJsonContentType(request)) {
            return unsupportedMediaTypeResponse(
                    request,
                    QStringLiteral("Expected Content-Type application/json"));
        }

        if (requestTooLarge(request)) {
            return payloadTooLargeResponse(request);
        }

        QJsonParseError parseError;
        const auto document = QJsonDocument::fromJson(request.body(), &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            return badRequestResponse(
                    request,
                    tr("Invalid JSON: %1").arg(parseError.errorString()));
        }
        if (!document.isObject()) {
            return badRequestResponse(request, QStringLiteral("Expected JSON request body"));
        }
        const auto body = document.object();
        const QString idempotencyKey = idempotencyKeyFor(request);
        const QString endpoint = request.url().path();
        const QString token = auth.tokenValue;
        const QString requestId = requestIdFor(request);
        kAuditLogger.info() << "REST control action" << requestDescription(request)
                            << "token" << auth.tokenDescription
                            << "requestId" << requestId;
        return invokeGateway(request,
                [this, body, token, idempotencyKey, endpoint]() {
            return m_gateway->withIdempotencyCache(
                    token,
                    idempotencyKey,
                    endpoint,
                    [this, body]() { return m_gateway->control(body); });
        },
                requestId);
    };
    m_httpServer->route("/api/v1/control", controlRoute);

    const auto autoDjRoute = [this](const QHttpServerRequest& request) {
        const AuthorizationResult auth = authorizeRequest(request);
        if (!auth.authorized) {
            if (auth.forbidden) {
                return forbiddenResponse(request, forbiddenMessage(auth.missingScopes));
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

        if (!isJsonContentType(request)) {
            return unsupportedMediaTypeResponse(
                    request,
                    QStringLiteral("Expected Content-Type application/json"));
        }

        if (requestTooLarge(request)) {
            return payloadTooLargeResponse(request);
        }

        QJsonParseError parseError;
        const auto document = QJsonDocument::fromJson(request.body(), &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            return badRequestResponse(
                    request,
                    tr("Invalid JSON: %1").arg(parseError.errorString()));
        }
        if (!document.isObject()) {
            return badRequestResponse(request, QStringLiteral("Expected JSON request body"));
        }
        const auto body = document.object();
        const QString idempotencyKey = idempotencyKeyFor(request);
        const QString endpoint = request.url().path();
        const QString token = auth.tokenValue;
        const QString requestId = requestIdFor(request);
        kAuditLogger.info() << "REST control action" << requestDescription(request)
                            << "token" << auth.tokenDescription
                            << "requestId" << requestId;
        return invokeGateway(request,
                [this, body, token, idempotencyKey, endpoint]() {
            return m_gateway->withIdempotencyCache(
                    token,
                    idempotencyKey,
                    endpoint,
                    [this, body]() { return m_gateway->autoDj(body); });
        },
                requestId);
    };
    m_httpServer->route("/api/v1/autodj", autoDjRoute);

    const auto playlistsRoute = [this](const QHttpServerRequest& request) {
        const AuthorizationResult auth = authorizeRequest(request);
        if (!auth.authorized) {
            if (auth.forbidden) {
                return forbiddenResponse(request, forbiddenMessage(auth.missingScopes));
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

        if (!isJsonContentType(request)) {
            return unsupportedMediaTypeResponse(
                    request,
                    QStringLiteral("Expected Content-Type application/json"));
        }

        if (requestTooLarge(request)) {
            return payloadTooLargeResponse(request);
        }

        QJsonParseError parseError;
        const auto document = QJsonDocument::fromJson(request.body(), &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            return badRequestResponse(
                    request,
                    tr("Invalid JSON: %1").arg(parseError.errorString()));
        }
        if (!document.isObject()) {
            return badRequestResponse(request, QStringLiteral("Expected JSON request body"));
        }
        const auto body = document.object();
        const QString idempotencyKey = idempotencyKeyFor(request);
        const QString endpoint = request.url().path();
        const QString token = auth.tokenValue;
        const QString requestId = requestIdFor(request);
        kAuditLogger.info() << "REST control action" << requestDescription(request)
                            << "token" << auth.tokenDescription
                            << "requestId" << requestId;
        return invokeGateway(request,
                [this, body, token, idempotencyKey, endpoint]() {
            return m_gateway->withIdempotencyCache(
                    token,
                    idempotencyKey,
                    endpoint,
                    [this, body]() { return m_gateway->playlistCommand(body); });
        },
                requestId);
    };
    m_httpServer->route("/api/v1/playlists", playlistsRoute);
}

void RestServer::addStatusStreamClient(
        const QHttpServerRequest& request,
        QHttpServerResponder&& responder) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(m_httpServer.get());

    if (m_settings.streamMaxClients > 0 &&
            static_cast<int>(m_streamClients.size()) >= m_settings.streamMaxClients) {
        const QString message = QStringLiteral("Too many stream clients");
        const QString requestId = requestIdFor(request);
        logRouteError(
                request,
                QHttpServerResponse::StatusCode::TooManyRequests,
                message,
                requestId);
        responder.write(jsonResponse(request, QJsonObject{{"error", message}},
                QHttpServerResponse::StatusCode::TooManyRequests,
                requestId));
        return;
    }

    QHttpServerResponse response(QHttpServerResponse::StatusCode::Ok);
    response.setHeader(QByteArrayLiteral(kContentTypeHeader),
            QByteArrayLiteral(kEventStreamContentType));
    response.setHeader(QByteArrayLiteral(kCacheControlHeader),
            QByteArrayLiteral(kNoCacheValue));
    response.setHeader(QByteArrayLiteral(kConnectionHeader),
            QByteArrayLiteral(kKeepAliveValue));
    response.setHeader(QByteArrayLiteral(kAccelBufferingHeader),
            QByteArrayLiteral(kAccelBufferingDisabled));
    const QString allowedOrigin = allowedCorsOrigin(request);
    if (!allowedOrigin.isEmpty()) {
        response.setHeader(QByteArrayLiteral(kCorsAllowOriginHeader), allowedOrigin.toUtf8());
    }
    responder.write(response);

    const quint64 clientId = ++m_streamClientCounter;
    QJsonObject payload = fetchStatusPayload();
    const QJsonObject delta = statusDelta(QJsonObject{}, payload);
    StreamClient client{clientId, std::move(responder), payload};
    auto result = m_streamClients.emplace(clientId, std::move(client));
    if (!result.second) {
        return;
    }
    sendStatusStreamEvent(delta, &result.first->second.responder);

}

void RestServer::sendStatusStreamEvent(
        const QJsonObject& payload,
        QHttpServerResponder* responder) const {
    if (!responder) {
        return;
    }
    responder->write(formatSseEvent(payload));
}

QJsonObject RestServer::fetchStatusPayload() const {
    if (m_gateway.isNull()) {
        return QJsonObject{};
    }
    QJsonObject payload;
    QSemaphore semaphore;
    QMetaObject::invokeMethod(
            m_gateway,
            [&]() {
                payload = m_gateway->statusPayload();
                semaphore.release();
            },
            Qt::QueuedConnection);
    semaphore.acquire();
    return payload;
}

QJsonObject RestServer::statusDelta(
        const QJsonObject& previous,
        const QJsonObject& current) const {
    QJsonObject delta;
    for (auto it = current.begin(); it != current.end(); ++it) {
        if (!previous.contains(it.key()) || previous.value(it.key()) != it.value()) {
            delta.insert(it.key(), it.value());
        }
    }
    for (auto it = previous.begin(); it != previous.end(); ++it) {
        if (!current.contains(it.key())) {
            delta.insert(it.key(), QJsonValue::Null);
        }
    }
    return delta;
}

void RestServer::pushStatusStreamUpdate() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(m_httpServer.get());

    if (!m_settings.streamEnabled || m_streamClients.empty()) {
        return;
    }

    const QJsonObject payload = fetchStatusPayload();
    for (auto& entry : m_streamClients) {
        StreamClient& client = entry.second;
        const QJsonObject delta = statusDelta(client.lastPayload, payload);
        sendStatusStreamEvent(delta, &client.responder);
        client.lastPayload = payload;
    }
}

void RestServer::addCorsHeaders(
        QHttpServerResponse* response,
        const QHttpServerRequest& request,
        bool includeAllowHeaders) const {
    const QString allowedOrigin = allowedCorsOrigin(request);
    if (allowedOrigin.isEmpty()) {
        return;
    }
    response->setHeader(QByteArrayLiteral(kCorsAllowOriginHeader), allowedOrigin.toUtf8());
    if (includeAllowHeaders) {
        response->setHeader(QByteArrayLiteral(kCorsAllowMethodsHeader),
                QByteArrayLiteral("GET, POST, PUT, PATCH, DELETE, OPTIONS"));
        const QByteArray requestedHeaders =
                request.headers().value(QByteArrayLiteral(kCorsRequestHeadersHeader)).trimmed();
        const QByteArray allowHeaders = requestedHeaders.isEmpty()
                ? QByteArrayLiteral("Authorization, Content-Type, Idempotency-Key, X-Request-Id")
                : requestedHeaders;
        response->setHeader(QByteArrayLiteral(kCorsAllowHeadersHeader), allowHeaders);
    }
}

QString RestServer::allowedCorsOrigin(const QHttpServerRequest& request) const {
    const QByteArray originHeader =
            request.headers().value(QByteArrayLiteral(kCorsOriginHeader)).trimmed();
    if (originHeader.isEmpty()) {
        return QString();
    }
    const QString origin = QString::fromUtf8(originHeader);
    for (const auto& allowed : m_settings.corsAllowlist) {
        const QString trimmed = allowed.trimmed();
        if (trimmed.isEmpty()) {
            continue;
        }
        if (trimmed == QStringLiteral("*")) {
            return QStringLiteral("*");
        }
        if (trimmed == origin) {
            return origin;
        }
    }
    return QString();
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

    if (m_settings.streamEnabled) {
        m_streamTimer.stop();
        m_streamTimer.setInterval(m_settings.streamIntervalMs);
        m_streamTimer.setSingleShot(false);
        if (m_streamTimer.thread() != m_threadObject->thread()) {
            m_streamTimer.moveToThread(m_threadObject->thread());
        }
        QObject::disconnect(&m_streamTimer, nullptr, nullptr, nullptr);
        connect(&m_streamTimer, &QTimer::timeout, m_threadObject.get(), [this]() {
            pushStatusStreamUpdate();
        });
        m_streamTimer.start();
    }
    return true;
}

void RestServer::stopOnThread() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(m_threadObject.get());
    m_streamTimer.stop();
    m_streamClients.clear();
    if (m_httpServer) {
        m_httpServer->close();
        m_httpServer.reset();
    }
    kLogger.info() << "REST API server stopped";
}

} // namespace mixxx::network::rest

#endif // MIXXX_HAS_HTTP_SERVER
