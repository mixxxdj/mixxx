#include <gmock/gmock.h>
#include <gtest/gtest.h>

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QDateTime>
#include <QEventLoop>
#include <QFile>
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
#include <limits>

#include "network/rest/certificategenerator.h"
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

    QHttpServerResponse deck(int /*deckNumber*/) const override {
        return jsonResponse("deck");
    }

    QHttpServerResponse decks() const override {
        return jsonResponse("decks");
    }

    QHttpServerResponse control(const QJsonObject&) const override {
        return jsonResponse("control");
    }

    QHttpServerResponse autoDjStatus() const override {
        return jsonResponse("autodjstatus");
    }

    QHttpServerResponse autoDj(const QJsonObject&) const override {
        return jsonResponse("autodj");
    }

    QHttpServerResponse playlists(const std::optional<int>&) const override {
        return jsonResponse("playlists");
    }

    QHttpServerResponse playlistCommand(const QJsonObject&) const override {
        return jsonResponse("playlistcommand");
    }

  private:
    QHttpServerResponse jsonResponse(const QString& message) const {
        return QHttpServerResponse(
                QHttpServerResponse::StatusCode::Ok,
                QJsonDocument(QJsonObject{{"message", message}}).toJson(
                        QJsonDocument::Compact),
                "application/json");
    }
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

    RestServer::Settings settings = baseSettings(RestServerSettings::kDefaultPort);
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
    token.permission = QStringLiteral("full");
    token.createdUtc = QDateTime::currentDateTimeUtc();
    settings.tokens.append(token);

    QString error;
    ASSERT_TRUE(server.start(settings, std::nullopt, &error)) << error.toStdString();

    QNetworkAccessManager manager;
    const QUrl url(QStringLiteral("http://127.0.0.1:%1/health").arg(port));

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

TEST(RestServerRoutesTest, ReadOnlyTokenAllowsStatusButBlocksControl) {
    StubRestApiGateway gateway;
    RestServer server(&gateway);
    const int port = findFreePort();

    RestServer::Settings settings = baseSettings(port);
    RestServer::Token token;
    token.value = QStringLiteral("ro-token");
    token.permission = QStringLiteral("read");
    token.createdUtc = QDateTime::currentDateTimeUtc();
    settings.tokens.append(token);

    QString error;
    ASSERT_TRUE(server.start(settings, std::nullopt, &error)) << error.toStdString();

    QNetworkAccessManager manager;
    const QUrl statusUrl(QStringLiteral("http://127.0.0.1:%1/status").arg(port));
    const QUrl controlUrl(QStringLiteral("http://127.0.0.1:%1/control").arg(port));

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
    const QUrl controlUrl(QStringLiteral("http://127.0.0.1:%1/control").arg(port));
    const QByteArray body = QJsonDocument(QJsonObject{{"group", "[Master]"}, {"key", "volume"}})
                                    .toJson(QJsonDocument::Compact);
    const auto controlResponse = sendRequest(
            &manager,
            controlUrl,
            "POST",
            body,
            {{"Content-Type", "application/json"}});

    EXPECT_EQ(static_cast<int>(QHttpServerResponse::StatusCode::Forbidden), controlResponse.status);

    const QUrl autoDjUrl(QStringLiteral("http://127.0.0.1:%1/autodj").arg(port));
    const auto autoDjResponse = sendRequest(
            &manager,
            autoDjUrl,
            "POST",
            QJsonDocument(QJsonObject{{"enabled", true}}).toJson(QJsonDocument::Compact),
            {{"Content-Type", "application/json"}});
    EXPECT_EQ(static_cast<int>(QHttpServerResponse::StatusCode::Forbidden), autoDjResponse.status);

    const QUrl playlistsUrl(QStringLiteral("http://127.0.0.1:%1/playlists").arg(port));
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
    const QUrl controlUrl(QStringLiteral("http://127.0.0.1:%1/control").arg(port));
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
    const QUrl controlUrl(QStringLiteral("http://127.0.0.1:%1/control").arg(port));
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

} // namespace
} // namespace mixxx::network::rest

#endif // MIXXX_HAS_HTTP_SERVER
