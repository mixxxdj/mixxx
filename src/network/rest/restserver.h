#pragma once

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QDateTime>
#include <QHostAddress>
#include <QHttpServer>
#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonObject>
#include <QList>
#include <QPointer>
#include <QSslConfiguration>
#include <QThread>
#include <QString>
#include <QStringList>
#include <functional>
#include <memory>
#include <optional>

#include "util/logger.h"
#include "util/ratelimitedlogger.h"

class QObject;

namespace mixxx::network::rest {

class RestApiProvider;
class CertificateGenerator;

class RestServer : public QObject {
    Q_OBJECT

  public:
    enum class AccessPolicy {
        Status,
        Control,
    };

    struct Token {
        QString value;
        QString description;
        QString permission; // e.g. "read", "full"
        QDateTime createdUtc;
        std::optional<QDateTime> expiresUtc;

        friend bool operator==(const Token& lhs, const Token& rhs) {
            return lhs.value == rhs.value &&
                    lhs.description == rhs.description &&
                    lhs.permission == rhs.permission &&
                    lhs.createdUtc == rhs.createdUtc &&
                    lhs.expiresUtc == rhs.expiresUtc;
        }
    };

    struct AuthorizationResult {
        bool authorized{false};
        bool forbidden{false};
        bool usedReadOnlyToken{false};
        QString tokenValue;
    };

    struct Settings {
        bool enabled{false};
        QString host;
        bool hostValid{true};
        QHostAddress address{QHostAddress::LocalHost};
        int port{0};
        bool portValid{true};
        bool useHttps{false};
        bool autoGenerateCertificate{false};
        bool requireTls{false};
        QString certificatePath;
        QString privateKeyPath;
        QList<Token> tokens;
        int maxRequestBytes{64 * 1024};
        QStringList corsAllowlist;

        friend bool operator==(const Settings& lhs, const Settings& rhs) {
            return lhs.enabled == rhs.enabled &&
                    lhs.host == rhs.host &&
                    lhs.hostValid == rhs.hostValid &&
                    lhs.address == rhs.address &&
                    lhs.port == rhs.port &&
                    lhs.portValid == rhs.portValid &&
                    lhs.useHttps == rhs.useHttps &&
                    lhs.autoGenerateCertificate == rhs.autoGenerateCertificate &&
                    lhs.requireTls == rhs.requireTls &&
                    lhs.certificatePath == rhs.certificatePath &&
                    lhs.privateKeyPath == rhs.privateKeyPath &&
                    lhs.tokens == rhs.tokens &&
                    lhs.maxRequestBytes == rhs.maxRequestBytes &&
                    lhs.corsAllowlist == rhs.corsAllowlist;
        }

        friend bool operator!=(const Settings& lhs, const Settings& rhs) {
            return !(lhs == rhs);
        }
    };

    struct TlsResult {
        bool success{false};
        bool generated{false};
        QString error;
        QString certificatePath;
        QString privateKeyPath;
        QSslConfiguration configuration;
    };

    explicit RestServer(RestApiProvider* gateway, QObject* parent = nullptr);
    ~RestServer() override;

    static TlsResult prepareTlsConfiguration(
            const Settings& settings,
            CertificateGenerator* certificateGenerator);

    bool start(
            const Settings& settings,
            const std::optional<TlsResult>& tlsConfiguration = std::nullopt,
            QString* error = nullptr);
    void stop();

    bool isRunning() const {
        return m_isRunning;
    }

    QString lastError() const {
        return m_lastError;
    }

  signals:
    void started(quint16 port);
    void stopped();

  private:
    QHttpServerResponse invokeGateway(
            const QHttpServerRequest& request,
            const std::function<QHttpServerResponse()>& action) const;
    QHttpServerResponse unauthorizedResponse(const QHttpServerRequest& request) const;
    QHttpServerResponse forbiddenResponse(
            const QHttpServerRequest& request,
            const QString& message) const;
    QHttpServerResponse badRequestResponse(
            const QHttpServerRequest& request, const QString& message) const;
    QHttpServerResponse unsupportedMediaTypeResponse(
            const QHttpServerRequest& request, const QString& message) const;
    QHttpServerResponse payloadTooLargeResponse(const QHttpServerRequest& request) const;
    QHttpServerResponse methodNotAllowedResponse(const QHttpServerRequest& request) const;
    QHttpServerResponse serviceUnavailableResponse(const QHttpServerRequest* request) const;
    QHttpServerResponse jsonResponse(
            const QHttpServerRequest& request,
            const QJsonObject& body,
            QHttpServerResponse::StatusCode status) const;

    QHttpServerResponse tlsRequiredResponse(const QHttpServerRequest& request) const;
    bool applyTlsConfiguration();
    AuthorizationResult authorize(
            const QHttpServerRequest& request,
            AccessPolicy policy) const;
    bool controlRouteRequiresTls(const QHttpServerRequest& request) const;
    void registerRoutes();
    void logRouteError(
            const QHttpServerRequest& request,
            QHttpServerResponse::StatusCode status,
            const QString& message) const;
    QString requestDescription(const QHttpServerRequest& request) const;
    void addCorsHeaders(
            QHttpServerResponse* response,
            const QHttpServerRequest& request,
            bool includeAllowHeaders) const;
    QString allowedCorsOrigin(const QHttpServerRequest& request) const;
    bool startOnThread();
    void stopOnThread();

    std::unique_ptr<QObject> m_threadObject;
    std::unique_ptr<QHttpServer> m_httpServer;
    std::optional<TlsResult> m_tlsConfiguration;
    Settings m_settings;
    QPointer<RestApiProvider> m_gateway;
    QThread m_thread;
    bool m_isRunning;
    bool m_tlsActive{false};
    quint16 m_listeningPort;
    QString m_lastError;
    mutable RateLimitedLogger m_routeErrorLogger;

    static const Logger kLogger;
};

} // namespace mixxx::network::rest

#endif // MIXXX_HAS_HTTP_SERVER
