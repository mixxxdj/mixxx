#include "preferences/restserversettings.h"

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QHostAddress>
#include <QtGlobal>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <limits>

#include "network/rest/restscopes.h"

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
const QString kConfigTokens = QStringLiteral("tokens");
const QString kConfigRequireTls = QStringLiteral("require_tls");
const QString kConfigMaxRequestBytes = QStringLiteral("max_request_bytes");
const QString kConfigCorsAllowlist = QStringLiteral("cors_allowlist");
const QString kConfigStreamEnabled = QStringLiteral("stream_enabled");
const QString kConfigStreamIntervalMs = QStringLiteral("stream_interval_ms");
const QString kConfigStreamMaxClients = QStringLiteral("stream_max_clients");
const QString kConfigStatusRunning = QStringLiteral("status_running");
const QString kConfigStatusTlsActive = QStringLiteral("status_tls_active");
const QString kConfigStatusCertificateGenerated = QStringLiteral("status_certificate_generated");
const QString kConfigStatusLastError = QStringLiteral("status_last_error");
const QString kConfigStatusTlsError = QStringLiteral("status_tls_error");

QStringList normalizeScopes(const QStringList& scopes) {
    QStringList normalized;
    normalized.reserve(scopes.size());
    for (const auto& scope : scopes) {
        const QString trimmed = scope.trimmed().toLower();
        if (trimmed.isEmpty()) {
            continue;
        }
        if (!normalized.contains(trimmed)) {
            normalized.append(trimmed);
        }
    }
    return normalized;
}

QStringList legacyScopesForPermission(const QString& permission) {
    const QString normalized = permission.trimmed().toLower();
    if (normalized == QStringLiteral("full")) {
        return mixxx::network::rest::scopes::allScopes();
    }
    if (normalized == QStringLiteral("read")) {
        return mixxx::network::rest::scopes::defaultReadScopes();
    }
    return {};
}

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
    if (sanitized.tokens.size() > RestServerSettings::kMaxTokens) {
        sanitized.tokens = sanitized.tokens.mid(0, RestServerSettings::kMaxTokens);
    }
    for (auto& token : sanitized.tokens) {
        token.scopes = normalizeScopes(token.scopes);
    }
    if (sanitized.maxRequestBytes <= 0) {
        sanitized.maxRequestBytes = RestServerSettings::kDefaultMaxRequestBytes;
    }
    if (sanitized.streamIntervalMs <= 0) {
        sanitized.streamIntervalMs = RestServerSettings::kDefaultStreamIntervalMs;
    }
    if (sanitized.streamMaxClients <= 0) {
        sanitized.streamMaxClients = RestServerSettings::kDefaultStreamMaxClients;
    }
    const QStringList allowlistEntries =
            sanitized.corsAllowlist.split(',', Qt::SkipEmptyParts);
    QStringList normalized;
    normalized.reserve(allowlistEntries.size());
    for (const auto& entry : allowlistEntries) {
        const QString trimmed = entry.trimmed();
        if (!trimmed.isEmpty()) {
            normalized.append(trimmed);
        }
    }
    sanitized.corsAllowlist = normalized.join(QStringLiteral(", "));
    if (sanitized.corsAllowlist.isEmpty()) {
        sanitized.corsAllowlist =
                QString::fromLatin1(RestServerSettings::kDefaultCorsAllowlist);
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
    const QString tokensJson = m_pConfig->getValue<QString>(ConfigKey(kConfigGroup, kConfigTokens), QString());
    const QJsonDocument tokensDoc = QJsonDocument::fromJson(tokensJson.toUtf8());
    const QJsonArray tokenArray = tokensDoc.isArray() ? tokensDoc.array() : QJsonArray{};
    for (const auto& value : tokenArray) {
        if (!value.isObject()) {
            continue;
        }
        const QJsonObject obj = value.toObject();
        RestServerToken token;
        token.value = obj.value(QStringLiteral("token")).toString();
        token.description = obj.value(QStringLiteral("description")).toString();
        const QJsonArray scopesArray = obj.value(QStringLiteral("scopes")).toArray();
        QStringList scopes;
        scopes.reserve(scopesArray.size());
        for (const auto& scopeValue : scopesArray) {
            if (scopeValue.isString()) {
                scopes.append(scopeValue.toString());
            }
        }
        if (scopes.isEmpty()) {
            scopes = legacyScopesForPermission(
                    obj.value(QStringLiteral("permission")).toString());
        }
        token.scopes = normalizeScopes(scopes);
        token.createdUtc = QDateTime::fromString(
                obj.value(QStringLiteral("created_utc")).toString(),
                Qt::ISODate);
        if (!token.createdUtc.isValid()) {
            token.createdUtc = QDateTime::currentDateTimeUtc();
        }
        const QString expires = obj.value(QStringLiteral("expires_utc")).toString();
        if (!expires.isEmpty()) {
            token.expiresUtc = QDateTime::fromString(expires, Qt::ISODate);
        }
        if (!token.value.isEmpty()) {
            values.tokens.append(token);
        }
    }
    values.requireTls = m_pConfig->getValue<bool>(ConfigKey(kConfigGroup, kConfigRequireTls), false);
    values.maxRequestBytes = m_pConfig->getValue<int>(
            ConfigKey(kConfigGroup, kConfigMaxRequestBytes),
            kDefaultMaxRequestBytes);
    values.corsAllowlist = m_pConfig->getValue<QString>(
            ConfigKey(kConfigGroup, kConfigCorsAllowlist),
            QString::fromLatin1(kDefaultCorsAllowlist));
    values.streamEnabled = m_pConfig->getValue<bool>(
            ConfigKey(kConfigGroup, kConfigStreamEnabled),
            false);
    values.streamIntervalMs = m_pConfig->getValue<int>(
            ConfigKey(kConfigGroup, kConfigStreamIntervalMs),
            kDefaultStreamIntervalMs);
    values.streamMaxClients = m_pConfig->getValue<int>(
            ConfigKey(kConfigGroup, kConfigStreamMaxClients),
            kDefaultStreamMaxClients);
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
    QJsonArray tokenArray;
    tokenArray.reserve(sanitized.tokens.size());
    int count = 0;
    for (const auto& token : sanitized.tokens) {
        if (token.value.isEmpty()) {
            continue;
        }
        if (count >= kMaxTokens) {
            break;
        }
        QJsonObject object;
        object.insert(QStringLiteral("token"), token.value);
        object.insert(QStringLiteral("description"), token.description);
        QJsonArray scopesArray;
        const QStringList normalizedScopes = normalizeScopes(token.scopes);
        scopesArray.reserve(normalizedScopes.size());
        for (const auto& scope : normalizedScopes) {
            scopesArray.append(scope);
        }
        object.insert(QStringLiteral("scopes"), scopesArray);
        object.insert(QStringLiteral("created_utc"), token.createdUtc.toString(Qt::ISODate));
        if (token.expiresUtc.has_value()) {
            object.insert(QStringLiteral("expires_utc"), token.expiresUtc->toString(Qt::ISODate));
        }
        tokenArray.append(object);
        ++count;
    }
    const QJsonDocument tokensDoc(tokenArray);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigTokens), QString::fromUtf8(tokensDoc.toJson(QJsonDocument::Compact)));
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigRequireTls), sanitized.requireTls);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigMaxRequestBytes), sanitized.maxRequestBytes);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigCorsAllowlist), sanitized.corsAllowlist);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigStreamEnabled), sanitized.streamEnabled);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigStreamIntervalMs),
            sanitized.streamIntervalMs);
    m_pConfig->setValue(ConfigKey(kConfigGroup, kConfigStreamMaxClients),
            sanitized.streamMaxClients);
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
    values.tokens.clear();
    values.requireTls = false;
    values.maxRequestBytes = kDefaultMaxRequestBytes;
    values.corsAllowlist = QString::fromLatin1(kDefaultCorsAllowlist);
    values.streamEnabled = false;
    values.streamIntervalMs = kDefaultStreamIntervalMs;
    values.streamMaxClients = kDefaultStreamMaxClients;
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
