#pragma once

#include <QMetaType>
#include <QString>

#include "preferences/usersettings.h"

class WebServerSettings {
  public:
    enum class BindAddress {
        Localhost,
        Any
    };

    struct Values {
        bool enabled;
        BindAddress bindAddress;
        int port;
        bool authenticationEnabled;
        QString username;
        QString password;
        bool useHttps;
        QString certPath;
        QString keyPath;
        QString caBundlePath;
        bool autoGenerateCert;
    };

    static constexpr int kDefaultPort = 8000;

    explicit WebServerSettings(UserSettingsPointer pConfig);

    Values get() const;
    void set(const Values& values);
    Values defaults() const;
    void resetToDefaults();

    static QString bindAddressToString(BindAddress bindAddress);
    static BindAddress bindAddressFromString(const QString& value);

  private:
    UserSettingsPointer m_pConfig;
};

Q_DECLARE_METATYPE(WebServerSettings::BindAddress)
Q_DECLARE_METATYPE(WebServerSettings::Values)
