#include "preferences/restserversettings.h"

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QHostAddress>
#include <QtGlobal>
#include <limits>

namespace {
const QString kConfigGroup = QStringLiteral("[Rest]");
const QString kConfigEnabled = QStringLiteral("enabled");
const QString kConfigHttpEnabled = QStringLiteral("http_enabled");
const QString kConfigHost = QStringLiteral("host");
const QString kConfigPort = QStringLiteral("port");
const QString kConfigHttpsPort = QStringLiteral("tls_port");
const QString kConfigUseHttps = QStringLiteral("tls_enabled");
const QString kConfigAutoGenerate = QStringLiteral("auto_generate_certificate");
const QString kConfigCertificatePath = QStringLiteral("tls_certificate_path");
const QString kConfigPrivateKeyPath = QStringLiteral("tls_private_key_path");
const QString kConfigAuthToken = QStringLiteral("token");
const QString kConfigRequireTls = QStringLiteral("require_tls");
const QString kConfigStatusRunning = QStringLiteral("status_running");
const QString kConfigStatusTlsActive = QStringLiteral("status_tls_active");
const QString kConfigStatusCertificateGenerated = QStringLiteral("status_certificate_generated");
const QString kConfigStatusLastError = QStringLiteral("status_last_error");
const QString kConfigStatusTlsError = QStringLiteral("status_tls_error");

RestServerSettings::Values applyDefaults(const RestServerSettings::Values& values) {
    RestServerSettings::Values sanitized = values;
    auto ensurePort = [](int port, int fallback) {
        if (port <= 0 || port > std::numeric_limits<quint16>::max()) {
            return fallback;
        }
        return port;
    };
    sanitized.httpPort = ensurePort(sanitized.httpPort, RestServerSettings::kDefaultPort);
    sanitized.httpsPort = ensurePort(sanitized.httpsPort, RestServerSettings::kDefaultHttpsPort);
    if (sanitized.enableHttp && sanitized.useHttps && sanitized.httpPort == sanitized.httpsPort) {
        sanitized.httpsPort = ensurePort(sanitized.httpPort + 1, RestServerSettings::kDefaultHttpsPort);
    }
    return sanitized;
}
} // namespace

RestServerSettings::RestServerSettings(UserSettingsPointer pConfig)
        : m_pConfig(std::move(pConfig)) {
}

RestServerSettings::Values RestServerSettings::get() const {
    Values values;
    values.enabled = m_pConfig->getValue<bool>(ConfigKey(kConfigGroup, kConfigEnabled), false);
    values.enableHttp = m_pConfig->getValue<bool>(ConfigKey(kConfigGroup, kConfigHttpEnabled), true);
    values.host = m_pConfig->getValue<QString>(
            ConfigKey(kConfigGroup, kConfigHost),
            QHostAddress(QHostAddress::LocalHost).toString());
    values.httpPort = m_pConfig->getValue<int>(ConfigKey(kConfigGroup, kConfigPort), kDefaultPort);
    values.httpsPort = m_pConfig->getValue<int>(ConfigKey(kConfigGroup, kConfigHttpsPort), kDefaultHttpsPort);
    values.useHttps = m_pConfig->getValue<bool>(ConfigKey(kConfigGroup, kConfigUseHttps), false);
    values.autoGenerateCert = m_pConfig->getValue<bool>(ConfigKey(kConfigGroup, kConfigAutoGenerate), false);
    values.certificatePath = m_pConfig->getValue<QString>(ConfigKey(kConfigGroup, kConfigCertificatePath), QString());
    values.privateKeyPath = m_pConfig->getValue<QString>(ConfigKey(kConfigGroup, kConfigPrivateKeyPath), QString());
    values.authToken = m_pConfig->getValue<QString>(ConfigKey(kConfigGroup, kConfigAuthToken), QString());
    values.requireTls = m_pConfig->getValue<bool>(ConfigKey(kConfigGroup, kConfigRequireTls), false);
    return applyDefaults(values);
}

void RestServerSettings::set(const Values& values) {
    const Values sanitized = applyDefaults(values);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigEnabled), sanitized.enabled);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigHttpEnabled), sanitized.enableHttp);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigHost), sanitized.host);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigPort), sanitized.httpPort);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigHttpsPort), sanitized.httpsPort);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigUseHttps), sanitized.useHttps);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigAutoGenerate), sanitized.autoGenerateCert);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigCertificatePath), sanitized.certificatePath);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigPrivateKeyPath), sanitized.privateKeyPath);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigAuthToken), sanitized.authToken);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigRequireTls), sanitized.requireTls);
}

RestServerSettings::Values RestServerSettings::defaults() const {
    Values values;
    values.enabled = false;
    values.enableHttp = true;
    values.host = QHostAddress(QHostAddress::LocalHost).toString();
    values.httpPort = kDefaultPort;
    values.httpsPort = kDefaultHttpsPort;
    values.useHttps = false;
    values.autoGenerateCert = false;
    values.certificatePath = QString();
    values.privateKeyPath = QString();
    values.authToken = QString();
    values.requireTls = false;
    return values;
}

void RestServerSettings::resetToDefaults() {
    set(defaults());
}

RestServerSettings::Status RestServerSettings::getStatus() const {
    Status status;
    status.running = m_pConfig->getValue<bool>(ConfigKey(kConfigGroup, kConfigStatusRunning), false);
    status.tlsActive = m_pConfig->getValue<bool>(ConfigKey(kConfigGroup, kConfigStatusTlsActive), false);
    status.certificateGenerated = m_pConfig->getValue<bool>(
            ConfigKey(kConfigGroup, kConfigStatusCertificateGenerated),
            false);
    status.lastError = m_pConfig->getValue<QString>(ConfigKey(kConfigGroup, kConfigStatusLastError), QString());
    status.tlsError = m_pConfig->getValue<QString>(ConfigKey(kConfigGroup, kConfigStatusTlsError), QString());
    return status;
}

void RestServerSettings::setStatus(const Status& status) {
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigStatusRunning), status.running);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigStatusTlsActive), status.tlsActive);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigStatusCertificateGenerated), status.certificateGenerated);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigStatusLastError), status.lastError);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigStatusTlsError), status.tlsError);
}

#endif // MIXXX_HAS_HTTP_SERVER
