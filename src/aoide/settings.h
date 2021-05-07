#pragma once

#include <QUrl>

#include "preferences/usersettings.h"

namespace aoide {

class Settings {
  public:
    explicit Settings(
            UserSettingsPointer userSettings);

    QString command() const;
    void setCommand(QString command) const;

    QString database() const;
    void setDatabase(QString database) const;

    QString protocol() const;
    void setProtocol(QString protocol) const;

    QString host() const;
    void setHost(QString host) const;

    int port() const;
    void setPort(int port) const;

    // host + port
    QString endpointAddress() const;

    // protocol + host + port
    QUrl baseUrl(QString endpointAddress = QString()) const;

    QString collectionUid() const;
    void setCollectionUid(QString collectionUid) const;

    QString preparedQueriesFilePath() const;
    void setPreparedQueriesFilePath(QString preparedQueriesFilePath) const;

    QString getSettingsPath() const {
        return m_userSettings->getSettingsPath();
    }

  private:
    const UserSettingsPointer m_userSettings;
};

} // namespace aoide
