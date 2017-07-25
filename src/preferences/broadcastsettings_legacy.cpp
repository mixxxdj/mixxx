#include "preferences/broadcastsettings.h"

namespace {
const char* kConfigKey = "[Shoutcast]";
const char* kBitrate = "bitrate";
const char* kChannels = "channels";
const char* kCustomArtist = "custom_artist";
const char* kCustomTitle = "custom_title";
const char* kEnableMetadata = "enable_metadata";
const char* kEnableReconnect = "enable_reconnect";
const char* kEnabled = "enabled";
const char* kFormat = "format";
const char* kHost = "host";
const char* kLimitReconnects = "limit_reconnects";
const char* kLogin = "login";
const char* kMaximumRetries = "maximum_retries";
const char* kMetadataCharset = "metadata_charset";
const char* kMetadataFormat = "metadata_format";
const char* kMountPoint = "mountpoint";
const char* kNoDelayFirstReconnect = "no_delay_first_reconnect";
const char* kOggDynamicUpdate = "ogg_dynamicupdate";
const char* kPassword = "password";
const char* kPort = "port";
const char* kReconnectFirstDelay = "reconnect_first_delay";
const char* kReconnectPeriod = "reconnect_period";
const char* kServertype = "servertype";
const char* kStreamDesc = "stream_desc";
const char* kStreamGenre = "stream_genre";
const char* kStreamName = "stream_name";
const char* kStreamPublic = "stream_public";
const char* kStreamWebsite = "stream_website";
}

void BroadcastSettings::loadLegacySettings(const BroadcastProfilePtr& profile) {
    if(!profile)
        return;

    // For each value, the current value is kept if it can't be found in the
    // general settings file.
    profile->setEnabled(m_pConfig->getValue(
                            ConfigKey(kConfigKey, kEnabled),
                            profile->getEnabled()));

    profile->setHost(m_pConfig->getValue(
                         ConfigKey(kConfigKey, kHost),
                         profile->getHost()));

    profile->setPort(m_pConfig->getValue(
                         ConfigKey(kConfigKey, kPort),
                         profile->getPort()));

    profile->setServertype(m_pConfig->getValue(
                               ConfigKey(kConfigKey, kServertype),
                               profile->getServertype()));

    profile->setLogin(m_pConfig->getValue(
                          ConfigKey(kConfigKey, kLogin),
                          profile->getLogin()));

    profile->setPassword(m_pConfig->getValue(
                             ConfigKey(kConfigKey, kPassword),
                             profile->getPassword()));

    profile->setEnableReconnect(m_pConfig->getValue(
                                    ConfigKey(kConfigKey, kEnableReconnect),
                                    profile->getEnableReconnect()));

    profile->setReconnectPeriod(m_pConfig->getValue(
                                    ConfigKey(kConfigKey, kReconnectPeriod),
                                    profile->getReconnectPeriod()));

    profile->setLimitReconnects(m_pConfig->getValue(
                                    ConfigKey(kConfigKey, kLimitReconnects),
                                    profile->getLimitReconnects()));

    profile->setMountPoint(m_pConfig->getValue(
                               ConfigKey(kConfigKey, kMountPoint),
                               profile->getMountpoint()));

    profile->setStreamDesc(m_pConfig->getValue(
                               ConfigKey(kConfigKey, kStreamDesc),
                               profile->getStreamDesc()));

    profile->setStreamGenre(m_pConfig->getValue(
                                ConfigKey(kConfigKey, kStreamGenre),
                                profile->getStreamGenre()));

    profile->setStreamName(m_pConfig->getValue(
                               ConfigKey(kConfigKey, kStreamName),
                               profile->getStreamName()));

    profile->setStreamPublic(m_pConfig->getValue(
                                 ConfigKey(kConfigKey, kStreamPublic),
                                 profile->getStreamPublic()));

    profile->setStreamWebsite(m_pConfig->getValue(
                                  ConfigKey(kConfigKey, kStreamWebsite),
                                  profile->getStreamWebsite()));

    profile->setEnableMetadata(m_pConfig->getValue(
                                   ConfigKey(kConfigKey, kEnableMetadata),
                                   profile->getEnableMetadata()));

    profile->setMetadataCharset(m_pConfig->getValue(
                                    ConfigKey(kConfigKey, kMetadataCharset),
                                    profile->getMetadataCharset()));

    profile->setCustomArtist(m_pConfig->getValue(
                                 ConfigKey(kConfigKey, kCustomArtist),
                                 profile->getCustomArtist()));

    profile->setCustomTitle(m_pConfig->getValue(
                                ConfigKey(kConfigKey, kCustomTitle),
                                profile->getCustomTitle()));

    profile->setMetadataFormat(m_pConfig->getValue(
                                   ConfigKey(kConfigKey, kMetadataFormat),
                                   profile->getMetadataFormat()));

    profile->setOggDynamicUpdate(m_pConfig->getValue(
                                     ConfigKey(kConfigKey, kOggDynamicUpdate),
                                     profile->getOggDynamicUpdate()));

    profile->setBitrate(m_pConfig->getValue(
                            ConfigKey(kConfigKey, kBitrate),
                            profile->getBitrate()));

    profile->setChannels(m_pConfig->getValue(
                             ConfigKey(kConfigKey, kChannels),
                             profile->getChannels()));

    profile->setFormat(m_pConfig->getValue(
                           ConfigKey(kConfigKey, kFormat),
                           profile->getFormat()));

    profile->setNoDelayFirstReconnect(
                m_pConfig->getValue(
                    ConfigKey(kConfigKey, kNoDelayFirstReconnect),
                    profile->getNoDelayFirstReconnect()));

    profile->setReconnectFirstDelay(
                m_pConfig->getValue(
                    ConfigKey(kConfigKey, kReconnectFirstDelay),
                    profile->getReconnectFirstDelay()));

    profile->setMaximumRetries(m_pConfig->getValue(
                                   ConfigKey(kConfigKey, kMaximumRetries),
                                   profile->getMaximumRetries()));
}
