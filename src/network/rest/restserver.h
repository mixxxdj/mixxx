#pragma once

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QHostAddress>
#include <QHttpServer>
#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonObject>
#include <QPointer>
#include <QSslConfiguration>
#include <QThread>
#include <functional>
#include <memory>
#include <optional>

#include "util/logger.h"

class QObject;

namespace mixxx::network::rest {

class RestApiGateway;
class CertificateGenerator;

class RestServer : public QObject {
    Q_OBJECT

  public:
    struct Settings {
        bool enabled{false};
        QHostAddress address{QHostAddress::LocalHost};
        quint16 port{0};
        bool useHttps{false};
        bool autoGenerateCertificate{false};
        bool requireTls{false};
        QString certificatePath;
        QString privateKeyPath;
        QString authToken;

        friend bool operator==(const Settings& lhs, const Settings& rhs) {
            return lhs.enabled == rhs.enabled &&
                    lhs.address == rhs.address &&
                    lhs.port == rhs.port &&
                    lhs.useHttps == rhs.useHttps &&
                    lhs.autoGenerateCertificate == rhs.autoGenerateCertificate &&
                    lhs.requireTls == rhs.requireTls &&
                    lhs.certificatePath == rhs.certificatePath &&
                    lhs.privateKeyPath == rhs.privateKeyPath &&
                    lhs.authToken == rhs.authToken;
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

    explicit RestServer(RestApiGateway* gateway, QObject* parent = nullptr);
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
    QHttpServerResponse invokeGateway(const std::function<QHttpServerResponse()>& action) const;
    QHttpServerResponse unauthorizedResponse() const;
    QHttpServerResponse badRequestResponse(const QString& message) const;
    QHttpServerResponse methodNotAllowedResponse() const;
    QHttpServerResponse serviceUnavailableResponse() const;
    QHttpServerResponse jsonResponse(
            const QJsonObject& body,
            QHttpServerResponse::StatusCode status) const;

    QHttpServerResponse tlsRequiredResponse() const;
    bool applyTlsConfiguration();
    bool checkAuthorization(const QHttpServerRequest& request) const;
    bool controlRouteRequiresTls(const QHttpServerRequest& request) const;
    void registerRoutes();
    bool startOnThread();
    void stopOnThread();

    std::unique_ptr<QObject> m_threadObject;
    std::unique_ptr<QHttpServer> m_httpServer;
    std::optional<TlsResult> m_tlsConfiguration;
    Settings m_settings;
    QPointer<RestApiGateway> m_gateway;
    QThread m_thread;
    bool m_isRunning;
    bool m_tlsActive{false};
    quint16 m_listeningPort;
    QString m_lastError;

    static const Logger kLogger;
};

} // namespace mixxx::network::rest

#endif // MIXXX_HAS_HTTP_SERVER
