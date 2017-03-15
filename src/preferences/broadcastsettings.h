#ifndef PREFERENCES_BROADCASTSETTINGS_H
#define PREFERENCES_BROADCASTSETTINGS_H

#include "preferences/usersettings.h"
#include "track/track.h"

class BroadcastSettings {
  public:
    BroadcastSettings(UserSettingsPointer pConfig);

    int getBitrate() const;
    void setBitrate(int value);
    int getDefaultBitrate() const;
    int getChannels() const;
    void setChannels(int value);
    int getDefaultChannels() const;
    QString getCustomArtist() const;
    void setCustomArtist(const QString& value);
    QString getDefaultCustomArtist() const;
    QString getCustomTitle() const;
    void setCustomTitle(const QString& value);
    QString getDefaultCustomTitle() const;
    bool getEnableMetadata() const;
    void setEnableMetadata(bool value);
    bool getDefaultEnableMetadata() const;
    bool getEnableReconnect() const;
    void setEnableReconnect(bool value);
    bool getDefaultEnableReconnect() const;
    bool getEnabled() const;
    void setEnabled(bool value);
    QString getFormat() const;
    void setFormat(const QString& value);
    QString getDefaultFormat() const;
    QString getHost() const;
    void setHost(const QString& value);
    QString getDefaultHost() const;
    bool getLimitReconnects() const;
    void setLimitReconnects(bool value);
    bool getDefaultLimitReconnects() const;
    QString getLogin() const;
    void setLogin(const QString& value);
    QString getDefaultLogin() const;
    int getMaximumRetries() const;
    void setMaximumRetries(int value);
    int getDefaultMaximumRetries() const;
    QString getMetadataCharset() const;
    void setMetadataCharset(const QString& value);
    QString getDefaultMetadataCharset() const;
    QString getMetadataFormat() const;
    void setMetadataFormat(const QString& value);
    QString getDefaultMetadataFormat() const;
    QString getMountpoint() const;
    void setMountPoint(const QString& value);
    QString getDefaultMountpoint() const;
    bool getNoDelayFirstReconnect() const;
    void setNoDelayFirstReconnect(bool value);
    bool getDefaultNoDelayFirstReconnect() const;
    bool getOggDynamicUpdate() const;
    void setOggDynamicUpdate(bool value);
    bool getDefaultOggDynamicUpdate() const;
    QString getPassword() const;
    void setPassword(const QString& value);
    QString getDefaultPassword() const;
    int getPort() const;
    void setPort(int value);
    int getDefaultPort() const;
    double getReconnectFirstDelay() const;
    void setReconnectFirstDelay(double value);
    double getDefaultReconnectFirstDelay() const;
    double getReconnectPeriod() const;
    void setReconnectPeriod(double value);
    double getDefaultReconnectPeriod() const;
    QString getServertype() const;
    void setServertype(const QString& value);
    QString getDefaultServertype() const;
    QString getStreamDesc() const;
    void setStreamDesc(const QString& value);
    QString getDefaultStreamDesc() const;
    QString getStreamGenre() const;
    void setStreamGenre(const QString& value);
    QString getDefaultStreamGenre() const;
    QString getStreamName() const;
    void setStreamName(const QString& value);
    QString getDefaultStreamName() const;
    bool getStreamPublic() const;
    void setStreamPublic(bool value);
    bool getDefaultStreamPublic() const;
    QString getStreamWebsite() const;
    void setStreamWebsite(const QString& value);
    QString getDefaultStreamWebsite() const;

  private:
    // Pointer to config object
    UserSettingsPointer m_pConfig;
};

#endif /* PREFERENCES_BROADCASTSETTINGS_H */
