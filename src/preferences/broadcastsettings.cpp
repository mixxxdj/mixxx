#include "preferences/broadcastsettings.h"
#include "broadcast/defs_broadcast.h"
#include "defs_urls.h"

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

const double kDefaultBitrate = 128;
const int kDefaultChannels = 2;
const bool kDefaultEnableMetadata = false;
const bool kDefaultEnableReconnect = true;
const bool kDefaultLimitReconnects = true;
const int kDefaultMaximumRetries = 10;
// No tr() here, see https://bugs.launchpad.net/mixxx/+bug/1419500
const QString kDefaultMetadataFormat("$artist - $title");
const bool kDefaultNoDelayFirstReconnect = true;
const bool kDefaultOggDynamicupdate = false;
double kDefaultReconnectFirstDelay = 0.0;
double kDefaultReconnectPeriod = 5.0;
const QString kDefaultStreamDesc = QObject::tr("This stream is online for testing purposes!");
const QString kDefaultStreamGenre = QObject::tr("Live Mix");
const bool kDefaultStreamPublic = false;
} // anonymous namespace

BroadcastSettings::BroadcastSettings(UserSettingsPointer pConfig)
    : m_pConfig(pConfig) {
}

int BroadcastSettings::getBitrate() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kBitrate), getDefaultBitrate());
}

void BroadcastSettings::setBitrate(int value) {
    m_pConfig->setValue(ConfigKey(kConfigKey, kBitrate), value);
}

int BroadcastSettings::getDefaultBitrate() const {
    return kDefaultBitrate;
}

int BroadcastSettings::getChannels() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kChannels), getDefaultChannels());
}

void BroadcastSettings::setChannels(int value) {
    m_pConfig->setValue(ConfigKey(kConfigKey, kChannels), value);
}

int BroadcastSettings::getDefaultChannels() const {
    return kDefaultChannels;
}

QString BroadcastSettings::getCustomArtist() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kCustomArtist), getDefaultCustomArtist());

}

void BroadcastSettings::setCustomArtist(const QString& value) {
    m_pConfig->setValue(ConfigKey(kConfigKey, kCustomArtist), value);
}

QString BroadcastSettings::getDefaultCustomArtist() const {
    return QString();
}

QString BroadcastSettings::getCustomTitle() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kCustomTitle), getDefaultCustomTitle());
}

void BroadcastSettings::setCustomTitle(const QString& value) {
    m_pConfig->setValue(ConfigKey(kConfigKey, kCustomTitle), value);
}

QString BroadcastSettings::getDefaultCustomTitle() const {
    return QString();
}

bool BroadcastSettings::getEnableMetadata() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kEnableMetadata), getDefaultEnableMetadata());
}

void BroadcastSettings::setEnableMetadata(bool value) {
    m_pConfig->setValue(ConfigKey(kConfigKey, kEnableMetadata), value);
}

bool BroadcastSettings::getDefaultEnableMetadata() const {
    return kDefaultEnableMetadata;
}

bool BroadcastSettings::getEnableReconnect() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kEnableReconnect), getDefaultEnableReconnect());
}

void BroadcastSettings::setEnableReconnect(bool value) {
    m_pConfig->setValue(ConfigKey(kConfigKey, kEnableReconnect), value);
}

bool BroadcastSettings::getDefaultEnableReconnect() const {
    return kDefaultEnableReconnect;
}

// Unused, but we keep this to reserve the name
bool BroadcastSettings::getEnabled() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kEnabled), true);
}

void BroadcastSettings::setEnabled(bool value) {
    m_pConfig->setValue(ConfigKey(kConfigKey, kEnabled), value);
}

QString BroadcastSettings::getFormat() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kFormat), getDefaultFormat());
}

void BroadcastSettings::setFormat(const QString& value) {
    m_pConfig->setValue(ConfigKey(kConfigKey, kFormat), value);
}

QString BroadcastSettings::getDefaultFormat() const {
    return QString();
}

QString BroadcastSettings::getHost() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kHost), getDefaultHost());
}

void BroadcastSettings::setHost(const QString& value) {
    m_pConfig->setValue(ConfigKey(kConfigKey, kHost), value);
}

QString BroadcastSettings::getDefaultHost() const {
    return QString();
}

bool BroadcastSettings::getLimitReconnects() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kLimitReconnects), getDefaultLimitReconnects());
}

void BroadcastSettings::setLimitReconnects(bool value) {
    m_pConfig->setValue(ConfigKey(kConfigKey, kLimitReconnects), value);
}

bool BroadcastSettings::getDefaultLimitReconnects() const {
    return kDefaultLimitReconnects;
}

QString BroadcastSettings::getLogin() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kLogin), getDefaultLogin());
}

void BroadcastSettings::setLogin(const QString& value) {
    m_pConfig->setValue(ConfigKey(kConfigKey, kLogin), value);
}

QString BroadcastSettings::getDefaultLogin() const {
    return QString();
}

int BroadcastSettings::getMaximumRetries() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kMaximumRetries), getDefaultMaximumRetries());
}

void BroadcastSettings::setMaximumRetries(int value) {
    m_pConfig->setValue(ConfigKey(kConfigKey, kMaximumRetries), value);
}

int BroadcastSettings::getDefaultMaximumRetries() const {
    return kDefaultMaximumRetries;
}

QString BroadcastSettings::getMetadataCharset() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kMetadataCharset));
}

void BroadcastSettings::setMetadataCharset(const QString& value) {
    m_pConfig->setValue(ConfigKey(kConfigKey, kMetadataCharset), value);
}

QString BroadcastSettings::getDefaultMetadataCharset() const {
    return QString();
}

QString BroadcastSettings::getMetadataFormat() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kMetadataFormat), getDefaultMetadataFormat());
}

void BroadcastSettings::setMetadataFormat(const QString& value) {
    m_pConfig->setValue(ConfigKey(kConfigKey, kMetadataFormat), value);
}

QString BroadcastSettings::getDefaultMetadataFormat() const {
    return kDefaultMetadataFormat;
}

QString BroadcastSettings::getMountpoint() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kMountPoint));
}

void BroadcastSettings::setMountPoint(const QString& value) {
    m_pConfig->setValue(ConfigKey(kConfigKey, kMountPoint), value);
}

QString BroadcastSettings::getDefaultMountpoint() const {
    return QString();
}

bool BroadcastSettings::getNoDelayFirstReconnect() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kNoDelayFirstReconnect),
            getDefaultNoDelayFirstReconnect());
}

void BroadcastSettings::setNoDelayFirstReconnect(bool value) {
    m_pConfig->setValue(ConfigKey(kConfigKey, kNoDelayFirstReconnect), value);
}

bool BroadcastSettings::getDefaultNoDelayFirstReconnect() const {
    return kDefaultNoDelayFirstReconnect;
}

bool BroadcastSettings::getOggDynamicUpdate() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kOggDynamicUpdate),
            getDefaultOggDynamicUpdate());
}

void BroadcastSettings::setOggDynamicUpdate(bool value) {
    m_pConfig->setValue(ConfigKey(kConfigKey, kOggDynamicUpdate), value);
}

bool BroadcastSettings::getDefaultOggDynamicUpdate() const {
    return kDefaultOggDynamicupdate;
}

QString BroadcastSettings::getPassword() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kPassword), getDefaultPassword());
}

void BroadcastSettings::setPassword(const QString& value) {
    m_pConfig->setValue(ConfigKey(kConfigKey, kPassword), value);
}

QString BroadcastSettings::getDefaultPassword() const {
    return QString();
}

int BroadcastSettings::getPort() const {
    // Valid port numbers are 0 .. 65535 (16 bit unsigned)
    int port =  m_pConfig->getValue(
            ConfigKey(kConfigKey, kPort), getDefaultPort());
    if (port < 0 || port > 0xFFFF) {
        return getDefaultPort();
    }
    return port;
}

void BroadcastSettings::setPort(int value) {
    m_pConfig->setValue(ConfigKey(kConfigKey, kPort), value);
}

int BroadcastSettings::getDefaultPort() const {
    return BROADCAST_DEFAULT_PORT;
}

double BroadcastSettings::getReconnectFirstDelay() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kReconnectFirstDelay),
            getDefaultReconnectFirstDelay());
}

void BroadcastSettings::setReconnectFirstDelay(double value) {
    m_pConfig->setValue(ConfigKey(kConfigKey, kReconnectFirstDelay), value);
}

double BroadcastSettings::getDefaultReconnectFirstDelay() const {
    return kDefaultReconnectFirstDelay;
}

double BroadcastSettings::getReconnectPeriod() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kReconnectPeriod),
            getDefaultReconnectPeriod());
}

void BroadcastSettings::setReconnectPeriod(double value) {
    m_pConfig->setValue(ConfigKey(kConfigKey, kReconnectPeriod), value);
}

double BroadcastSettings::getDefaultReconnectPeriod() const {
    return kDefaultReconnectPeriod;
}

QString BroadcastSettings::getServertype() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kServertype), getDefaultServertype());
}

void BroadcastSettings::setServertype(const QString& value) {
    m_pConfig->set(ConfigKey(kConfigKey, kServertype),
            ConfigValue(value));
}

QString BroadcastSettings::getDefaultServertype() const {
    return QString();
}

QString BroadcastSettings::getStreamDesc() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kStreamDesc),
            getDefaultStreamDesc());
}

void BroadcastSettings::setStreamDesc(const QString& value) {
    m_pConfig->setValue(ConfigKey(kConfigKey, kStreamDesc), value);
}

QString BroadcastSettings::getDefaultStreamDesc() const {
    return kDefaultStreamDesc;
}

QString BroadcastSettings::getStreamGenre() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kStreamGenre),
            getDefaultStreamGenre());
}

void BroadcastSettings::setStreamGenre(const QString& value) {
    m_pConfig->setValue(ConfigKey(kConfigKey, kStreamGenre), value);
}

QString BroadcastSettings::getDefaultStreamGenre() const {
    return kDefaultStreamGenre;
}

QString BroadcastSettings::getStreamName() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kStreamName),
            getDefaultStreamName());
}

void BroadcastSettings::setStreamName(const QString& value) {
    m_pConfig->setValue(ConfigKey(kConfigKey, kStreamName), value);
}

QString BroadcastSettings::getDefaultStreamName() const {
    return QString();
}

bool BroadcastSettings::getStreamPublic() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kStreamPublic), getDefaultStreamPublic());
}

void BroadcastSettings::setStreamPublic(bool value) {
    m_pConfig->setValue(ConfigKey(kConfigKey, kStreamPublic), value);
}

bool BroadcastSettings::getDefaultStreamPublic() const {
    return kDefaultStreamPublic;
}

QString BroadcastSettings::getStreamWebsite() const {
    return m_pConfig->getValue(
            ConfigKey(kConfigKey, kStreamWebsite), getDefaultStreamWebsite());
}

void BroadcastSettings::setStreamWebsite(const QString& value) {
    m_pConfig->setValue(ConfigKey(kConfigKey, kStreamWebsite), value);
}

QString BroadcastSettings::getDefaultStreamWebsite() const {
    return MIXXX_WEBSITE_URL;
}
