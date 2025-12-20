#pragma once

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QDateTime>
#include <QHostAddress>
#include <QHttpServer>
#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QHttpServerResponder>
#include <QJsonObject>
#include <QList>
#include <QPointer>
#include <QSslConfiguration>
#include <QThread>
#include <QTimer>
#include <QString>
#include <QStringList>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>

#include "util/logger.h"
#include "util/ratelimitedlogger.h"

class QObject;

namespace mixxx::network::rest {

class RestApiProvider;
class CertificateGenerator;

class RestServer : public QObject {
    Q_OBJECT

  public:
    struct Token {
        QString value;
        QString description;
        QStringList scopes;
        QDateTime createdUtc;
        std::optional<QDateTime> expiresUtc;

        friend bool operator==(const Token& lhs, const Token& rhs) {
            return lhs.value == rhs.value &&
                    lhs.description == rhs.description &&
                    lhs.scopes == rhs.scopes &&
                    lhs.createdUtc == rhs.createdUtc &&
                    lhs.expiresUtc == rhs.expiresUtc;
        }
    };

    struct AuthorizationResult {
        bool authorized{false};
        bool forbidden{false};
        bool usedReadOnlyToken{false};
        QString tokenValue;
        QString tokenDescription;
        QStringList missingScopes;
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
        bool allowUnauthenticated{false};
        QString certificatePath;
        QString privateKeyPath;
        QList<Token> tokens;
        int maxRequestBytes{64 * 1024};
        QStringList corsAllowlist;
        bool streamEnabled{false};
        int streamIntervalMs{1000};
        int streamMaxClients{5};

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
                    lhs.allowUnauthenticated == rhs.allowUnauthenticated &&
                    lhs.certificatePath == rhs.certificatePath &&
                    lhs.privateKeyPath == rhs.privateKeyPath &&
                    lhs.tokens == rhs.tokens &&
                    lhs.maxRequestBytes == rhs.maxRequestBytes &&
                    lhs.corsAllowlist == rhs.corsAllowlist &&
                    lhs.streamEnabled == rhs.streamEnabled &&
                    lhs.streamIntervalMs == rhs.streamIntervalMs &&
                    lhs.streamMaxClients == rhs.streamMaxClients;
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
            const std::function<QHttpServerResponse()>& action,
            const QString& requestId = QString()) const;
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
            QHttpServerResponse::StatusCode status,
            const QString& requestId) const;

    QHttpServerResponse tlsRequiredResponse(const QHttpServerRequest& request) const;
    bool applyTlsConfiguration();
    AuthorizationResult authorize(
            const QHttpServerRequest& request,
            const QStringList& requiredScopes) const;
    bool controlRouteRequiresTls(const QHttpServerRequest& request) const;
    void registerRoutes();
    void addStatusStreamClient(
            const QHttpServerRequest& request,
            QHttpServerResponder&& responder);
    void pushStatusStreamUpdate();
    bool sendStatusStreamEvent(
            const QJsonObject& payload,
            QHttpServerResponder* responder) const;
    QJsonObject fetchStatusPayload() const;
    QJsonObject statusDelta(const QJsonObject& previous, const QJsonObject& current) const;
    void logRouteError(
            const QHttpServerRequest& request,
            QHttpServerResponse::StatusCode status,
            const QString& message,
            const QString& requestId) const;
    QString requestDescription(const QHttpServerRequest& request) const;
    QString requestIdFor(const QHttpServerRequest& request) const;
    void addCorsHeaders(
            QHttpServerResponse* response,
            const QHttpServerRequest& request,
            bool includeAllowHeaders) const;
    QString allowedCorsOrigin(const QHttpServerRequest& request) const;
    bool startOnThread();
    void stopOnThread();

    struct StreamClient {
        quint64 id;
        QHttpServerResponder responder;
        QJsonObject lastPayload;
    };

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
    quint64 m_streamClientCounter{0};
    std::unordered_map<quint64, StreamClient> m_streamClients;
    QTimer m_streamTimer;
    mutable RateLimitedLogger m_routeErrorLogger;
    mutable RateLimitedLogger m_authFailureLogger;

    static const Logger kLogger;
    static const Logger kAuditLogger;
};

} // namespace mixxx::network::rest

#endif // MIXXX_HAS_HTTP_SERVER
