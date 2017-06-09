// broadcastprofile.cpp
// Created June 2nd 2017 by Stéphane Lepin <stephane.lepin@gmail.com>

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QRegExp>
#include <QDebug>
#include "util/xml.h"
#include "broadcast/defs_broadcast.h"
#include "defs_urls.h"

#include "broadcastprofile.h"

namespace {
const char* kDocumentName = "BroadcastProfile";
const char* kBitrate = "Bitrate";
const char* kChannels = "Channels";
const char* kCustomArtist = "CustomArtist";
const char* kCustomTitle = "CustomTitle";
const char* kEnableMetadata = "EnableMetadata";
const char* kEnableReconnect = "EnableReconnect";
const char* kEnabled = "Enabled";
const char* kFormat = "Format";
const char* kHost = "Host";
const char* kLimitReconnects = "LimitReconnects";
const char* kLogin = "Login";
const char* kMaximumRetries = "MaximumRetries";
const char* kMetadataCharset = "MetadataCharset";
const char* kMetadataFormat = "MetadataFormat";
const char* kMountPoint = "Mountpoint";
const char* kNoDelayFirstReconnect = "NoDelayFirstReconnect";
const char* kOggDynamicUpdate = "OggDynamicUpdate";
const char* kPassword = "Password";
const char* kPort = "Port";
const char* kReconnectFirstDelay = "ReconnectFirstDelay";
const char* kReconnectPeriod = "ReconnectPeriod";
const char* kServertype = "Servertype";
const char* kStreamDesc = "StreamDesc";
const char* kStreamGenre = "StreamGenre";
const char* kStreamName = "StreamName";
const char* kStreamPublic = "StreamPublic";
const char* kStreamWebsite = "StreamWebsite";

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
const QString kDefaultStreamDesc =
        QObject::tr("This stream is online for testing purposes!");
const QString kDefaultStreamGenre = QObject::tr("Live Mix");
const bool kDefaultStreamPublic = false;
const QRegExp kForbiddenChars = QRegExp("[<>:\"/\\\|\?\*]");
} // anonymous namespace

BroadcastProfile::BroadcastProfile(const QString& profileName) {
    defaultValues();
    setProfileName(profileName);
}

bool BroadcastProfile::checkNameCompliance(const QString& str) {
    return str.contains(kForbiddenChars);
}

BroadcastProfile* BroadcastProfile::loadFromFile(const QString& filename) {
    QFileInfo xmlFile(filename);
    if(!xmlFile.exists())
        return nullptr;

    QString profileName = xmlFile.baseName();

    BroadcastProfile* profile = new BroadcastProfile(profileName);
    profile->loadValues(filename);

    return profile;
}

void BroadcastProfile::defaultValues() {
    m_enabled = true;

    m_host = QString(),
    m_port = BROADCAST_DEFAULT_PORT;
    m_serverType = QString();
    m_login = QString();
    m_password = QString();

    m_enableReconnect = kDefaultEnableReconnect;
    m_reconnectPeriod = kDefaultReconnectPeriod;
    m_limitReconnects = kDefaultLimitReconnects;

    m_mountpoint = QString();
    m_streamDesc = kDefaultStreamDesc;
    m_streamGenre = kDefaultStreamGenre;
    m_streamName = QString();
    m_streamPublic = kDefaultStreamPublic;
    m_streamWebsite = MIXXX_WEBSITE_URL;

    m_enableMetadata = kDefaultEnableMetadata;
    m_metadataCharset = QString();
    m_customArtist = QString();
    m_customTitle = QString();
    m_metadataFormat = kDefaultMetadataFormat;
    m_oggDynamicUpdate = kDefaultOggDynamicupdate;

    m_bitrate = kDefaultBitrate;
    m_channels = kDefaultChannels;
    m_format = QString();

    m_noDelayFirstReconnect = kDefaultNoDelayFirstReconnect;
    m_reconnectFirstDelay = kDefaultReconnectFirstDelay;
    m_maximumRetries = kDefaultMaximumRetries;
}

void BroadcastProfile::loadValues(const QString& filename) {
    if(!QFile::exists(filename)) {
        setDefaultValues();
        return;
    }

    QDomElement xmlRoot = XmlParse::openXMLFile(filename, kDocumentName);

    m_enabled = (bool)XmlParse::selectNodeInt(xmlRoot, kEnabled);

    m_host = XmlParse::selectNodeQString(xmlRoot, kHost);
    m_port = XmlParse::selectNodeInt(xmlRoot, kPort);
    m_serverType = XmlParse::selectNodeQString(xmlRoot, kServertype);
    m_login = XmlParse::selectNodeQString(xmlRoot, kLogin);
    m_password = XmlParse::selectNodeQString(xmlRoot, kPassword);

    m_enableReconnect =
            (bool)XmlParse::selectNodeInt(xmlRoot, kEnableReconnect);
    m_reconnectPeriod =
            XmlParse::selectNodeDouble(xmlRoot, kReconnectPeriod);

    m_limitReconnects =
            (bool)XmlParse::selectNodeInt(xmlRoot, kLimitReconnects);
    m_maximumRetries =
            XmlParse::selectNodeInt(xmlRoot, kMaximumRetries);

    m_noDelayFirstReconnect =
            (bool)XmlParse::selectNodeInt(xmlRoot, kNoDelayFirstReconnect);
    m_reconnectFirstDelay =
            XmlParse::selectNodeDouble(xmlRoot, kReconnectFirstDelay);

    m_mountpoint = XmlParse::selectNodeQString(xmlRoot, kMountPoint);
    m_streamName = XmlParse::selectNodeQString(xmlRoot, kStreamName);
    m_streamDesc = XmlParse::selectNodeQString(xmlRoot, kStreamDesc);
    m_streamGenre = XmlParse::selectNodeQString(xmlRoot, kStreamGenre);
    m_streamPublic = (bool)XmlParse::selectNodeInt(xmlRoot, kStreamPublic);
    m_streamWebsite = XmlParse::selectNodeQString(xmlRoot, kStreamWebsite);

    m_format = XmlParse::selectNodeQString(xmlRoot, kFormat);
    m_bitrate = XmlParse::selectNodeInt(xmlRoot, kBitrate);
    m_channels = XmlParse::selectNodeInt(xmlRoot, kChannels);

    m_enableMetadata = (bool)XmlParse::selectNodeInt(xmlRoot, kEnableMetadata);
    m_metadataCharset = XmlParse::selectNodeQString(xmlRoot, kMetadataCharset);
    m_customArtist = XmlParse::selectNodeQString(xmlRoot, kCustomArtist);
    m_customTitle = XmlParse::selectNodeQString(xmlRoot, kCustomTitle);
    m_metadataFormat = XmlParse::selectNodeQString(xmlRoot, kMetadataFormat);
    m_oggDynamicUpdate =
            (bool)XmlParse::selectNodeInt(xmlRoot, kMetadataFormat);
}

void BroadcastProfile::save(const QString& filename) {
    QDomDocument doc(kDocumentName);
    QDomElement docRoot = doc.documentElement();

    XmlParse::addElement(doc, docRoot,
                         kEnabled, QString::number((int)m_enabled));

    XmlParse::addElement(doc, docRoot, kHost, m_host);
    XmlParse::addElement(doc, docRoot, kPort, QString::number(m_port));
    XmlParse::addElement(doc, docRoot, kServertype, m_serverType);
    XmlParse::addElement(doc, docRoot, kLogin, m_login);
    XmlParse::addElement(doc, docRoot, kPassword, m_password);

    XmlParse::addElement(doc, docRoot, kEnableReconnect,
                         QString::number((int)m_enableReconnect));
    XmlParse::addElement(doc, docRoot, kReconnectPeriod,
                         QString::number(m_reconnectPeriod));

    XmlParse::addElement(doc, docRoot, kLimitReconnects,
                         QString::number((int)m_limitReconnects));
    XmlParse::addElement(doc, docRoot, kMaximumRetries,
                         QString::number(m_maximumRetries));

    XmlParse::addElement(doc, docRoot, kNoDelayFirstReconnect,
                         QString::number((int)m_noDelayFirstReconnect));
    XmlParse::addElement(doc, docRoot, kReconnectFirstDelay,
                         QString::number(m_reconnectFirstDelay));

    XmlParse::addElement(doc, docRoot, kMountPoint, m_mountpoint);
    XmlParse::addElement(doc, docRoot, kStreamName, m_streamName);
    XmlParse::addElement(doc, docRoot, kStreamDesc, m_streamDesc);
    XmlParse::addElement(doc, docRoot, kStreamGenre, m_streamGenre);
    XmlParse::addElement(doc, docRoot, kStreamPublic,
                         QString::number((int)m_streamPublic));
    XmlParse::addElement(doc, docRoot, kStreamWebsite, m_streamWebsite);

    XmlParse::addElement(doc, docRoot, kFormat, m_format);
    XmlParse::addElement(doc, docRoot, kBitrate,
                         QString::number(m_bitrate));
    XmlParse::addElement(doc, docRoot, kChannels,
                         QString::number(m_channels));

    XmlParse::addElement(doc, docRoot, kEnableMetadata,
                         QString::number((int)m_enableMetadata));
    XmlParse::addElement(doc, docRoot, kMetadataCharset, m_metadataCharset);
    XmlParse::addElement(doc, docRoot, kCustomArtist, m_customArtist);
    XmlParse::addElement(doc, docRoot, kCustomTitle, m_customTitle);
    XmlParse::addElement(doc, docRoot, kMetadataFormat, m_metadataFormat);
    XmlParse::addElement(doc, docRoot, kOggDynamicUpdate,
                         QString::number((int)m_oggDynamicUpdate));


    QFile xmlFile(filename);
    xmlFile.open(QIODevice::WriteOnly | QIODevice::Text);

    QTextStream fileStream(&xmlFile);
    doc.save(fileStream, 4);
    xmlFile.close();
}

void BroadcastProfile::setProfileName(const QString &profileName) {
    m_profileName = QString(profileName);

    // Replace occurences of forbidden characters with a space
    // to avoid problems with the underlying filesystem.
    // Char list comes from MSDN article "Naming Files, Paths, and Namespaces".
    m_profileName.replace(kForbiddenChars, " ");
}

QString BroadcastProfile::getProfileName() const {
    return m_profileName;
}

bool BroadcastProfile::getEnabled() const {
    return m_enabled;
}

void BroadcastProfile::setEnabled(bool value) {
    m_enabled = value;
}

QString BroadcastProfile::getHost() const {
    return m_host;
}

void BroadcastProfile::setHost(const QString& value) {
    m_host = QString(value);
}

int BroadcastProfile::getPort() const {
    // Valid port numbers are 0 .. 65535 (16 bit unsigned)
    if (m_port < 0 || m_port > 0xFFFF) {
        return BROADCAST_DEFAULT_PORT;
    }

    return m_port;
}

void BroadcastProfile::setPort(int value) {
    m_port = value;
}

QString BroadcastProfile::getServertype() const {
    return m_serverType;
}

void BroadcastProfile::setServertype(const QString& value) {
    m_serverType = QString(value);
}

QString BroadcastProfile::getLogin() const {
    return m_login;
}

void BroadcastProfile::setLogin(const QString& value) {
    m_login = QString(value);
}

// TODO(Palakis, June 2nd 2017): implement secure password storage
QString BroadcastProfile::getPassword() const {
    return m_password;
}

void BroadcastProfile::setPassword(const QString& value) {
    m_password = QString(value);
}

bool BroadcastProfile::getEnableReconnect() const {
    return m_enableReconnect;
}

void BroadcastProfile::setEnableReconnect(bool value) {
    m_enableReconnect = value;
}

double BroadcastProfile::getReconnectPeriod() const {
    return m_reconnectPeriod;
}

void BroadcastProfile::setReconnectPeriod(double value) {
    m_reconnectPeriod = value;
}

bool BroadcastProfile::getLimitReconnects() const {
    return m_limitReconnects;
}

void BroadcastProfile::setLimitReconnects(bool value) {
    m_limitReconnects = value;
}

int BroadcastProfile::getMaximumRetries() const {
    return m_maximumRetries;
}

void BroadcastProfile::setMaximumRetries(int value) {
    m_maximumRetries = value;
}

bool BroadcastProfile::getNoDelayFirstReconnect() const {
    return m_noDelayFirstReconnect;
}

void BroadcastProfile::setNoDelayFirstReconnect(bool value) {
    m_noDelayFirstReconnect = value;
}

double BroadcastProfile::getReconnectFirstDelay() const {
    return m_reconnectFirstDelay;
}

void BroadcastProfile::setReconnectFirstDelay(double value) {
    m_reconnectFirstDelay = value;
}

QString BroadcastProfile::getMountpoint() const {
    return m_mountpoint;
}

void BroadcastProfile::setMountPoint(const QString& value) {
    m_mountpoint = QString(value);
}

QString BroadcastProfile::getStreamName() const {
    return m_streamName;
}

void BroadcastProfile::setStreamName(const QString& value) {
    m_streamName = QString(value);
}

QString BroadcastProfile::getStreamDesc() const {
    return m_streamDesc;
}

void BroadcastProfile::setStreamDesc(const QString& value) {
    m_streamDesc = QString(value);
}

QString BroadcastProfile::getStreamGenre() const {
    return m_streamGenre;
}

void BroadcastProfile::setStreamGenre(const QString& value) {
    m_streamGenre = QString(value);
}

bool BroadcastProfile::getStreamPublic() const {
    return m_streamPublic;
}

void BroadcastProfile::setStreamPublic(bool value) {
    m_streamPublic = value;
}

QString BroadcastProfile::getStreamWebsite() const {
    return m_streamWebsite;
}

void BroadcastProfile::setStreamWebsite(const QString& value) {
    m_streamWebsite = QString(value);
}

QString BroadcastProfile::getFormat() const {
    return m_format;
}

void BroadcastProfile::setFormat(const QString& value) {
    m_format = QString(value);
}

int BroadcastProfile::getBitrate() const {
    return m_bitrate;
}

void BroadcastProfile::setBitrate(int value) {
    m_bitrate = value;
}

int BroadcastProfile::getChannels() const {
    return m_channels;
}

void BroadcastProfile::setChannels(int value) {
    m_channels = value;
}

bool BroadcastProfile::getEnableMetadata() const {
    return m_enableMetadata;
}

void BroadcastProfile::setEnableMetadata(bool value) {
    m_enableMetadata = value;
}

QString BroadcastProfile::getMetadataCharset() const {
    return m_metadataCharset;
}

void BroadcastProfile::setMetadataCharset(const QString& value) {
    m_metadataCharset = QString(value);
}

QString BroadcastProfile::getCustomArtist() const {
    return m_customArtist;
}

void BroadcastProfile::setCustomArtist(const QString& value) {
    m_customArtist = QString(value);
}

QString BroadcastProfile::getCustomTitle() const {
    return m_customTitle;
}

void BroadcastProfile::setCustomTitle(const QString& value) {
    m_customTitle = QString(value);
}

QString BroadcastProfile::getMetadataFormat() const {
    return m_metadataFormat;
}

void BroadcastProfile::setMetadataFormat(const QString& value) {
    m_metadataFormat = QString(value);
}

bool BroadcastProfile::getOggDynamicUpdate() const {
    return m_oggDynamicUpdate;
}

void BroadcastProfile::setOggDynamicUpdate(bool value) {
    m_oggDynamicUpdate = value;
}
