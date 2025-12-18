#pragma once

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QHostAddress>
#include <QHttpServer>
#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonObject>
#include <QPointer>
#include <QThread>
#include <functional>
#include <memory>
#include <optional>

#include "util/logger.h"

class QObject;

namespace mixxx::network::rest {

class RestApiGateway;

class RestServer : public QObject {
    Q_OBJECT

  public:
    struct Settings {
        bool enabled{false};
        QHostAddress address{QHostAddress::LocalHost};
        quint16 port{0};
        bool tlsEnabled{false};
        QString certificatePath;
        QString privateKeyPath;
        QString authToken;

        friend bool operator==(const Settings& lhs, const Settings& rhs) {
            return lhs.enabled == rhs.enabled &&
                    lhs.address == rhs.address &&
                    lhs.port == rhs.port &&
                    lhs.tlsEnabled == rhs.tlsEnabled &&
                    lhs.certificatePath == rhs.certificatePath &&
                    lhs.privateKeyPath == rhs.privateKeyPath &&
                    lhs.authToken == rhs.authToken;
        }

        friend bool operator!=(const Settings& lhs, const Settings& rhs) {
            return !(lhs == rhs);
        }
    };

    explicit RestServer(RestApiGateway* gateway, QObject* parent = nullptr);
    ~RestServer() override;

    bool start(const Settings& settings);
    void stop();

    bool isRunning() const {
        return m_isRunning;
    }

  signals:
    void started(quint16 port);
    void stopped();

  private:
    QHttpServerResponse invokeGateway(const std::function<QHttpServerResponse()>& action) const;
    QHttpServerResponse unauthorizedResponse() const;
    QHttpServerResponse badRequestResponse(const QString& message) const;
    QHttpServerResponse serviceUnavailableResponse() const;
    QHttpServerResponse jsonResponse(
            const QJsonObject& body,
            QHttpServerResponse::StatusCode status) const;

    bool applyTlsConfiguration();
    bool checkAuthorization(const QHttpServerRequest& request) const;
    void registerRoutes();
    bool startOnThread();
    void stopOnThread();

    std::unique_ptr<QObject> m_threadObject;
    std::unique_ptr<QHttpServer> m_httpServer;
    Settings m_settings;
    QPointer<RestApiGateway> m_gateway;
    QThread m_thread;
    bool m_isRunning;
    quint16 m_listeningPort;

    static const Logger kLogger;
};

} // namespace mixxx::network::rest

#endif // MIXXX_HAS_HTTP_SERVER
