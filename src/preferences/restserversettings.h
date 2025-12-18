#pragma once

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QString>

#include "preferences/usersettings.h"

class RestServerSettings {
  public:
    struct Values {
        bool enabled{false};
        bool enableHttp{true};
        QString host;
        int httpPort{0};
        int httpsPort{0};
        bool useHttps{false};
        bool autoGenerateCert{false};
        QString certificatePath;
        QString privateKeyPath;
        QString authToken;
        bool requireTls{false};
    };

    struct Status {
        bool running{false};
        bool tlsActive{false};
        bool certificateGenerated{false};
        QString lastError;
        QString tlsError;
    };

    static constexpr int kDefaultPort = 8989;
    static constexpr int kDefaultHttpsPort = 8990;

    explicit RestServerSettings(UserSettingsPointer pConfig);

    Values get() const;
    void set(const Values& values);
    Values defaults() const;
    void resetToDefaults();

    Status getStatus() const;
    void setStatus(const Status& status);

  private:
    UserSettingsPointer m_pConfig;
};

#endif // MIXXX_HAS_HTTP_SERVER
