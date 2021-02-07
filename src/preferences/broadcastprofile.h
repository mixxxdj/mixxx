#pragma once

#include <QSharedPointer>
#include <QObject>
#include <QString>

#include "preferences/usersettings.h"

class BroadcastProfile;
typedef QSharedPointer<BroadcastProfile> BroadcastProfilePtr;
Q_DECLARE_METATYPE(BroadcastProfilePtr)

class BroadcastProfile : public QObject {
  Q_OBJECT

  public:
    enum StatusStates {
          STATUS_UNCONNECTED = 0, // IDLE state, no error
          STATUS_CONNECTING = 1, // 30 s max
          STATUS_CONNECTED = 2, // On Air
          STATUS_FAILURE = 3 // Happens when disconnected by an error
    };

    explicit BroadcastProfile(const QString& profileName,
                              QObject* parent = nullptr);
    bool save(const QString& filename);
    bool equals(BroadcastProfilePtr other);
    bool valuesEquals(BroadcastProfilePtr other);
    BroadcastProfilePtr valuesCopy();
    void copyValuesTo(BroadcastProfilePtr other);

    static BroadcastProfilePtr loadFromFile(const QString& filename);
    static bool validName(const QString& str);
    static QString stripForbiddenChars(const QString& str);

    QString getLastFilename() const;

    void setConnectionStatus(int newState);
    int connectionStatus();

    void setSecureCredentialStorage(bool enabled);
    bool secureCredentialStorage();

    void setProfileName(const QString& profileName);
    QString getProfileName() const;

    bool getEnabled() const;
    void setEnabled(bool value);

    QString getHost() const;
    void setHost(const QString& value);

    int getPort() const;
    void setPort(int value);

    QString getServertype() const;
    void setServertype(const QString& value);

    QString getLogin() const;
    void setLogin(const QString& value);

    QString getPassword() const;
    void setPassword(const QString& value);

    bool getEnableReconnect() const;
    void setEnableReconnect(bool value);

    double getReconnectPeriod() const;
    void setReconnectPeriod(double value);

    bool getLimitReconnects() const;
    void setLimitReconnects(bool value);

    int getMaximumRetries() const;
    void setMaximumRetries(int value);

    bool getNoDelayFirstReconnect() const;
    void setNoDelayFirstReconnect(bool value);

    double getReconnectFirstDelay() const;
    void setReconnectFirstDelay(double value);

    QString getFormat() const;
    void setFormat(const QString& value);

    int getBitrate() const;
    void setBitrate(int value);

    int getChannels() const;
    void setChannels(int value);

    QString getMountpoint() const;
    void setMountPoint(const QString& value);

    QString getStreamName() const;
    void setStreamName(const QString& value);

    QString getStreamDesc() const;
    void setStreamDesc(const QString& value);

    QString getStreamGenre() const;
    void setStreamGenre(const QString& value);

    bool getStreamPublic() const;
    void setStreamPublic(bool value);

    QString getStreamWebsite() const;
    void setStreamWebsite(const QString& value);

    QString getStreamIRC() const;
    void setStreamIRC(const QString& value);

    QString getStreamAIM() const;
    void setStreamAIM(const QString& value);

    QString getStreamICQ() const;
    void setStreamICQ(const QString& value);

    bool getEnableMetadata() const;
    void setEnableMetadata(bool value);

    QString getMetadataCharset() const;
    void setMetadataCharset(const QString& value);

    QString getCustomArtist() const;
    void setCustomArtist(const QString& value);

    QString getCustomTitle() const;
    void setCustomTitle(const QString& value);

    QString getMetadataFormat() const;
    void setMetadataFormat(const QString& value);

    bool getOggDynamicUpdate() const;
    void setOggDynamicUpdate(bool value);

  signals:
    void profileNameChanged(const QString& oldName, const QString& newName);
    void statusChanged(bool newStatus);
    void connectionStatusChanged(int newConnectionStatus);

  public slots:
    void relayStatus(bool newStatus);
    void relayConnectionStatus(int newConnectionStatus);

  private:
    void adoptDefaultValues();
    bool loadValues(const QString& filename);

    bool setSecurePassword(const QString& login, const QString& password);
    QString getSecurePassword(const QString& login);

    void errorDialog(const QString& text, const QString& detailedError);

    bool m_secureCredentials;

    QString m_filename;

    QString m_profileName;
    bool m_enabled;

    QString m_host;
    int m_port;
    QString m_serverType;
    QString m_login;
    QString m_password;

    bool m_enableReconnect;
    double m_reconnectPeriod;
    bool m_limitReconnects;
    int m_maximumRetries;
    bool m_noDelayFirstReconnect;
    double m_reconnectFirstDelay;

    QString m_mountpoint;
    QString m_streamName;
    QString m_streamDesc;
    QString m_streamGenre;
    bool m_streamPublic;
    QString m_streamWebsite;
    QString m_streamIRC;
    QString m_streamAIM;
    QString m_streamICQ;

    QString m_format;
    int m_bitrate;
    int m_channels;

    bool m_enableMetadata;
    QString m_metadataCharset;
    QString m_customArtist;
    QString m_customTitle;
    QString m_metadataFormat;
    bool m_oggDynamicUpdate;

    QAtomicInt m_connectionStatus;
};
