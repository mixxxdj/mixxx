#include "preferences/broadcastprofile.h"

#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QTextStream>

#ifdef __QTKEYCHAIN__
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <qt6keychain/keychain.h>
#else
#include <qt5keychain/keychain.h>
#endif
using namespace QKeychain;
#endif // __QTKEYCHAIN__

#include "broadcast/defs_broadcast.h"
#include "defs_urls.h"
#include "errordialoghandler.h"
#include "moc_broadcastprofile.cpp"
#include "recording/defs_recording.h"
#include "util/compatibility/qatomic.h"
#include "util/logger.h"
#include "util/xml.h"

namespace {
constexpr const char* kDoctype = "broadcastprofile";
constexpr const char* kDocumentRoot = "BroadcastProfile";
constexpr const char* kSecureCredentials = "SecureCredentialsStorage";
constexpr const char* kBitrate = "Bitrate";
constexpr const char* kChannels = "Channels";
constexpr const char* kCustomArtist = "CustomArtist";
constexpr const char* kCustomTitle = "CustomTitle";
constexpr const char* kEnableMetadata = "EnableMetadata";
constexpr const char* kEnableReconnect = "EnableReconnect";
constexpr const char* kEnabled = "Enabled";
constexpr const char* kFormat = "Format";
constexpr const char* kHost = "Host";
constexpr const char* kLimitReconnects = "LimitReconnects";
constexpr const char* kLogin = "Login";
constexpr const char* kMaximumRetries = "MaximumRetries";
constexpr const char* kMetadataCharset = "MetadataCharset";
constexpr const char* kMetadataFormat = "MetadataFormat";
constexpr const char* kMountPoint = "Mountpoint";
constexpr const char* kNoDelayFirstReconnect = "NoDelayFirstReconnect";
constexpr const char* kOggDynamicUpdate = "OggDynamicUpdate";
constexpr const char* kPassword = "Password";
constexpr const char* kPort = "Port";
constexpr const char* kProfileName = "ProfileName";
constexpr const char* kReconnectFirstDelay = "ReconnectFirstDelay";
constexpr const char* kReconnectPeriod = "ReconnectPeriod";
constexpr const char* kServertype = "Servertype";
constexpr const char* kStreamDesc = "StreamDesc";
constexpr const char* kStreamGenre = "StreamGenre";
constexpr const char* kStreamName = "StreamName";
constexpr const char* kStreamIRC = "StreamIRC";
constexpr const char* kStreamAIM = "StreamAIM";
constexpr const char* kStreamICQ = "StreamICQ";
constexpr const char* kStreamPublic = "StreamPublic";
constexpr const char* kStreamWebsite = "StreamWebsite";

#ifdef __QTKEYCHAIN__
constexpr const char* kKeychainPrefix = "Mixxx - ";
#endif

constexpr int kDefaultBitrate = 128;
constexpr int kDefaultChannels = 2;
constexpr bool kDefaultEnableMetadata = false;
constexpr bool kDefaultEnableReconnect = true;
constexpr bool kDefaultLimitReconnects = true;
constexpr int kDefaultMaximumRetries = 10;
// No tr() here, see https://github.com/mixxxdj/mixxx/issues/7848
const QString kDefaultMetadataFormat("$artist - $title");
constexpr bool kDefaultNoDelayFirstReconnect = true;
constexpr bool kDefaultOggDynamicupdate = false;
constexpr double kDefaultReconnectFirstDelay = 0.0;
constexpr double kDefaultReconnectPeriod = 5.0;
const QString kDefaultStreamName = QStringLiteral("Mixxx");
const QString kDefaultStreamDesc =
        QObject::tr("This stream is online for testing purposes!");
const QString kDefaultStreamGenre = QObject::tr("Live Mix");
constexpr bool kDefaultStreamPublic = false;

// Characters not allowed for profile name
const QRegularExpression kForbiddenChars =
        QRegularExpression(QStringLiteral(
                "[<>:\"\\/|?*\\\\]|(\\.\\.)"
                "|CON|AUX|PRN|COM(\\d+)|LPT(\\d+)|NUL"));
// Characters known to cause trouble for various fields, especially host, login
// and password.
// See https://mixxx.discourse.group/t/broadcasting-connection-problems/32861/3
const QRegularExpression kControlChars =
        QRegularExpression(QStringLiteral("[\\x00-\\x1F\\x7F]"));

const mixxx::Logger kLogger("BroadcastProfile");
} // anonymous namespace

BroadcastProfile::BroadcastProfile(const QString& profileName,
                                   QObject* parent)
    : QObject(parent) {
    adoptDefaultValues();

    // Direct assignment to avoid triggering the
    // profileNameChanged signal
    m_profileName = QString(profileName);
}

bool BroadcastProfile::validName(const QString& str) {
    return !str.contains(kForbiddenChars);
}

bool BroadcastProfile::validPassword() {
    return validPassword(m_password);
}

bool BroadcastProfile::validPassword(const QString& input) {
    // Here we get either the strings decoded from Xml when reading the profile
    // or the password QString we received from the preferences.
    return !input.contains(kControlChars);
}

QString BroadcastProfile::stripForbiddenChars(const QString& str) {
    QString sourceText(str);
    return sourceText.replace(kForbiddenChars, " ");
}

QString BroadcastProfile::removeControlCharacters(
        const QString& str,
        const QString& fieldName,
        bool* pFixed) {
    // Remove all control characters like %#d; (\r, Carriage Return)
    // If the config has been corrupted manually, for example by adding linebreaks,
    // we may also have literals like \n. We don't care about those because
    // a) we can't tell whether these are there on purpose
    // b) in contrast to for example &#xd; these are visible in the Password QLineEdit.
    QString input(str);
    input.remove(kControlChars);
    if (str != input) {
        qWarning() << "BroadcastProfile: removed invalid characters from"
                   << fieldName << "field";
        if (pFixed && !(*pFixed)) {
            *pFixed = true;
        }
    }
    return input;
}

QString BroadcastProfile::selectCleanNodeString(
        const QDomElement& doc,
        const QString& fieldName,
        bool* pFixed) {
    return removeControlCharacters(
            XmlParse::selectNodeQString(doc, fieldName),
            fieldName,
            pFixed);
}

BroadcastProfilePtr BroadcastProfile::loadFromFile(
        const QString& filename) {
    QFileInfo xmlFile(filename);
    if (!xmlFile.exists()) {
        return BroadcastProfilePtr(nullptr);
    }

    QString profileFilename = xmlFile.baseName();
    // The profile filename (without extension) is used to create the instance
    // but the real profile name (with forbidden chars but suitable for
    // non-filesystem uses) will be fetched from the XML file and set in the
    // object during the call to loadValues()
    BroadcastProfilePtr profile(new BroadcastProfile(profileFilename));
    profile->loadValues(filename);
    return profile;
}

QString BroadcastProfile::getLastFilename() const {
    return m_filename;
}

bool BroadcastProfile::equals(BroadcastProfilePtr other) {
    return ((getProfileName() == other->getProfileName())
            && valuesEquals(other));
}

bool BroadcastProfile::valuesEquals(BroadcastProfilePtr other) {
    if (getEnabled() == other->getEnabled()
            && secureCredentialStorage() == other->secureCredentialStorage()
            && getHost() == other->getHost()
            && getPort() == other->getPort()
            && getServertype() == other->getServertype()
            && getLogin() == other->getLogin()
            && getPassword() == other->getPassword()
            && getEnableReconnect() == other->getEnableReconnect()
            && getReconnectPeriod() == other->getReconnectPeriod()
            && getLimitReconnects() == other->getLimitReconnects()
            && getMaximumRetries() == other->getMaximumRetries()
            && getNoDelayFirstReconnect() == other->getNoDelayFirstReconnect()
            && getReconnectFirstDelay() == other->getReconnectFirstDelay()
            && getFormat() == other->getFormat()
            && getBitrate() == other->getBitrate()
            && getChannels() == other->getChannels()
            && getMountpoint() == other->getMountpoint()
            && getStreamName() == other->getStreamName()
            && getStreamDesc() == other->getStreamDesc()
            && getStreamGenre() == other->getStreamGenre()
            && getStreamPublic() == other->getStreamPublic()
            && getStreamWebsite() == other->getStreamWebsite()
            && getStreamIRC() == other->getStreamIRC()
            && getStreamAIM() == other->getStreamAIM()
            && getStreamICQ() == other->getStreamICQ()
            && getEnableMetadata() == other->getEnableMetadata()
            && getMetadataCharset() == other->getMetadataCharset()
            && getCustomArtist() == other->getCustomArtist()
            && getCustomTitle() == other->getCustomTitle()
            && getMetadataFormat() == other->getMetadataFormat()
            && getOggDynamicUpdate() == other->getOggDynamicUpdate()) {
        return true;
    }

    return false;
}

BroadcastProfilePtr BroadcastProfile::valuesCopy() {
    BroadcastProfilePtr newProfile(
            new BroadcastProfile(getProfileName()));
    copyValuesTo(newProfile);
    return newProfile;
}

void BroadcastProfile::copyValuesTo(BroadcastProfilePtr other) {
    other->setSecureCredentialStorage(this->secureCredentialStorage());

    other->setHost(this->getHost());
    other->setPort(this->getPort());

    other->setServertype(this->getServertype());
    other->setLogin(this->getLogin());
    other->setPassword(this->getPassword());

    other->setEnableReconnect(this->getEnableReconnect());
    other->setReconnectPeriod(this->getReconnectPeriod());

    other->setLimitReconnects(this->getLimitReconnects());
    other->setMaximumRetries(this->getMaximumRetries());

    other->setNoDelayFirstReconnect(this->getNoDelayFirstReconnect());
    other->setReconnectFirstDelay(this->getReconnectFirstDelay());

    other->setFormat(this->getFormat());
    other->setBitrate(this->getBitrate());
    other->setChannels(this->getChannels());

    other->setMountPoint(this->getMountpoint());
    other->setStreamName(this->getStreamName());
    other->setStreamDesc(this->getStreamDesc());
    other->setStreamGenre(this->getStreamGenre());
    other->setStreamPublic(this->getStreamPublic());
    other->setStreamWebsite(this->getStreamWebsite());
    other->setStreamIRC(this->getStreamIRC());
    other->setStreamAIM(this->getStreamAIM());
    other->setStreamICQ(this->getStreamICQ());

    other->setEnableMetadata(this->getEnableMetadata());
    other->setMetadataCharset(this->getMetadataCharset());
    other->setCustomArtist(this->getCustomArtist());
    other->setCustomTitle(this->getCustomTitle());
    other->setMetadataFormat(this->getMetadataFormat());
    other->setOggDynamicUpdate(this->getOggDynamicUpdate());

    other->setEnabled(this->getEnabled());
}

void BroadcastProfile::adoptDefaultValues() {
    m_secureCredentials = false;
    m_enabled = false;

    m_host = QString();
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
    m_streamName = kDefaultStreamName;
    m_streamPublic = kDefaultStreamPublic;
    m_streamWebsite = MIXXX_WEBSITE_URL;
    m_streamIRC.clear();
    m_streamAIM.clear();
    m_streamICQ.clear();

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

bool BroadcastProfile::loadValues(const QString& filename) {
    QDomElement doc = XmlParse::openXMLFile(filename, kDoctype);
    if (doc.childNodes().size() < 1) {
        return false;
    }

    m_filename = filename;

#ifdef __QTKEYCHAIN__
    m_secureCredentials = (bool)XmlParse::selectNodeInt(doc, kSecureCredentials);
#else
    // Secure credentials storage can't be enabled nor disabled from the UI,
    // so force it to disabled to avoid issues if enabled.
    m_secureCredentials = false;
#endif

    // ProfileName is special because it was not previously saved in the file.
    // When loading old files, we need to use the file name (set in the
    // constructor) as the profile name and only load it if present in the
    // file.
    QDomNode node = XmlParse::selectNode(doc, kProfileName);
    if (!node.isNull()) {
        m_profileName = node.toElement().text();
    }

    m_enabled = (bool)XmlParse::selectNodeInt(doc, kEnabled);

    // Check if we have to remove control chars.
    // If yes, write the config back to file
    bool fixedStrings = false;
    m_host = selectCleanNodeString(doc, kHost, &fixedStrings);
    m_port = XmlParse::selectNodeInt(doc, kPort);
    m_serverType = selectCleanNodeString(doc, kServertype, &fixedStrings);

    m_login = selectCleanNodeString(doc, kLogin, &fixedStrings);
    if (m_secureCredentials) {
        m_password = getSecurePassword(m_login);
    } else {
        m_password = XmlParse::selectNodeQString(doc, kPassword);
    }
    // If password contains invalid characters, for example linebreaks,
    // we log a warning. When attempting to connect with invalid credentials
    // users will get a warning popup and need to check Broadcasting
    // preferences which will notify them, too.
    if (!validPassword()) {
        qWarning() << "BroadcastProfile::loadValues: stored password contains invalid characters!";
    }

    m_enableReconnect =
            (bool)XmlParse::selectNodeInt(doc, kEnableReconnect);
    m_reconnectPeriod =
            XmlParse::selectNodeDouble(doc, kReconnectPeriod);

    m_limitReconnects =
            (bool)XmlParse::selectNodeInt(doc, kLimitReconnects);
    m_maximumRetries =
            XmlParse::selectNodeInt(doc, kMaximumRetries);

    m_noDelayFirstReconnect =
            (bool)XmlParse::selectNodeInt(doc, kNoDelayFirstReconnect);
    m_reconnectFirstDelay =
            XmlParse::selectNodeDouble(doc, kReconnectFirstDelay);

    m_mountpoint = selectCleanNodeString(doc, kMountPoint, &fixedStrings);
    m_streamName = selectCleanNodeString(doc, kStreamName, &fixedStrings);
    m_streamDesc = selectCleanNodeString(doc, kStreamDesc, &fixedStrings);
    m_streamGenre = selectCleanNodeString(doc, kStreamGenre, &fixedStrings);
    m_streamPublic = (bool)XmlParse::selectNodeInt(doc, kStreamPublic);
    m_streamWebsite = selectCleanNodeString(doc, kStreamWebsite, &fixedStrings);
    m_streamIRC = selectCleanNodeString(doc, kStreamIRC, &fixedStrings);
    m_streamAIM = selectCleanNodeString(doc, kStreamAIM, &fixedStrings);
    m_streamICQ = selectCleanNodeString(doc, kStreamICQ, &fixedStrings);

    m_format = selectCleanNodeString(doc, kFormat, &fixedStrings);
    if (m_format == BROADCAST_FORMAT_OV_LEGACY) {
        // Upgrade to have the same codec name than the recording define.
        m_format = ENCODING_OGG;
    }
    m_bitrate = XmlParse::selectNodeInt(doc, kBitrate);
    m_channels = XmlParse::selectNodeInt(doc, kChannels);

    m_enableMetadata = (bool)XmlParse::selectNodeInt(doc, kEnableMetadata);
    m_metadataCharset = selectCleanNodeString(doc, kMetadataCharset, &fixedStrings);
    m_customArtist = selectCleanNodeString(doc, kCustomArtist, &fixedStrings);
    m_customTitle = selectCleanNodeString(doc, kCustomTitle, &fixedStrings);
    m_metadataFormat = selectCleanNodeString(doc, kMetadataFormat, &fixedStrings);
    m_oggDynamicUpdate =
            (bool)XmlParse::selectNodeInt(doc, kOggDynamicUpdate);

    if (fixedStrings) {
        if (save(m_filename)) {
            qWarning() << "BroadcastProfile::loadValues: wrote config for profile"
                       << m_profileName
                       << "with corrected field strings back to file";
        } else {
            qWarning() << "BroadcastProfile::loadValues: failed to write config for profile"
                       << m_profileName
                       << "with corrected field strings back to file!";
        }
    }

    return true;
}

bool BroadcastProfile::save(const QString& filename) {
    QDomDocument doc(kDoctype);
    QDomElement docRoot = doc.createElement(kDocumentRoot);

    XmlParse::addElement(doc, docRoot, kProfileName, m_profileName);

    XmlParse::addElement(doc, docRoot,
                         kSecureCredentials, QString::number((int)m_secureCredentials));
    XmlParse::addElement(doc, docRoot,
                         kEnabled, QString::number((int)m_enabled));

    XmlParse::addElement(doc, docRoot, kHost, m_host);
    XmlParse::addElement(doc, docRoot, kPort, QString::number(m_port));
    XmlParse::addElement(doc, docRoot, kServertype, m_serverType);

    XmlParse::addElement(doc, docRoot, kLogin, m_login);
    if (m_secureCredentials) {
        setSecurePassword(m_login, m_password);
    } else {
        XmlParse::addElement(doc, docRoot, kPassword, m_password);
    }

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
    XmlParse::addElement(doc, docRoot, kStreamIRC, m_streamIRC);
    XmlParse::addElement(doc, docRoot, kStreamAIM, m_streamAIM);
    XmlParse::addElement(doc, docRoot, kStreamICQ, m_streamICQ);

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

    doc.appendChild(docRoot);

    QFile xmlFile(filename);
    if (xmlFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_filename = filename;
        QTextStream fileStream(&xmlFile);
        doc.save(fileStream, 4);
        xmlFile.close();

        return true;
    }
    return false;
}

void BroadcastProfile::setProfileName(const QString &profileName) {
    QString oldName(m_profileName);
    m_profileName = removeControlCharacters(profileName, kProfileName);

    emit profileNameChanged(oldName, m_profileName);
}

QString BroadcastProfile::getProfileName() const {
    return m_profileName;
}

void BroadcastProfile::setConnectionStatus(int newState) {
    m_connectionStatus = newState;
    emit connectionStatusChanged(connectionStatus());
}

int BroadcastProfile::connectionStatus() {
    return atomicLoadRelaxed(m_connectionStatus);
}

void BroadcastProfile::setSecureCredentialStorage(bool value) {
    m_secureCredentials = value;
}

bool BroadcastProfile::secureCredentialStorage() {
    return m_secureCredentials;
}

bool BroadcastProfile::setSecurePassword(const QString& login, const QString& password) {
    if (!validPassword(password)) {
        qWarning() << "BroadcastProfile::setSecurePassword: password contains invalid characters!";
    }
#ifdef __QTKEYCHAIN__
    QString serviceName = QString(kKeychainPrefix) + getProfileName();

    WritePasswordJob writeJob(serviceName);
    writeJob.setAutoDelete(false);
    writeJob.setKey(login);
    writeJob.setTextData(password);

    QEventLoop loop;
    writeJob.connect(&writeJob, &WritePasswordJob::finished, &loop, &QEventLoop::quit);
    writeJob.start();
    loop.exec();

    if (writeJob.error() == Error::NoError) {
        kLogger.debug() << "setSecureValue: write successful";
        return true;
    } else {
        kLogger.warning() << "setSecureValue: write job failed with error:"
                << writeJob.errorString();
        errorDialog(tr("Can't use secure password storage: keychain access failed."),
                writeJob.errorString());
        return false;
    }
#else
    Q_UNUSED(login);
    Q_UNUSED(password);
    return false;
#endif
}

QString BroadcastProfile::getSecurePassword(const QString& login) {
#ifdef __QTKEYCHAIN__
    QString serviceName = QString(kKeychainPrefix) + getProfileName();

    ReadPasswordJob readJob(serviceName);
    readJob.setAutoDelete(false);
    readJob.setKey(login);

    QEventLoop loop;
    readJob.connect(&readJob, &ReadPasswordJob::finished, &loop, &QEventLoop::quit);
    readJob.start();
    loop.exec();

    if (readJob.error() == Error::NoError) {
        kLogger.debug() << "getSecureValue: read successful";
        const QString password = readJob.textData();
        if (!validPassword(password)) {
            // Don't alter the password. user will get notified when trying to connect
            // and when entering invalid characters in the preferences
            qWarning() << "BroadcastProfile::getSecurePassword: password "
                          "contains invalid characters!";
        }
        return password;
    } else {
        kLogger.warning() << "getSecureValue: read job failed with error:"
                        << readJob.errorString();
        errorDialog(tr("Secure password retrieval unsuccessful: keychain access failed."),
                        readJob.errorString());
    }
#else
    Q_UNUSED(login);
#endif
    return QString();
}

void BroadcastProfile::errorDialog(const QString& text, const QString& detailedError) {
    ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
    props->setType(DLG_WARNING);
    props->setTitle(tr("Settings error"));
    props->setText(tr("<b>Error with settings for '%1':</b><br>")
            .arg(getProfileName()) + text);
    props->setDetails(detailedError);
    props->setKey(detailedError);   // To prevent multiple windows for the same error
    props->setDefaultButton(QMessageBox::Close);
    props->setModal(false);
    ErrorDialogHandler::instance()->requestErrorDialog(props);
}

void BroadcastProfile::relayStatus(bool newStatus) {
    setEnabled(newStatus);
}

// Used by BroadcastSettings to relay connection status
// to copies in BroadcastSettingsModel
void BroadcastProfile::relayConnectionStatus(int newConnectionStatus) {
    setConnectionStatus(newConnectionStatus);
}

// This was useless before, but now comes in handy for multi-broadcasting,
// where it means "this connection is enabled and will be started by Mixxx"
bool BroadcastProfile::getEnabled() const {
    return m_enabled;
}

void BroadcastProfile::setEnabled(bool value) {
    m_enabled = value;
    emit statusChanged(m_enabled);
}

QString BroadcastProfile::getHost() const {
    return m_host;
}

void BroadcastProfile::setHost(const QString& value) {
    m_host = removeControlCharacters(value, kHost);
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
    m_login = removeControlCharacters(value, kLogin);
}

// TODO(Palakis, June 2nd 2017): implement secure password storage
QString BroadcastProfile::getPassword() const {
    return m_password;
}

void BroadcastProfile::setPassword(const QString& value) {
    if (!validPassword(value)) {
        // Don't alter the password. user will get notified when trying to connect
        // and when entering invalid characters in the preferences
        qWarning() << "BroadcastProfile::setPassword: password contains invalid characters!";
    }
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
    m_mountpoint = removeControlCharacters(value, kMountPoint);
}

QString BroadcastProfile::getStreamName() const {
    return m_streamName;
}

void BroadcastProfile::setStreamName(const QString& value) {
    m_streamName = removeControlCharacters(value, kStreamName);
}

QString BroadcastProfile::getStreamDesc() const {
    return m_streamDesc;
}

void BroadcastProfile::setStreamDesc(const QString& value) {
    m_streamDesc = removeControlCharacters(value, kStreamDesc);
}

QString BroadcastProfile::getStreamGenre() const {
    return m_streamGenre;
}

void BroadcastProfile::setStreamGenre(const QString& value) {
    m_streamGenre = removeControlCharacters(value, kStreamGenre);
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
    m_streamWebsite = removeControlCharacters(value, kStreamWebsite);
}

QString BroadcastProfile::getStreamIRC() const {
    return m_streamIRC;
}

void BroadcastProfile::setStreamIRC(const QString& value) {
    m_streamIRC = removeControlCharacters(value, kStreamIRC);
}

QString BroadcastProfile::getStreamAIM() const {
    return m_streamAIM;
}

void BroadcastProfile::setStreamAIM(const QString& value) {
    m_streamAIM = removeControlCharacters(value, kStreamAIM);
}

QString BroadcastProfile::getStreamICQ() const {
    return m_streamICQ;
}

void BroadcastProfile::setStreamICQ(const QString& value) {
    m_streamICQ = removeControlCharacters(value, kStreamICQ);
}

QString BroadcastProfile::getFormat() const {
    return m_format;
}

void BroadcastProfile::setFormat(const QString& value) {
    m_format = removeControlCharacters(value, kFormat);
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
    m_metadataCharset = removeControlCharacters(value, kMetadataCharset);
}

QString BroadcastProfile::getCustomArtist() const {
    return m_customArtist;
}

void BroadcastProfile::setCustomArtist(const QString& value) {
    m_customArtist = removeControlCharacters(value, kCustomArtist);
}

QString BroadcastProfile::getCustomTitle() const {
    return m_customTitle;
}

void BroadcastProfile::setCustomTitle(const QString& value) {
    m_customTitle = removeControlCharacters(value, kCustomTitle);
}

QString BroadcastProfile::getMetadataFormat() const {
    return m_metadataFormat;
}

void BroadcastProfile::setMetadataFormat(const QString& value) {
    m_metadataFormat = removeControlCharacters(value, kMetadataFormat);
}

bool BroadcastProfile::getOggDynamicUpdate() const {
    return m_oggDynamicUpdate;
}

void BroadcastProfile::setOggDynamicUpdate(bool value) {
    m_oggDynamicUpdate = value;
}
