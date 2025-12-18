#include "preferences/webserversettings.h"

#include <QVariant>

#include <utility>

namespace {
const QString kConfigGroup = QStringLiteral("[WebServer]");
const QString kConfigEnabled = QStringLiteral("Enabled");
const QString kConfigBindAddress = QStringLiteral("BindAddress");
const QString kConfigPort = QStringLiteral("Port");
const QString kConfigAuthenticationEnabled = QStringLiteral("AuthenticationEnabled");
const QString kConfigUsername = QStringLiteral("Username");
const QString kConfigPassword = QStringLiteral("Password");
const QString kConfigUseHttps = QStringLiteral("UseHttps");
const QString kConfigCertPath = QStringLiteral("CertPath");
const QString kConfigKeyPath = QStringLiteral("KeyPath");
const QString kConfigCaBundlePath = QStringLiteral("CaBundlePath");
const QString kConfigAutoGenerateCert = QStringLiteral("AutoGenerateCert");

const QString kBindAddressLocalhost = QStringLiteral("localhost");
const QString kBindAddressAny = QStringLiteral("0.0.0.0");

WebServerSettings::Values applyDefaults(const WebServerSettings::Values& values) {
    WebServerSettings::Values sanitized = values;
    if (sanitized.port <= 0 || sanitized.port > 65535) {
        sanitized.port = WebServerSettings::kDefaultPort;
    }
    return sanitized;
}
} // namespace

WebServerSettings::WebServerSettings(UserSettingsPointer pConfig)
        : m_pConfig(std::move(pConfig)) {
}

WebServerSettings::Values WebServerSettings::get() const {
    Values values;
    values.enabled = m_pConfig->getValue<bool>(
            ConfigKey(kConfigGroup, kConfigEnabled), false);
    const QString bindAddress = m_pConfig->getValue<QString>(
            ConfigKey(kConfigGroup, kConfigBindAddress), kBindAddressLocalhost);
    values.bindAddress = bindAddressFromString(bindAddress);
    values.port = m_pConfig->getValue<int>(
            ConfigKey(kConfigGroup, kConfigPort), kDefaultPort);
    values.authenticationEnabled = m_pConfig->getValue<bool>(
            ConfigKey(kConfigGroup, kConfigAuthenticationEnabled), false);
    values.username = m_pConfig->getValue<QString>(
            ConfigKey(kConfigGroup, kConfigUsername), QString());
    values.password = m_pConfig->getValue<QString>(
            ConfigKey(kConfigGroup, kConfigPassword), QString());
    values.useHttps = m_pConfig->getValue<bool>(
            ConfigKey(kConfigGroup, kConfigUseHttps), false);
    values.certPath = m_pConfig->getValue<QString>(
            ConfigKey(kConfigGroup, kConfigCertPath), QString());
    values.keyPath = m_pConfig->getValue<QString>(
            ConfigKey(kConfigGroup, kConfigKeyPath), QString());
    values.caBundlePath = m_pConfig->getValue<QString>(
            ConfigKey(kConfigGroup, kConfigCaBundlePath), QString());
    values.autoGenerateCert = m_pConfig->getValue<bool>(
            ConfigKey(kConfigGroup, kConfigAutoGenerateCert), false);
    return applyDefaults(values);
}

void WebServerSettings::set(const Values& values) {
    const Values sanitized = applyDefaults(values);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigEnabled), sanitized.enabled);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigBindAddress),
            bindAddressToString(sanitized.bindAddress));
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigPort), sanitized.port);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigAuthenticationEnabled),
            sanitized.authenticationEnabled);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigUsername), sanitized.username);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigPassword), sanitized.password);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigUseHttps), sanitized.useHttps);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigCertPath), sanitized.certPath);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigKeyPath), sanitized.keyPath);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigCaBundlePath), sanitized.caBundlePath);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigAutoGenerateCert), sanitized.autoGenerateCert);
}

WebServerSettings::Values WebServerSettings::defaults() const {
    Values values;
    values.enabled = false;
    values.bindAddress = BindAddress::Localhost;
    values.port = kDefaultPort;
    values.authenticationEnabled = false;
    values.username = QString();
    values.password = QString();
    values.useHttps = false;
    values.certPath = QString();
    values.keyPath = QString();
    values.caBundlePath = QString();
    values.autoGenerateCert = false;
    return values;
}

void WebServerSettings::resetToDefaults() {
    set(defaults());
}

QString WebServerSettings::bindAddressToString(BindAddress bindAddress) {
    switch (bindAddress) {
    case BindAddress::Any:
        return kBindAddressAny;
    case BindAddress::Localhost:
    default:
        return kBindAddressLocalhost;
    }
}

WebServerSettings::BindAddress WebServerSettings::bindAddressFromString(const QString& value) {
    if (value == kBindAddressAny) {
        return BindAddress::Any;
    }
    return BindAddress::Localhost;
}
