#include "network/rest/restserver.h"

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaObject>
#include <QSemaphore>
#include <QSslConfiguration>
#include <QSslKey>
#include <QSslSocket>
#include <utility>

#include "moc_restserver.cpp"
#include "network/rest/restapigateway.h"
#include "util/logger.h"
#include "util/thread_affinity.h"

namespace mixxx::network::rest {

namespace {
constexpr auto kAuthHeader = "Authorization";
} // namespace

const Logger RestServer::kLogger("mixxx::network::rest::RestServer");

RestServer::RestServer(RestApiGateway* gateway, QObject* parent)
        : QObject(parent),
          m_gateway(gateway),
          m_isRunning(false),
          m_listeningPort(0) {
    DEBUG_ASSERT(m_gateway);
}

RestServer::~RestServer() {
    stop();
}

bool RestServer::start(const Settings& settings) {
    stop();

    if (m_gateway.isNull()) {
        kLogger.warning() << "REST API gateway is not available; aborting startup";
        return false;
    }

    m_settings = settings;

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
    m_isRunning = false;
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
    QMetaObject::invokeMethod(
            m_gateway,
            [&]() {
                response = action();
            },
            Qt::BlockingQueuedConnection);
    return response;
}

bool RestServer::applyTlsConfiguration() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(m_httpServer.get());

    if (m_settings.certificatePath.isEmpty() || m_settings.privateKeyPath.isEmpty()) {
        kLogger.warning()
                << "TLS is enabled but certificate or private key paths are not configured";
        return false;
    }

#if QT_CONFIG(ssl)
    QFile certificateFile(m_settings.certificatePath);
    QFile privateKeyFile(m_settings.privateKeyPath);
    if (!certificateFile.open(QIODevice::ReadOnly)) {
        kLogger.warning() << "Failed to open REST TLS certificate" << m_settings.certificatePath;
        return false;
    }
    if (!privateKeyFile.open(QIODevice::ReadOnly)) {
        kLogger.warning() << "Failed to open REST TLS private key" << m_settings.privateKeyPath;
        return false;
    }

    const auto certificate = QSslCertificate(certificateFile.readAll(), QSsl::Pem);
    const auto privateKey = QSslKey(privateKeyFile.readAll(), QSsl::Rsa);

    if (certificate.isNull() || privateKey.isNull()) {
        kLogger.warning() << "Invalid REST TLS certificate or key";
        return false;
    }

    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setLocalCertificate(certificate);
    sslConfig.setPrivateKey(privateKey);
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    m_httpServer->sslSetup(sslConfig);
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

void RestServer::registerRoutes() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(m_httpServer.get());

    m_httpServer->route("/api/health", [this](const QHttpServerRequest& request) {
        if (!checkAuthorization(request)) {
            return unauthorizedResponse();
        }
        return invokeGateway([this]() {
            return m_gateway->health();
        });
    });

    m_httpServer->route("/api/status", [this](const QHttpServerRequest& request) {
        if (!checkAuthorization(request)) {
            return unauthorizedResponse();
        }
        return invokeGateway([this]() {
            return m_gateway->status();
        });
    });

    m_httpServer->route("/api/control", [this](const QHttpServerRequest& request) {
        if (!checkAuthorization(request)) {
            return unauthorizedResponse();
        }

        const auto document = QJsonDocument::fromJson(request.body());
        if (!document.isObject()) {
            return badRequestResponse(QStringLiteral("Expected JSON request body"));
        }
        const auto body = document.object();
        const auto group = body.value("group").toString();
        const auto key = body.value("key").toString();
        if (group.isEmpty() || key.isEmpty()) {
            return badRequestResponse(QStringLiteral("Missing group or key in request body"));
        }
        const auto valueVariant = body.value("value");
        const std::optional<double> controlValue = valueVariant.isUndefined()
                ? std::optional<double>{}
                : std::optional<double>{valueVariant.toDouble()};
        return invokeGateway([this, group, key, controlValue]() {
            return m_gateway->control(group, key, controlValue);
        });
    });

    m_httpServer->route("/api/autodj/<arg>", [this](const QString& action, const QHttpServerRequest& request) {
        if (!checkAuthorization(request)) {
            return unauthorizedResponse();
        }
        return invokeGateway([this, action]() {
            return m_gateway->autoDj(action);
        });
    });

    m_httpServer->route("/api/playlists", [this](const QHttpServerRequest& request) {
        if (!checkAuthorization(request)) {
            return unauthorizedResponse();
        }
        return invokeGateway([this]() {
            return m_gateway->playlists();
        });
    });

    m_httpServer->route("/api/playlists/<arg>/tracks",
            [this](const QString& playlistId, const QHttpServerRequest& request) {
                if (!checkAuthorization(request)) {
                    return unauthorizedResponse();
                }
                bool ok = false;
                const int numericId = playlistId.toInt(&ok);
                if (!ok) {
                    return badRequestResponse(QStringLiteral("Playlist id must be numeric"));
                }
                return invokeGateway([this, numericId]() {
                    return m_gateway->playlistTracks(numericId);
                });
            });
}

bool RestServer::startOnThread() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(m_threadObject.get());

    m_httpServer = std::make_unique<QHttpServer>();
    registerRoutes();

    if (m_settings.tlsEnabled && !applyTlsConfiguration()) {
        return false;
    }

    const auto port = m_httpServer->listen(m_settings.address, m_settings.port);
    if (port == 0) {
        kLogger.warning()
                << "Failed to start REST API listener on" << m_settings.address << m_settings.port;
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
