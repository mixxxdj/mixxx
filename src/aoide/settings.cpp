#include "aoide/settings.h"

#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("aoide Settings");

const QString kGroup = QStringLiteral("[aoide]");

const ConfigKey kCommandKey(kGroup, "command");

const QString kCommandDefaultValue = QString();

const ConfigKey kDatabaseKey(kGroup, "database");

const QString kDatabaseDefaultValue = QString();

const ConfigKey kProtocolKey(kGroup, "protocol");

const QString kProtocolDefaultValue = QStringLiteral("http");

const ConfigKey kHostKey(kGroup, "host");

const QString kHostDefaultValue = QStringLiteral("::1"); // IPv6 loopback

const ConfigKey kPortKey(kGroup, "port");

const int kPortDefaultValue = 0;

const ConfigKey kCollectionUidKey(kGroup, "collectionUid");

const QString kCollectionUidDefaultValue = QString();

const ConfigKey kPreparedQueriesFilePathKey(kGroup, "preparedQueriesFilePath");

} // anonymous namespace

namespace aoide {

Settings::Settings(UserSettingsPointer userSettings)
        : m_userSettings(std::move(userSettings)) {
    DEBUG_ASSERT(m_userSettings);
}

QString Settings::collectionUid() const {
    return m_userSettings->getValue(kCollectionUidKey, kCollectionUidDefaultValue);
}

void Settings::setCollectionUid(QString collectionUid) const {
    if (collectionUid != kCollectionUidDefaultValue) {
        m_userSettings->setValue(kCollectionUidKey, collectionUid);
    } else {
        m_userSettings->remove(kProtocolKey);
    }
}

QString Settings::command() const {
    return m_userSettings->getValue(kCommandKey, kCommandDefaultValue);
}

void Settings::setCommand(QString command) const {
    if (command != kCommandDefaultValue) {
        m_userSettings->setValue(kCommandKey, command);
    } else {
        m_userSettings->remove(kCommandKey);
    }
}

QString Settings::database() const {
    return m_userSettings->getValue(kDatabaseKey, kDatabaseDefaultValue);
}

void Settings::setDatabase(QString database) const {
    if (database != kDatabaseDefaultValue) {
        m_userSettings->setValue(kDatabaseKey, database);
    } else {
        m_userSettings->remove(kDatabaseKey);
    }
}

QString Settings::protocol() const {
    return m_userSettings->getValue(kProtocolKey, kProtocolDefaultValue);
}

void Settings::setProtocol(QString protocol) const {
    if (protocol != kProtocolDefaultValue) {
        m_userSettings->setValue(kProtocolKey, protocol);
    } else {
        m_userSettings->remove(kProtocolKey);
    }
}

QString Settings::host() const {
    return m_userSettings->getValue(kHostKey, kHostDefaultValue);
}

void Settings::setHost(QString host) const {
    if (host != kHostDefaultValue) {
        m_userSettings->setValue(kHostKey, host);
    } else {
        m_userSettings->remove(kHostKey);
    }
}

int Settings::port() const {
    return m_userSettings->getValue(kPortKey, kPortDefaultValue);
}

void Settings::setPort(int port) const {
    if (port != kPortDefaultValue) {
        m_userSettings->setValue(kPortKey, port);
    } else {
        m_userSettings->remove(kPortKey);
    }
}

QString Settings::endpointAddress() const {
    return QString("%1:%2").arg(host(), QString::number(port()));
}

QUrl Settings::baseUrl(QString endpointAddress) const {
    if (endpointAddress.isEmpty()) {
        QUrl url;
        url.setScheme(protocol());
        url.setHost(host());
        url.setPort(port());
        return url;
    } else {
        return QUrl(protocol() + "://" + endpointAddress);
    }
}

QString Settings::preparedQueriesFilePath() const {
    return m_userSettings->getValue(kPreparedQueriesFilePathKey);
}

void Settings::setPreparedQueriesFilePath(QString preparedQueriesFilePath) const {
    if (preparedQueriesFilePath.isEmpty()) {
        m_userSettings->remove(kPreparedQueriesFilePathKey);
    } else {
        m_userSettings->setValue(kPreparedQueriesFilePathKey, preparedQueriesFilePath);
    }
}

} // namespace aoide
