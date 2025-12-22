#include <gmock/gmock.h>
#include <gtest/gtest.h>

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QDateTime>
#include <QEventLoop>
#include <QFile>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QHostAddress>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTcpServer>
#include <QTemporaryDir>
#include <QUrl>
#include <algorithm>
#include <limits>

#include "network/rest/certificategenerator.h"
#include "network/rest/restapigateway.h"
#include "network/rest/restscopes.h"
#include "network/rest/restserver.h"
#include "network/rest/restservervalidator.h"
#include "preferences/restserversettings.h"
#include "test/mixxxtest.h"

namespace mixxx::network::rest {
namespace {

RestServer::Settings baseSettings(int port) {
    RestServer::Settings settings;
    settings.enabled = true;
    settings.host = QHostAddress(QHostAddress::LocalHost).toString();
    settings.hostValid = true;
    settings.address = QHostAddress::LocalHost;
    settings.port = port;
    settings.portValid = port > 0 && port <= std::numeric_limits<quint16>::max();
    settings.allowUnauthenticated = true;
    return settings;
}

class StubRestApiGateway : public RestApiProvider {
  public:
    using RestApiProvider::RestApiProvider;

    QHttpServerResponse health() const override {
        return jsonResponse("health");
    }

    QHttpServerResponse ready() const override {
        return jsonResponse("ready");
    }

    QHttpServerResponse status() const override {
        return jsonResponse("status");
    }

    QJsonObject statusPayload() const override {
        return QJsonObject{{"message", QStringLiteral("status")}};
    }

    QHttpServerResponse deck(int /*deckNumber*/) const override {
        return jsonResponse("deck");
    }

    QHttpServerResponse decks() const override {
        return jsonResponse("decks");
    }

    QHttpServerResponse control(const QJsonObject&) const override {
        return jsonResponseWithCounter("control");
    }

    QHttpServerResponse autoDjStatus() const override {
        return jsonResponse("autodjstatus");
    }

    QHttpServerResponse autoDj(const QJsonObject&) const override {
        return jsonResponseWithCounter("autodj");
    }

    QHttpServerResponse playlists(const std::optional<int>&) const override {
        return jsonResponse("playlists");
    }

    QHttpServerResponse playlistCommand(const QJsonObject&) const override {
        return jsonResponseWithCounter("playlistcommand");
    }

    QHttpServerResponse withIdempotencyCache(
            const QString& token,
            const QString& idempotencyKey,
            const QString& endpoint,
            const std::function<QHttpServerResponse()>& handler) const override {
        if (idempotencyKey.isEmpty()) {
            return handler();
        }
        const QString cacheKey =
                QStringLiteral("%1\n%2\n%3").arg(token, idempotencyKey, endpoint);
        const auto cached = m_idempotencyCache.constFind(cacheKey);
        if (cached != m_idempotencyCache.constEnd()) {
            return buildResponse(cached.value());
        }
        QHttpServerResponse response = handler();
        if (m_lastResponse.has_value()) {
            m_idempotencyCache.insert(cacheKey, m_lastResponse.value());
        }
        return response;
    }

  private:
    struct CachedResponse {
        QHttpServerResponse::StatusCode status{QHttpServerResponse::StatusCode::Ok};
        QByteArray body;
        QByteArray mimeType;
    };

    QHttpServerResponse jsonResponse(const QString& message) const {
        CachedResponse payload;
        payload.status = QHttpServerResponse::StatusCode::Ok;
        payload.body = QJsonDocument(QJsonObject{{"message", message}}).toJson(
                QJsonDocument::Compact);
        payload.mimeType = QByteArrayLiteral("application/json");
        m_lastResponse = payload;
        return buildResponse(payload);
    }

    QHttpServerResponse buildResponse(const CachedResponse& payload) const {
        return QHttpServerResponse(payload.mimeType, payload.body, payload.status);
    }

    QHttpServerResponse jsonResponseWithCounter(const QString& prefix) const {
        return jsonResponse(QStringLiteral("%1-%2").arg(prefix).arg(++m_responseCounter));
    }

    mutable int m_responseCounter{0};
    mutable std::optional<CachedResponse> m_lastResponse;
    mutable QHash<QString, CachedResponse> m_idempotencyCache;
};

struct HttpResult {
    int status{0};
    QByteArray body;
};

HttpResult sendRequest(
        QNetworkAccessManager* manager,
        const QUrl& url,
        const QByteArray& method,
        const QByteArray& body = QByteArray(),
        const QList<QPair<QByteArray, QByteArray>>& headers = {}) {
    QNetworkRequest request(url);
    for (const auto& header : headers) {
        request.setRawHeader(header.first, header.second);
    }

    QNetworkReply* reply = manager->sendCustomRequest(request, method, body);
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    HttpResult result;
    result.status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    result.body = reply->readAll();
    reply->deleteLater();
    return result;
}

int findFreePort() {
    QTcpServer server;
    server.listen(QHostAddress::LocalHost, 0);
    return static_cast<int>(server.serverPort());
}

class RestServerSettingsTest : public MixxxTest {
};

TEST_F(RestServerSettingsTest, DefaultsStayWithinRange) {
    RestServerSettings settings(m_pConfig);
    const auto defaults = settings.defaults();
    EXPECT_TRUE(defaults.enableHttp);
    EXPECT_FALSE(defaults.useHttps);
    EXPECT_FALSE(defaults.allowUnauthenticated);
    EXPECT_EQ(RestServerSettings::kDefaultPort, defaults.httpPort);
    EXPECT_EQ(RestServerSettings::kDefaultHttpsPort, defaults.httpsPort);
}

TEST_F(RestServerSettingsTest, OutOfRangePortFallsBackToDefault) {
    RestServerSettings settings(m_pConfig);
    RestServerSettings::Values values = settings.defaults();
    values.httpPort = std::numeric_limits<int>::max();
    values.httpsPort = 0;
    settings.set(values);

    const auto persisted = settings.get();
    EXPECT_EQ(RestServerSettings::kDefaultPort, persisted.httpPort);
    EXPECT_EQ(RestServerSettings::kDefaultHttpsPort, persisted.httpsPort);
}

TEST_F(RestServerSettingsTest, DuplicatePortsAreAdjusted) {
    RestServerSettings settings(m_pConfig);
    RestServerSettings::Values values = settings.defaults();
    values.useHttps = true;
    values.httpPort = RestServerSettings::kDefaultPort;
    values.httpsPort = RestServerSettings::kDefaultPort;
    settings.set(values);

    const auto persisted = settings.get();
    EXPECT_NE(persisted.httpPort, persisted.httpsPort);
}

TEST(RestServerValidatorTest, RejectsInvalidHost) {
    RestServerValidator validator(RestServer::Settings{}, false, nullptr);

    const int port = findFreePort();
    RestServer::Settings settings = baseSettings(port);
    settings.hostValid = false;

    const auto result = validator.validate(settings);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error.isEmpty());
}

TEST(RestServerValidatorTest, DetectsUnavailablePort) {
    QTcpServer probe;
    ASSERT_TRUE(probe.listen(QHostAddress::LocalHost, 0));

    RestServerValidator validator(RestServer::Settings{}, false, nullptr);
    RestServer::Settings settings = baseSettings(static_cast<int>(probe.serverPort()));

    const auto result = validator.validate(settings);
    EXPECT_FALSE(result.success);
    EXPECT_THAT(result.error.toStdString(), ::testing::HasSubstr("unavailable"));
}

TEST(RestServerValidatorTest, CapturesTlsConfigurationErrors) {
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    const QString certificatePath = tempDir.filePath("cert.pem");
    QFile certFile(certificatePath);
    ASSERT_TRUE(certFile.open(QIODevice::WriteOnly));
    certFile.write("invalid");
    certFile.close();

    const QString keyPath = tempDir.filePath("key.pem");
    QFile keyFile(keyPath);
    ASSERT_TRUE(keyFile.open(QIODevice::WriteOnly));
    keyFile.write("invalid");
    keyFile.close();

    CertificateGenerator generator(tempDir.path());
    RestServerValidator validator(RestServer::Settings{}, false, &generator);

    RestServer::Settings settings = baseSettings(RestServerSettings::kDefaultPort);
    settings.useHttps = true;
    settings.certificatePath = certificatePath;
    settings.privateKeyPath = keyPath;

    const auto result = validator.validate(settings);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(QObject::tr("TLS configuration failed"), result.error);
    EXPECT_FALSE(result.tlsError.isEmpty());
}

TEST(RestServerRoutesTest, EnforcesAuthorization) {
    StubRestApiGateway gateway;
    RestServer server(&gateway);
    const int port = findFreePort();

    RestServer::Settings settings = baseSettings(port);
    RestServer::Token token;
    token.value = QStringLiteral("secret-token");
    token.scopes = scopes::allScopes();
    token.createdUtc = QDateTime::currentDateTimeUtc();
    settings.tokens.append(token);

    QString error;
    ASSERT_TRUE(server.start(settings, std::nullopt, &error)) << error.toStdString();

    QNetworkAccessManager manager;
    const QUrl url(QStringLiteral("http://127.0.0.1:%1/api/v1/health").arg(port));

    const auto unauthorized = sendRequest(&manager, url, "GET");
    EXPECT_EQ(static_cast<int>(QHttpServerResponse::StatusCode::Unauthorized), unauthorized.status);

    const auto authorized = sendRequest(
            &manager,
            url,
            "GET",
            QByteArray(),
            {{"Authorization", "Bearer secret-token"}});
    EXPECT_EQ(static_cast<int>(QHttpServerResponse::StatusCode::Ok), authorized.status);

    server.stop();
}

TEST(RestServerRoutesTest, VersionedPathsServeRequests) {
    StubRestApiGateway gateway;
    RestServer server(&gateway);
    const int port = findFreePort();

    RestServer::Settings settings = baseSettings(port);

    QString error;
    ASSERT_TRUE(server.start(settings, std::nullopt, &error)) << error.toStdString();

    QNetworkAccessManager manager;

    const auto expectOk = [&](const QString& path,
                              const QByteArray& method,
                              const QByteArray& body = QByteArray(),
                              const QList<QPair<QByteArray, QByteArray>>& headers = {}) {
        const QUrl url(QStringLiteral("http://127.0.0.1:%1%2").arg(port).arg(path));
        const auto response = sendRequest(&manager, url, method, body, headers);
        EXPECT_EQ(static_cast<int>(QHttpServerResponse::StatusCode::Ok), response.status)
                << path.toStdString();
    };

    expectOk("/api/v1/health", "GET");
    expectOk("/api/v1/schema", "GET");
    expectOk("/api/v1/ready", "GET");
    expectOk("/api/v1/status", "GET");
    expectOk("/api/v1/decks", "GET");
    expectOk("/api/v1/decks/1", "GET");
    expectOk("/api/v1/autodj", "GET");
    expectOk("/api/v1/playlists", "GET");

    const QByteArray controlBody = QJsonDocument(QJsonObject{{"group", "[Master]"}, {"key", "volume"}})
                                           .toJson(QJsonDocument::Compact);
    expectOk("/api/v1/control",
            "POST",
            controlBody,
            {{"Content-Type", "application/json"}});

    const QByteArray autoDjBody = QJsonDocument(QJsonObject{{"enabled", true}})
                                          .toJson(QJsonDocument::Compact);
    expectOk("/api/v1/autodj",
            "POST",
            autoDjBody,
            {{"Content-Type", "application/json"}});

    const QByteArray playlistBody = QJsonDocument(QJsonObject{{"action", "create"}, {"name", "Test"}})
                                            .toJson(QJsonDocument::Compact);
    expectOk("/api/v1/playlists",
            "POST",
            playlistBody,
            {{"Content-Type", "application/json"}});

    server.stop();
}

TEST(RestServerRoutesTest, SchemaEndpointDescribesRoutesAndActions) {
    StubRestApiGateway gateway;
    RestServer server(&gateway);
    const int port = findFreePort();

    RestServer::Settings settings = baseSettings(port);

    QString error;
    ASSERT_TRUE(server.start(settings, std::nullopt, &error)) << error.toStdString();

    QNetworkAccessManager manager;
    const QUrl schemaUrl(QStringLiteral("http://127.0.0.1:%1/api/v1/schema").arg(port));
    const auto response = sendRequest(&manager, schemaUrl, "GET");

    EXPECT_EQ(static_cast<int>(QHttpServerResponse::StatusCode::Ok), response.status);

    QJsonParseError parseError;
    const auto doc = QJsonDocument::fromJson(response.body, &parseError);
    ASSERT_EQ(QJsonParseError::NoError, parseError.error);
    ASSERT_TRUE(doc.isObject());

    const auto root = doc.object();
    const auto links = root.value("links").toObject();
    EXPECT_EQ(QStringLiteral("/api/v1/health"), links.value("health").toString());
    EXPECT_EQ(QStringLiteral("/api/v1/status"), links.value("status").toString());
    EXPECT_EQ(QStringLiteral("/api/v1/control"), links.value("control").toString());
    EXPECT_EQ(QStringLiteral("/api/v1/autodj"), links.value("autodj").toString());
    EXPECT_EQ(QStringLiteral("/api/v1/playlists"), links.value("playlists").toString());

    const auto actions = root.value("actions").toObject();
    const auto autodjActions = actions.value("autodj").toArray();
    const auto playlistsActions = actions.value("playlists").toArray();
    const auto containsString = [](const QJsonArray& array, const QString& value) {
        return std::any_of(array.begin(), array.end(), [&value](const QJsonValue& entry) {
            return entry.toString() == value;
        });
    };
    EXPECT_TRUE(containsString(autodjActions, QStringLiteral("enable")));
    EXPECT_TRUE(containsString(autodjActions, QStringLiteral("add")));
    EXPECT_TRUE(containsString(playlistsActions, QStringLiteral("create")));
    EXPECT_TRUE(containsString(playlistsActions, QStringLiteral("send_to_autodj")));

    const auto endpoints = root.value("endpoints").toArray();
    const auto findEndpoint = [](const QJsonArray& array, const QString& path) {
        for (const auto& entry : array) {
            const auto obj = entry.toObject();
            if (obj.value("path").toString() == path) {
                return obj;
            }
        }
        return QJsonObject{};
    };
    const auto endpointScopes = [](const QJsonObject& endpoint, const QString& key) {
        return endpoint.value("scopes").toObject().value(key).toArray();
    };

    const auto healthEndpoint = findEndpoint(endpoints, QStringLiteral("/api/v1/health"));
    EXPECT_TRUE(containsString(endpointScopes(healthEndpoint, QStringLiteral("read")),
            QStringLiteral("status:read")));

    const auto controlEndpoint = findEndpoint(endpoints, QStringLiteral("/api/v1/control"));
    EXPECT_TRUE(containsString(endpointScopes(controlEndpoint, QStringLiteral("write")),
            QStringLiteral("control:write")));

    const auto autodjEndpoint = findEndpoint(endpoints, QStringLiteral("/api/v1/autodj"));
    EXPECT_TRUE(containsString(endpointScopes(autodjEndpoint, QStringLiteral("read")),
            QStringLiteral("autodj:read")));
    EXPECT_TRUE(containsString(endpointScopes(autodjEndpoint, QStringLiteral("write")),
            QStringLiteral("autodj:write")));

    const auto playlistsEndpoint = findEndpoint(endpoints, QStringLiteral("/api/v1/playlists"));
    EXPECT_TRUE(containsString(endpointScopes(playlistsEndpoint, QStringLiteral("read")),
            QStringLiteral("playlists:read")));
    EXPECT_TRUE(containsString(endpointScopes(playlistsEndpoint, QStringLiteral("write")),
            QStringLiteral("playlists:write")));

    server.stop();
}

TEST(RestServerRoutesTest, RejectsUnversionedPaths) {
    StubRestApiGateway gateway;
    RestServer server(&gateway);
    const int port = findFreePort();

    RestServer::Settings settings = baseSettings(port);

    QString error;
    ASSERT_TRUE(server.start(settings, std::nullopt, &error)) << error.toStdString();

    QNetworkAccessManager manager;
    const QUrl url(QStringLiteral("http://127.0.0.1:%1/health").arg(port));
    const auto response = sendRequest(&manager, url, "GET");

    EXPECT_EQ(static_cast<int>(QHttpServerResponse::StatusCode::NotFound), response.status);

    server.stop();
}

TEST(RestServerRoutesTest, ReadOnlyTokenAllowsStatusButBlocksControl) {
    StubRestApiGateway gateway;
    RestServer server(&gateway);
    const int port = findFreePort();

    RestServer::Settings settings = baseSettings(port);
    RestServer::Token token;
    token.value = QStringLiteral("ro-token");
    token.scopes = scopes::defaultReadScopes();
    token.createdUtc = QDateTime::currentDateTimeUtc();
    settings.tokens.append(token);

    QString error;
    ASSERT_TRUE(server.start(settings, std::nullopt, &error)) << error.toStdString();

    QNetworkAccessManager manager;
    const QUrl statusUrl(QStringLiteral("http://127.0.0.1:%1/api/v1/status").arg(port));
    const QUrl controlUrl(QStringLiteral("http://127.0.0.1:%1/api/v1/control").arg(port));

    const auto statusResponse = sendRequest(
            &manager,
            statusUrl,
            "GET",
            QByteArray(),
            {{"Authorization", "Bearer ro-token"}});
    EXPECT_EQ(static_cast<int>(QHttpServerResponse::StatusCode::Ok), statusResponse.status);

    const QByteArray controlBody = QJsonDocument(QJsonObject{{"group", "[Master]"}, {"key", "volume"}})
                                           .toJson(QJsonDocument::Compact);
    const auto controlResponse = sendRequest(
            &manager,
            controlUrl,
            "POST",
            controlBody,
            {{"Authorization", "Bearer ro-token"}, {"Content-Type", "application/json"}});
    EXPECT_EQ(static_cast<int>(QHttpServerResponse::StatusCode::Forbidden), controlResponse.status);

    server.stop();
}

TEST(RestServerRoutesTest, RequiresTlsForControlRoutesWhenRequested) {
    StubRestApiGateway gateway;
    RestServer server(&gateway);
    const int port = findFreePort();

    RestServer::Settings settings = baseSettings(port);
    settings.requireTls = true;

    QString error;
    ASSERT_TRUE(server.start(settings, std::nullopt, &error)) << error.toStdString();

    QNetworkAccessManager manager;
    const QUrl controlUrl(QStringLiteral("http://127.0.0.1:%1/api/v1/control").arg(port));
    const QByteArray body = QJsonDocument(QJsonObject{{"group", "[Master]"}, {"key", "volume"}})
                                    .toJson(QJsonDocument::Compact);
    const auto controlResponse = sendRequest(
            &manager,
            controlUrl,
            "POST",
            body,
            {{"Content-Type", "application/json"}});

    EXPECT_EQ(static_cast<int>(QHttpServerResponse::StatusCode::Forbidden), controlResponse.status);

    const QUrl autoDjUrl(QStringLiteral("http://127.0.0.1:%1/api/v1/autodj").arg(port));
    const auto autoDjResponse = sendRequest(
            &manager,
            autoDjUrl,
            "POST",
            QJsonDocument(QJsonObject{{"enabled", true}}).toJson(QJsonDocument::Compact),
            {{"Content-Type", "application/json"}});
    EXPECT_EQ(static_cast<int>(QHttpServerResponse::StatusCode::Forbidden), autoDjResponse.status);

    const QUrl playlistsUrl(QStringLiteral("http://127.0.0.1:%1/api/v1/playlists").arg(port));
    const auto playlistsResponse = sendRequest(
            &manager,
            playlistsUrl,
            "POST",
            QJsonDocument(QJsonObject{{"command", "enable"}}).toJson(QJsonDocument::Compact),
            {{"Content-Type", "application/json"}});
    EXPECT_EQ(static_cast<int>(QHttpServerResponse::StatusCode::Forbidden), playlistsResponse.status);

    server.stop();
}

TEST(RestServerRoutesTest, RejectsInvalidContentTypes) {
    StubRestApiGateway gateway;
    RestServer server(&gateway);
    const int port = findFreePort();

    RestServer::Settings settings = baseSettings(port);

    QString error;
    ASSERT_TRUE(server.start(settings, std::nullopt, &error)) << error.toStdString();

    QNetworkAccessManager manager;
    const QUrl controlUrl(QStringLiteral("http://127.0.0.1:%1/api/v1/control").arg(port));
    const QByteArray body = QJsonDocument(QJsonObject{{"group", "[Master]"}, {"key", "volume"}})
                                    .toJson(QJsonDocument::Compact);

    const auto missingContentType = sendRequest(&manager, controlUrl, "POST", body);
    EXPECT_EQ(static_cast<int>(QHttpServerResponse::StatusCode::UnsupportedMediaType),
            missingContentType.status);

    const auto invalidContentType = sendRequest(
            &manager,
            controlUrl,
            "POST",
            body,
            {{"Content-Type", "text/plain"}});
    EXPECT_EQ(static_cast<int>(QHttpServerResponse::StatusCode::UnsupportedMediaType),
            invalidContentType.status);

    server.stop();
}

TEST(RestServerRoutesTest, ReportsJsonParseErrors) {
    StubRestApiGateway gateway;
    RestServer server(&gateway);
    const int port = findFreePort();

    RestServer::Settings settings = baseSettings(port);

    QString error;
    ASSERT_TRUE(server.start(settings, std::nullopt, &error)) << error.toStdString();

    QNetworkAccessManager manager;
    const QUrl controlUrl(QStringLiteral("http://127.0.0.1:%1/api/v1/control").arg(port));
    const QByteArray invalidJson("{");

    QJsonParseError parseError;
    QJsonDocument::fromJson(invalidJson, &parseError);

    const auto response = sendRequest(
            &manager,
            controlUrl,
            "POST",
            invalidJson,
            {{"Content-Type", "application/json"}});

    EXPECT_EQ(static_cast<int>(QHttpServerResponse::StatusCode::BadRequest), response.status);

    QJsonParseError responseError;
    const auto responseDoc = QJsonDocument::fromJson(response.body, &responseError);
    ASSERT_EQ(responseError.error, QJsonParseError::NoError);
    ASSERT_TRUE(responseDoc.isObject());
    const QString errorMessage = responseDoc.object().value("error").toString();
    EXPECT_THAT(errorMessage.toStdString(),
            ::testing::HasSubstr(parseError.errorString().toStdString()));

    server.stop();
}

TEST(RestServerRoutesTest, IdempotencyKeyReusesPostResponse) {
    StubRestApiGateway gateway;
    RestServer server(&gateway);
    const int port = findFreePort();

    RestServer::Settings settings = baseSettings(port);
    RestServer::Token tokenA;
    tokenA.value = QStringLiteral("token-a");
    tokenA.scopes = scopes::allScopes();
    tokenA.createdUtc = QDateTime::currentDateTimeUtc();
    settings.tokens.append(tokenA);
    RestServer::Token tokenB;
    tokenB.value = QStringLiteral("token-b");
    tokenB.scopes = scopes::allScopes();
    tokenB.createdUtc = QDateTime::currentDateTimeUtc();
    settings.tokens.append(tokenB);

    QString error;
    ASSERT_TRUE(server.start(settings, std::nullopt, &error)) << error.toStdString();

    QNetworkAccessManager manager;
    const QUrl controlUrl(QStringLiteral("http://127.0.0.1:%1/api/v1/control").arg(port));
    const QByteArray body = QJsonDocument(QJsonObject{{"group", "[Master]"}, {"key", "volume"}})
                                    .toJson(QJsonDocument::Compact);

    const auto first = sendRequest(
            &manager,
            controlUrl,
            "POST",
            body,
            {{"Authorization", "Bearer token-a"},
                    {"Content-Type", "application/json"},
                    {"Idempotency-Key", "repeat-me"}});
    ASSERT_EQ(static_cast<int>(QHttpServerResponse::StatusCode::Ok), first.status);

    const auto duplicate = sendRequest(
            &manager,
            controlUrl,
            "POST",
            body,
            {{"Authorization", "Bearer token-a"},
                    {"Content-Type", "application/json"},
                    {"Idempotency-Key", "repeat-me"}});
    EXPECT_EQ(static_cast<int>(QHttpServerResponse::StatusCode::Ok), duplicate.status);
    EXPECT_EQ(first.body, duplicate.body);

    const auto otherToken = sendRequest(
            &manager,
            controlUrl,
            "POST",
            body,
            {{"Authorization", "Bearer token-b"},
                    {"Content-Type", "application/json"},
                    {"Idempotency-Key", "repeat-me"}});
    EXPECT_EQ(static_cast<int>(QHttpServerResponse::StatusCode::Ok), otherToken.status);
    EXPECT_NE(first.body, otherToken.body);

    const auto otherKey = sendRequest(
            &manager,
            controlUrl,
            "POST",
            body,
            {{"Authorization", "Bearer token-a"},
                    {"Content-Type", "application/json"},
                    {"Idempotency-Key", "other-key"}});
    EXPECT_EQ(static_cast<int>(QHttpServerResponse::StatusCode::Ok), otherKey.status);
    EXPECT_NE(first.body, otherKey.body);

    server.stop();
}

} // namespace
} // namespace mixxx::network::rest

#endif // MIXXX_HAS_HTTP_SERVER
