#pragma once

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QString>
#include <QList>
#include <optional>
#include <QDateTime>
#include <QStringList>

#include "preferences/usersettings.h"

struct RestServerToken {
    QString value;
    QString description;
    QStringList scopes;
    QDateTime createdUtc;
    std::optional<QDateTime> lastUsedUtc;
    std::optional<QDateTime> expiresUtc;

    friend bool operator==(const RestServerToken& lhs, const RestServerToken& rhs) {
        return lhs.value == rhs.value &&
                lhs.description == rhs.description &&
                lhs.scopes == rhs.scopes &&
                lhs.createdUtc == rhs.createdUtc &&
                lhs.lastUsedUtc == rhs.lastUsedUtc &&
                lhs.expiresUtc == rhs.expiresUtc;
    }
};

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
        QList<RestServerToken> tokens;
        bool requireTls{false};
        int maxRequestBytes{0};
        QString corsAllowlist;
        bool streamEnabled{false};
        int streamIntervalMs{0};
        int streamMaxClients{0};
    };

    struct Status {
        bool running{false};
        bool tlsActive{false};
        bool certificateGenerated{false};
        QString lastError;
        QString tlsError;
    };

    static constexpr int kMaxTokens = 16;

    static constexpr int kDefaultPort = 8989;
    static constexpr int kDefaultHttpsPort = 8990;
    static constexpr int kDefaultMaxRequestBytes = 64 * 1024;
    static constexpr int kDefaultStreamIntervalMs = 1000;
    static constexpr int kDefaultStreamMaxClients = 5;
    static constexpr const char* kDefaultCorsAllowlist = "*";

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
