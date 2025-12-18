#include "preferences/restserversettings.h"

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QHostAddress>

namespace {
const QString kConfigGroup = QStringLiteral("[Rest]");
const QString kConfigEnabled = QStringLiteral("enabled");
const QString kConfigHost = QStringLiteral("host");
const QString kConfigPort = QStringLiteral("port");
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
    if (sanitized.port <= 0 || sanitized.port > 65535) {
        sanitized.port = RestServerSettings::kDefaultPort;
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
    values.host = m_pConfig->getValue<QString>(
            ConfigKey(kConfigGroup, kConfigHost),
            QHostAddress(QHostAddress::LocalHost).toString());
    values.port = m_pConfig->getValue<int>(ConfigKey(kConfigGroup, kConfigPort), kDefaultPort);
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
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigHost), sanitized.host);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigPort), sanitized.port);
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
    values.host = QHostAddress(QHostAddress::LocalHost).toString();
    values.port = kDefaultPort;
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
