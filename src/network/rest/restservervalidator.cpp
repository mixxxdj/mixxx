#include "network/rest/restservervalidator.h"

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QTcpServer>
#include <QtGlobal>
#include <limits>
#include <type_traits>
#include <utility>

namespace mixxx::network::rest {

namespace {
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

constexpr bool kHttpServerHasTlsSupport =
        HasHttpServerSslConfiguration<QHttpServer>::value ||
        HasHttpServerSslSetup<QHttpServer>::value;
} // namespace

RestServerValidator::RestServerValidator(
        RestServer::Settings activeSettings,
        bool serverRunning,
        CertificateGenerator* certificateGenerator)
        : m_activeSettings(std::move(activeSettings)),
          m_serverRunning(serverRunning),
          m_certificateGenerator(certificateGenerator) {
}

RestServerValidationResult RestServerValidator::validate(
        const RestServer::Settings& settings) const {
    RestServerValidationResult result;
    result.settings = settings;

    if (!settings.enabled) {
        result.success = true;
        return result;
    }

    if (!settings.hostValid) {
        result.error = QObject::tr("Invalid REST API host address");
        return result;
    }

    if (!settings.portValid) {
        result.error = QObject::tr("REST API port must be between %1 and %2")
                               .arg(1)
                               .arg(std::numeric_limits<quint16>::max());
        return result;
    }

    if (!isPortAvailable(settings)) {
        result.error = QObject::tr("REST API port %1 is unavailable").arg(settings.port);
        return result;
    }

    if (settings.useHttps) {
        if (!kHttpServerHasTlsSupport) {
            const QString baseError = QObject::tr("HTTPS is not supported by this Qt build");
            const QString error = QStringLiteral("%1 (build: %2, runtime: %3)")
                                          .arg(baseError,
                                                  QString::fromLatin1(QT_VERSION_STR),
                                                  QString::fromLatin1(qVersion()));
            result.error = error;
            result.tlsError = QString();
            return result;
        }
        const RestServer::TlsResult tlsResult = RestServer::prepareTlsConfiguration(
                settings, m_certificateGenerator);
        if (!tlsResult.success) {
            result.error = QObject::tr("TLS configuration failed");
            result.tlsError = tlsResult.error;
            return result;
        }

        result.tlsConfiguration = tlsResult;
        result.settings.certificatePath = tlsResult.certificatePath;
        result.settings.privateKeyPath = tlsResult.privateKeyPath;
    }

    result.success = true;
    return result;
}

bool RestServerValidator::isPortAvailable(const RestServer::Settings& settings) const {
    if (m_serverRunning &&
            settings.address == m_activeSettings.address &&
            settings.port == m_activeSettings.port) {
        return true;
    }

    QTcpServer probe;
    if (!probe.listen(settings.address, static_cast<quint16>(settings.port))) {
        return false;
    }
    probe.close();
    return true;
}

} // namespace mixxx::network::rest

#endif // MIXXX_HAS_HTTP_SERVER
