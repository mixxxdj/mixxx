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
#include <QRegularExpression>
#include <QSemaphore>
#include <QSslCipher>
#include <QSslSocket>
#include <QUuid>
#include <QUrl>
#include <QUrlQuery>
#include <type_traits>
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
constexpr auto kStrictTransportSecurityHeader = "Strict-Transport-Security";
constexpr auto kStrictTransportSecurityValue = "max-age=15552000; includeSubDomains";
constexpr int kMaxRequestIdLength = 128;
const QRegularExpression kUuidRequestIdRegex(
        QStringLiteral("^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-"
                       "[0-9a-fA-F]{12}$"));
const QRegularExpression kAllowedRequestIdRegex(QStringLiteral("^[A-Za-z0-9._-]+$"));

bool tokenHasWriteScope(const QStringList& tokenScopes) {
    for (const auto& scope : scopes::writeScopes()) {
        if (tokenScopes.contains(scope, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

#if defined(QT_HTTPSERVER_VERSION)
#define MIXXX_HAS_QT_HTTPSERVER_VERSION
#endif

template<typename T, typename = void>
struct HasHttpServerListen : std::false_type { };

template<typename T>
struct HasHttpServerListen<T,
        std::void_t<decltype(std::declval<T&>().listen(
                std::declval<const QHostAddress&>(),
                std::declval<quint16>()))>> : std::true_type { };

template<typename T, typename = void>
struct HasHttpServerStop : std::false_type { };

template<typename T>
struct HasHttpServerStop<T,
        std::void_t<decltype(std::declval<T&>().stop())>> : std::true_type { };

template<typename T, typename = void>
struct HasHttpServerClose : std::false_type { };

template<typename T>
struct HasHttpServerClose<T,
        std::void_t<decltype(std::declval<T&>().close())>> : std::true_type { };

template<typename T, typename = void>
struct HasHttpServerSslConfiguration : std::false_type { };

template<typename T>
struct HasHttpServerSslConfiguration<T,
        std::void_t<decltype(std::declval<T&>().setSslConfiguration(
                std::declval<const QSslConfiguration&>()))>> : std::true_type { };

template<typename T, typename = void>
struct HasHttpServerSslSetup : std::false_type { };

template<typename T>
struct HasHttpServerSslSetup<T,
        std::void_t<decltype(std::declval<T&>().sslSetup(
                std::declval<const QSslConfiguration&>()))>> : std::true_type { };

template<typename T, typename = void>
struct HasHttpServerResponderWrite : std::false_type { };

template<typename T>
struct HasHttpServerResponderWrite<T,
        std::void_t<decltype(std::declval<T&>().write(
                std::declval<const QByteArray&>()))>> : std::true_type { };

template<typename T, typename = void>
struct HasListenResultErrorStringMethod : std::false_type { };

template<typename T>
struct HasListenResultErrorStringMethod<T,
        std::void_t<decltype(std::declval<const T&>().errorString())>> : std::true_type { };

template<typename T, typename = void>
struct HasListenResultErrorStringMember : std::false_type { };

template<typename T>
struct HasListenResultErrorStringMember<T,
        std::void_t<decltype(std::declval<const T&>().errorString)>> : std::true_type { };

enum class HttpServerListenStatus {
    NotSupported,
    Failed,
    Ok,
};

constexpr bool kHttpServerHasResponderWrite =
        HasHttpServerResponderWrite<QHttpServerResponder>::value;

void appendHeader(RestHeaders* headers, const QByteArray& name, const QByteArray& value) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    headers->append(name, value);
#else
    headers->insert(name, value);
#endif
}

template<typename Response, typename Headers, typename = void>
struct HasSetHeadersMethod : std::false_type { };

template<typename Response, typename Headers>
struct HasSetHeadersMethod<Response,
        Headers,
        std::void_t<decltype(std::declval<Response&>().setHeaders(std::declval<Headers>()))>>
        : std::true_type { };

template<typename Response, typename Headers>
void setResponseHeaders(Response* response, const Headers& headers) {
    if constexpr (HasSetHeadersMethod<Response, Headers>::value) {
        response->setHeaders(headers);
    } else {
        for (auto it = headers.cbegin(); it != headers.cend(); ++it) {
            response->setHeader(it.key(), it.value());
        }
    }
}

template<typename Server>
bool configureHttpServerTls(Server* server, const QSslConfiguration& configuration) {
    if constexpr (HasHttpServerSslConfiguration<Server>::value) {
        server->setSslConfiguration(configuration);
        return true;
    } else if constexpr (HasHttpServerSslSetup<Server>::value) {
        server->sslSetup(configuration);
        return true;
    }
    return false;
}

template<typename Server>
HttpServerListenStatus listenHttpServer(
        Server* server,
        const QHostAddress& address,
        quint16 port,
        quint16* boundPort,
        QString* errorOut) {
    if constexpr (HasHttpServerListen<Server>::value) {
        const auto listenResult = server->listen(address, port);
        if (!listenResult) {
            if (errorOut) {
                if constexpr (HasListenResultErrorStringMethod<decltype(listenResult)>::value) {
                    *errorOut = listenResult.errorString();
                } else if constexpr (HasListenResultErrorStringMember<decltype(listenResult)>::value) {
                    *errorOut = listenResult.errorString;
                }
            }
            return HttpServerListenStatus::Failed;
        }
        if (boundPort) {
            *boundPort = listenResult.port;
        }
        return HttpServerListenStatus::Ok;
    }
    return HttpServerListenStatus::NotSupported;
}

template<typename Server>
void stopHttpServer(Server* server) {
    if constexpr (HasHttpServerStop<Server>::value) {
        server->stop();
    } else if constexpr (HasHttpServerClose<Server>::value) {
        server->close();
    }
}

QByteArray responseBody(const QHttpServerResponse& response) {
#if defined(MIXXX_HAS_QT_HTTPSERVER_VERSION)
    if constexpr (QT_HTTPSERVER_VERSION >= QT_VERSION_CHECK(6, 8, 0)) {
        return response.body();
    }
#endif
    return response.data();
}

bool requestIsSecure(const QHttpServerRequest& request) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    return request.url().scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) == 0;
#elif QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    return request.isSecure();
#else
    return request.url().scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) == 0;
#endif
}

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
    const auto values = request.headers().values(QByteArrayLiteral("Content-Type"));
    for (const auto& headerValue : values) {
        const QByteArray trimmedValue = headerValue.trimmed();
        if (trimmedValue.isEmpty()) {
            continue;
        }
        const int separatorIndex = trimmedValue.indexOf(';');
        const QByteArray mediaType = (separatorIndex >= 0 ? trimmedValue.left(separatorIndex)
                                                          : trimmedValue)
                                             .trimmed()
                                             .toLower();
        if (mediaType == QByteArrayLiteral("application/json")) {
            return true;
        }
    }
    return false;
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

template<typename Responder, typename Payload>
bool writeResponder(Responder* responder, const Payload& payload) {
    if (!responder) {
        return false;
    }
    if constexpr (std::is_same_v<std::decay_t<Payload>, QHttpServerResponse>) {
        RestHeaders headers = payload.headers();
        const QByteArray mimeType = payload.mimeType();
        if (!mimeType.isEmpty()) {
            appendHeader(&headers, QByteArrayLiteral("Content-Type"), mimeType);
        }
        responder->write(
                responseBody(payload),
                headers,
                static_cast<QHttpServerResponder::StatusCode>(payload.statusCode()));
        return true;
    } else if constexpr (std::is_same_v<std::decay_t<Payload>, QByteArray>) {
        if constexpr (kHttpServerHasResponderWrite) {
            responder->write(payload);
        } else {
            responder->write(payload, RestHeaders{}, QHttpServerResponder::StatusCode::Ok);
        }
        return true;
    } else {
        responder->write(payload);
        return true;
    }
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
            QByteArrayLiteral("application/json"),
            QJsonDocument(body).toJson(QJsonDocument::Compact),
            status);
    RestHeaders headers = response.headers();
    if (!requestId.isEmpty()) {
        appendHeader(&headers, QByteArrayLiteral(kRequestIdHeader), requestId.toUtf8());
    }
    if (m_tlsActive && requestIsSecure(request)) {
        appendHeader(&headers,
                QByteArrayLiteral(kStrictTransportSecurityHeader),
                QByteArrayLiteral(kStrictTransportSecurityValue));
    }
    addCorsHeaders(&headers, request, false);
    setResponseHeaders(&response, headers);
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
    if (!QSslSocket::supportsSsl()) {
        kLogger.warning() << "REST API TLS preparation failed: OpenSSL is not available";
        const QString supportDetails = QStringLiteral("Qt build: %1, runtime: %2, SSL build: %3, SSL runtime: %4")
                                               .arg(QString::fromLatin1(QT_VERSION_STR),
                                                       QString::fromLatin1(qVersion()),
                                                       QSslSocket::sslLibraryBuildVersionString(),
                                                       QSslSocket::sslLibraryVersionString());
        result.error = QObject::tr("OpenSSL is not available for TLS support (%1)").arg(supportDetails);
        return result;
    }
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
        if (!certificateResult.certificatePath.isEmpty() ||
                !certificateResult.privateKeyPath.isEmpty()) {
            result.error = QObject::tr("%1 (certificate: %2, key: %3)")
                                   .arg(certificateResult.error,
                                           certificateResult.certificatePath,
                                           certificateResult.privateKeyPath);
        } else {
            result.error = certificateResult.error;
        }
        return result;
    }

    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setLocalCertificate(certificateResult.certificate);
    sslConfig.setPrivateKey(certificateResult.privateKey);
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    sslConfig.setProtocol(QSsl::TlsV1_3OrLater);
#else
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
#endif
    sslConfig.setSslOption(QSsl::SslOptionDisableCompression, true);
    const QStringList modernCipherNames = {
            QStringLiteral("TLS_AES_256_GCM_SHA384"),
            QStringLiteral("TLS_AES_128_GCM_SHA256"),
            QStringLiteral("TLS_CHACHA20_POLY1305_SHA256"),
            QStringLiteral("ECDHE-ECDSA-AES256-GCM-SHA384"),
            QStringLiteral("ECDHE-RSA-AES256-GCM-SHA384"),
            QStringLiteral("ECDHE-ECDSA-AES128-GCM-SHA256"),
            QStringLiteral("ECDHE-RSA-AES128-GCM-SHA256"),
            QStringLiteral("ECDHE-ECDSA-CHACHA20-POLY1305"),
            QStringLiteral("ECDHE-RSA-CHACHA20-POLY1305"),
    };
    QList<QSslCipher> modernCiphers;
    modernCiphers.reserve(modernCipherNames.size());
    for (const QString& cipherName : modernCipherNames) {
        const QSslCipher cipher(cipherName);
        if (!cipher.isNull()) {
            modernCiphers.push_back(cipher);
        }
    }
    if (!modernCiphers.isEmpty()) {
        sslConfig.setCiphers(modernCiphers);
    }

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
            QByteArrayLiteral("application/json"),
            QJsonDocument(QJsonObject{{"error", message}}).toJson(QJsonDocument::Compact),
            QHttpServerResponse::StatusCode::ServiceUnavailable);
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
    RestHeaders headers = response.headers();
    if (!requestId.isEmpty()) {
        appendHeader(&headers, QByteArrayLiteral(kRequestIdHeader), requestId.toUtf8());
    }
    addCorsHeaders(&headers, request, false);
    setResponseHeaders(&response, headers);
    return response;
}

bool RestServer::applyTlsConfiguration() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(m_httpServer.get());

#if QT_CONFIG(ssl)
    if (!m_tlsConfiguration.has_value()) {
        kLogger.warning() << "TLS is enabled but no TLS configuration was provided";
        m_lastError = tr("TLS is enabled but no TLS configuration was provided");
        return false;
    }
    if (!configureHttpServerTls(
                m_httpServer.get(),
                m_tlsConfiguration->configuration)) {
        kLogger.warning() << "TLS is enabled but the Qt HTTP server lacks SSL configuration APIs";
        m_lastError = tr("TLS is enabled but the Qt HTTP server lacks SSL configuration APIs");
        return false;
    }
    m_tlsActive = true;
    return true;
#else
    Q_UNUSED(m_settings);
    kLogger.warning() << "TLS requested for REST API but Qt is built without SSL support";
    m_lastError = tr("TLS requested for REST API but Qt is built without SSL support");
    return false;
#endif
}

RestServer::AuthorizationResult RestServer::authorize(
        const QHttpServerRequest& request,
        const QStringList& requiredScopes) {
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
        result.usedWriteToken = tokenHasWriteScope(tokenScopes);
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
            emit tokenUsed(token.value, nowUtc);
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

bool RestServer::writeTokenRequiresTls(
        const AuthorizationResult& auth,
        const QHttpServerRequest& request) const {
    if (!m_settings.requireTls || !auth.usedWriteToken) {
        return false;
    }

    return !requestIsSecure(request);
}

QString RestServer::requestDescription(const QHttpServerRequest& request) const {
    return QStringLiteral("%1 %2")
            .arg(methodToString(request.method()), request.url().path());
}

QString RestServer::requestIdFor(const QHttpServerRequest& request) const {
    const QByteArray headerValue =
            QByteArray(request.headers().value(QByteArrayLiteral(kRequestIdHeader))).trimmed();
    if (!headerValue.isEmpty() && headerValue.size() <= kMaxRequestIdLength) {
        const QString requestId = QString::fromUtf8(headerValue);
        if (kUuidRequestIdRegex.match(requestId).hasMatch() ||
                kAllowedRequestIdRegex.match(requestId).hasMatch()) {
            return requestId;
        }
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
            {QStringLiteral("/api/v1/schema"), {QStringList{}, QStringList{}}},
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 11, 0)
        array.reserve(values.size());
#endif
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
    const auto requiredScopesFor = [routeScopes, deckDetailPrefix](
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
    const auto authorizeRequest = [this, requiredScopesFor](
                                          const QHttpServerRequest& request) {
        return authorize(request, requiredScopesFor(request));
    };

    const auto optionsRoute = [this](const QHttpServerRequest& request) {
        QHttpServerResponse response(QHttpServerResponse::StatusCode::NoContent);
        RestHeaders headers = response.headers();
        addCorsHeaders(&headers, request, true);
        setResponseHeaders(&response, headers);
        return response;
    };
    m_httpServer->route("/*", QHttpServerRequest::Method::Options, optionsRoute);

    const auto schemaRoute =
            [this, schemaPayload, authorizeRequest, forbiddenMessage](
                    const QHttpServerRequest& request) {
        std::optional<AuthorizationResult> auth;
        if (!(m_settings.allowUnauthenticated &&
                    request.value(kAuthHeader).trimmed().isEmpty())) {
            auth = authorizeRequest(request);
            if (!auth->authorized) {
                if (auth->forbidden) {
                    return forbiddenResponse(request, forbiddenMessage(auth->missingScopes));
                }
                return unauthorizedResponse(request);
            }
            if (writeTokenRequiresTls(*auth, request)) {
                return tlsRequiredResponse(request);
            }
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
    m_httpServer->route("/schema", schemaRoute);

    const auto requestTooLarge = [this](const QHttpServerRequest& request) {
        return m_settings.maxRequestBytes > 0 &&
                request.body().size() > m_settings.maxRequestBytes;
    };
    const auto idempotencyKeyFor = [](const QHttpServerRequest& request) {
        return QString::fromUtf8(request.value(kIdempotencyKeyHeader).trimmed());
    };

    const auto healthRoute = [this, authorizeRequest, forbiddenMessage](
                                     const QHttpServerRequest& request) {
        const AuthorizationResult auth = authorizeRequest(request);
        if (!auth.authorized) {
            if (auth.forbidden) {
                return forbiddenResponse(request, forbiddenMessage(auth.missingScopes));
            }
            return unauthorizedResponse(request);
        }
        if (writeTokenRequiresTls(auth, request)) {
            return tlsRequiredResponse(request);
        }
        if (request.method() != QHttpServerRequest::Method::Get) {
            return methodNotAllowedResponse(request);
        }
        return invokeGateway(request, [this]() {
            return m_gateway->health();
        });
    };
    m_httpServer->route("/api/v1/health", healthRoute);

    const auto readyRoute = [this, authorizeRequest, forbiddenMessage](
                                    const QHttpServerRequest& request) {
        const AuthorizationResult auth = authorizeRequest(request);
        if (!auth.authorized) {
            if (auth.forbidden) {
                return forbiddenResponse(request, forbiddenMessage(auth.missingScopes));
            }
            return unauthorizedResponse(request);
        }
        if (writeTokenRequiresTls(auth, request)) {
            return tlsRequiredResponse(request);
        }
        if (request.method() != QHttpServerRequest::Method::Get) {
            return methodNotAllowedResponse(request);
        }
        return invokeGateway(request, [this]() {
            return m_gateway->ready();
        });
    };
    m_httpServer->route("/api/v1/ready", readyRoute);

    const auto statusRoute = [this, authorizeRequest, forbiddenMessage](
                                     const QHttpServerRequest& request) {
        const AuthorizationResult auth = authorizeRequest(request);
        if (!auth.authorized) {
            if (auth.forbidden) {
                return forbiddenResponse(request, forbiddenMessage(auth.missingScopes));
            }
            return unauthorizedResponse(request);
        }
        if (writeTokenRequiresTls(auth, request)) {
            return tlsRequiredResponse(request);
        }
        if (request.method() != QHttpServerRequest::Method::Get) {
            return methodNotAllowedResponse(request);
        }
        return invokeGateway(request, [this]() {
            return m_gateway->status();
        });
    };
    m_httpServer->route("/api/v1/status", statusRoute);

    const auto statusStreamRoute =
            [this, authorizeRequest, forbiddenMessage](
        const QHttpServerRequest& request,
        QHttpServerResponder& responder) {
        if (!m_settings.streamEnabled) {
            writeResponder(&responder, serviceUnavailableResponse(&request));
            return;
        }
        const AuthorizationResult auth = authorizeRequest(request);
        if (!auth.authorized) {
            if (auth.forbidden) {
                writeResponder(&responder,
                        forbiddenResponse(request, forbiddenMessage(auth.missingScopes)));
                return;
            }
            writeResponder(&responder, unauthorizedResponse(request));
            return;
        }
        if (writeTokenRequiresTls(auth, request)) {
            writeResponder(&responder, tlsRequiredResponse(request));
            return;
        }
        if (request.method() != QHttpServerRequest::Method::Get) {
            writeResponder(&responder, methodNotAllowedResponse(request));
            return;
        }
        addStatusStreamClient(request, responder);
    };
    m_httpServer->route("/api/v1/stream/status", statusStreamRoute);

    const auto decksRoute = [this, authorizeRequest, forbiddenMessage](
                                    const QHttpServerRequest& request) {
        const AuthorizationResult auth = authorizeRequest(request);
        if (!auth.authorized) {
            if (auth.forbidden) {
                return forbiddenResponse(request, forbiddenMessage(auth.missingScopes));
            }
            return unauthorizedResponse(request);
        }
        if (writeTokenRequiresTls(auth, request)) {
            return tlsRequiredResponse(request);
        }
        if (request.method() != QHttpServerRequest::Method::Get) {
            return methodNotAllowedResponse(request);
        }
        return invokeGateway(request, [this]() {
            return m_gateway->decks();
        });
    };
    m_httpServer->route("/api/v1/decks", decksRoute);

    const auto deckRoute = [this, authorizeRequest, forbiddenMessage](
                                   const QString& deckNumberArg,
                                   const QHttpServerRequest& request) {
        const AuthorizationResult auth = authorizeRequest(request);
        if (!auth.authorized) {
            if (auth.forbidden) {
                return forbiddenResponse(request, forbiddenMessage(auth.missingScopes));
            }
            return unauthorizedResponse(request);
        }
        if (writeTokenRequiresTls(auth, request)) {
            return tlsRequiredResponse(request);
        }
        if (request.method() != QHttpServerRequest::Method::Get) {
            return methodNotAllowedResponse(request);
        }
        bool ok = false;
        const int deckNumber = deckNumberArg.toInt(&ok);
        if (!ok) {
            return badRequestResponse(request, QStringLiteral("Deck number must be numeric"));
        }
        return invokeGateway(request, [this, deckNumber]() {
            return m_gateway->deck(deckNumber);
        });
    };
    m_httpServer->route("/api/v1/decks/<arg>", deckRoute);

    const auto controlRoute =
            [this, authorizeRequest, forbiddenMessage, requestTooLarge, idempotencyKeyFor](
                    const QHttpServerRequest& request) {
        const AuthorizationResult auth = authorizeRequest(request);
        if (!auth.authorized) {
            if (auth.forbidden) {
                return forbiddenResponse(request, forbiddenMessage(auth.missingScopes));
            }
            return unauthorizedResponse(request);
        }
        if (writeTokenRequiresTls(auth, request)) {
            return tlsRequiredResponse(request);
        }
        if (request.method() != QHttpServerRequest::Method::Post) {
            return methodNotAllowedResponse(request);
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

    const auto autoDjRoute =
            [this, authorizeRequest, forbiddenMessage, requestTooLarge, idempotencyKeyFor](
                    const QHttpServerRequest& request) {
        const AuthorizationResult auth = authorizeRequest(request);
        if (!auth.authorized) {
            if (auth.forbidden) {
                return forbiddenResponse(request, forbiddenMessage(auth.missingScopes));
            }
            return unauthorizedResponse(request);
        }
        if (writeTokenRequiresTls(auth, request)) {
            return tlsRequiredResponse(request);
        }
        if (request.method() == QHttpServerRequest::Method::Get) {
            return invokeGateway(request, [this]() {
                return m_gateway->autoDjStatus();
            });
        }
        if (request.method() != QHttpServerRequest::Method::Post) {
            return methodNotAllowedResponse(request);
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

    const auto playlistsRoute =
            [this, authorizeRequest, forbiddenMessage, requestTooLarge, idempotencyKeyFor](
                    const QHttpServerRequest& request) {
        const AuthorizationResult auth = authorizeRequest(request);
        if (!auth.authorized) {
            if (auth.forbidden) {
                return forbiddenResponse(request, forbiddenMessage(auth.missingScopes));
            }
            return unauthorizedResponse(request);
        }
        if (writeTokenRequiresTls(auth, request)) {
            return tlsRequiredResponse(request);
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
        QHttpServerResponder& responder) {
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
        writeResponder(&responder,
                jsonResponse(request, QJsonObject{{"error", message}},
                        QHttpServerResponse::StatusCode::TooManyRequests,
                        requestId));
        return;
    }

    QHttpServerResponse response(QHttpServerResponse::StatusCode::Ok);
    RestHeaders headers = response.headers();
    appendHeader(&headers,
            QByteArrayLiteral(kContentTypeHeader),
            QByteArrayLiteral(kEventStreamContentType));
    appendHeader(&headers,
            QByteArrayLiteral(kCacheControlHeader),
            QByteArrayLiteral(kNoCacheValue));
    appendHeader(&headers,
            QByteArrayLiteral(kConnectionHeader),
            QByteArrayLiteral(kKeepAliveValue));
    appendHeader(&headers,
            QByteArrayLiteral(kAccelBufferingHeader),
            QByteArrayLiteral(kAccelBufferingDisabled));
    if (m_tlsActive && requestIsSecure(request)) {
        appendHeader(&headers,
                QByteArrayLiteral(kStrictTransportSecurityHeader),
                QByteArrayLiteral(kStrictTransportSecurityValue));
    }
    const QString allowedOrigin = allowedCorsOrigin(request);
    if (!allowedOrigin.isEmpty()) {
        appendHeader(&headers,
                QByteArrayLiteral(kCorsAllowOriginHeader),
                allowedOrigin.toUtf8());
    }
    setResponseHeaders(&response, headers);
    if (!writeResponder(&responder, response)) {
        return;
    }

    const quint64 clientId = ++m_streamClientCounter;
    QJsonObject payload = fetchStatusPayload();
    const QJsonObject delta = statusDelta(QJsonObject{}, payload);
    StreamClient client{clientId, std::move(responder), payload};
    auto result = m_streamClients.emplace(clientId, std::move(client));
    if (!result.second) {
        return;
    }
    if (!sendStatusStreamEvent(delta, &result.first->second.responder)) {
        m_streamClients.erase(clientId);
    }

}

bool RestServer::sendStatusStreamEvent(
        const QJsonObject& payload,
        QHttpServerResponder* responder) const {
    const QByteArray event = formatSseEvent(payload);
    return writeResponder(responder, event);
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
    for (auto it = m_streamClients.begin(); it != m_streamClients.end();) {
        StreamClient& client = it->second;
        const QJsonObject delta = statusDelta(client.lastPayload, payload);
        if (!sendStatusStreamEvent(delta, &client.responder)) {
            it = m_streamClients.erase(it);
            continue;
        }
        client.lastPayload = payload;
        ++it;
    }
}

void RestServer::addCorsHeaders(
        RestHeaders* headers,
        const QHttpServerRequest& request,
        bool includeAllowHeaders) const {
    const QString allowedOrigin = allowedCorsOrigin(request);
    if (allowedOrigin.isEmpty()) {
        return;
    }
    appendHeader(headers,
            QByteArrayLiteral(kCorsAllowOriginHeader),
            allowedOrigin.toUtf8());
    if (includeAllowHeaders) {
        appendHeader(headers,
                QByteArrayLiteral(kCorsAllowMethodsHeader),
                QByteArrayLiteral("GET, POST, PUT, PATCH, DELETE, OPTIONS"));
        const QByteArray requestedHeaders =
                QByteArray(request.headers().value(QByteArrayLiteral(kCorsRequestHeadersHeader))).trimmed();
        const QByteArray allowHeaders = requestedHeaders.isEmpty()
                ? QByteArrayLiteral("Authorization, Content-Type, Idempotency-Key, X-Request-Id")
                : requestedHeaders;
        appendHeader(headers,
                QByteArrayLiteral(kCorsAllowHeadersHeader),
                allowHeaders);
    }
}

QString RestServer::allowedCorsOrigin(const QHttpServerRequest& request) const {
    const QByteArray originHeader =
            QByteArray(request.headers().value(QByteArrayLiteral(kCorsOriginHeader))).trimmed();
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
        kLogger.warning() << "REST API write tokens require TLS but HTTPS is disabled";
    }

    if (m_settings.useHttps && !applyTlsConfiguration()) {
        if (m_lastError.isEmpty()) {
            m_lastError = tr("Failed to configure TLS for REST API");
        }
        return false;
    }

    DEBUG_ASSERT(m_settings.portValid);
    QString listenError;
    const auto listenStatus = listenHttpServer(
            m_httpServer.get(),
            m_settings.address,
            static_cast<quint16>(m_settings.port),
            &m_listeningPort,
            &listenError);
    if (listenStatus == HttpServerListenStatus::Ok) {
        kLogger.info()
                << "REST API listening on"
                << m_settings.address.toString()
                << m_listeningPort;
    } else if (listenStatus == HttpServerListenStatus::Failed) {
            kLogger.warning() << "Failed to start REST API listener on"
                              << m_settings.address << m_settings.port << listenError;
            m_lastError = tr("Failed to bind REST API listener on %1:%2")
                                  .arg(m_settings.address.toString())
                                  .arg(m_settings.port);
            if (!listenError.isEmpty()) {
                m_lastError = tr("%1 (%2)").arg(m_lastError, listenError);
            }
            return false;
    } else {
        m_tcpServer = std::make_unique<QTcpServer>();
        if (!m_tcpServer->listen(
                    m_settings.address,
                    static_cast<quint16>(m_settings.port))) {
            kLogger.warning() << "Failed to start REST API listener on"
                              << m_settings.address << m_settings.port
                              << m_tcpServer->errorString();
            m_lastError = tr("Failed to bind REST API listener on %1:%2")
                                  .arg(m_settings.address.toString())
                                  .arg(m_settings.port);
            const QString serverError = m_tcpServer->errorString();
            if (!serverError.isEmpty()) {
                m_lastError = tr("%1 (%2)").arg(m_lastError, serverError);
            }
            return false;
        }
        const auto bindServer = [this]() {
            if constexpr (std::is_same_v<decltype(m_httpServer->bind(m_tcpServer.get())), bool>) {
                return m_httpServer->bind(m_tcpServer.get());
            } else {
                m_httpServer->bind(m_tcpServer.get());
                return true;
            }
        };
        if (!bindServer()) {
            const QString serverError = m_tcpServer->errorString();
            m_lastError = tr("Failed to bind REST API listener on %1:%2")
                                  .arg(m_settings.address.toString())
                                  .arg(m_settings.port);
            if (!serverError.isEmpty()) {
                m_lastError = tr("%1 (%2)").arg(m_lastError, serverError);
            }
            return false;
        }
        m_listeningPort = m_tcpServer->serverPort();
        m_tcpServer.release();
        kLogger.info()
                << "REST API listening on"
                << m_settings.address.toString()
                << m_listeningPort;
    }

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
        stopHttpServer(m_httpServer.get());
        m_httpServer.reset();
    }
    if (m_tcpServer) {
        m_tcpServer->close();
        m_tcpServer.reset();
    }
    kLogger.info() << "REST API server stopped";
}

} // namespace mixxx::network::rest

#endif // MIXXX_HAS_HTTP_SERVER
