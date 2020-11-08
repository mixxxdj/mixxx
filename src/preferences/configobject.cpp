#include "preferences/configobject.h"

#include <QApplication>
#include <QDir>
#include <QIODevice>
#include <QTextStream>
#include <QtDebug>

#include "util/cmdlineargs.h"
#include "util/color/rgbcolor.h"
#include "util/xml.h"
#include "widget/wwidget.h"

// TODO(rryan): Move to a utility file.
namespace {
const QString kTempFilenameExtension = QStringLiteral(".tmp");

QString computeResourcePath() {
    // Try to read in the resource directory from the command line
    QString qResourcePath = CmdlineArgs::Instance().getResourcePath();

    if (qResourcePath.isEmpty()) {
        QDir mixxxDir(QCoreApplication::applicationDirPath());
        // We used to support using the mixxx.cfg's [Config],Path setting but
        // this causes issues if you try and use two different versions of Mixxx
        // on the same computer. See Bug #1392854. We start by checking if we're
        // running out of a build root ('res' dir exists or our path ends with
        // '_build') and if not then we fall back on a platform-specific method
        // of determining the resource path (see comments below).
        if (mixxxDir.cd("res")) {
            // We are running out of the repository root.
            qResourcePath = mixxxDir.absolutePath();
        } else if (mixxxDir.absolutePath().endsWith("_build") &&
                   mixxxDir.cdUp() && mixxxDir.cd("res")) {
            // We are running out of the (lin|win|osx)XX_build folder.
            qResourcePath = mixxxDir.absolutePath();
        }
#ifdef __UNIX__
        // On Linux if all of the above fail the /usr/share path is the logical
        // place to look.
        else {
            qResourcePath = UNIX_SHARE_PATH;
        }
#endif
#ifdef __WINDOWS__
        // On Windows, set the config dir relative to the application dir if all
        // of the above fail.
        else {
            qResourcePath = QCoreApplication::applicationDirPath();
        }
#endif
#ifdef __APPLE__
        else if (mixxxDir.cdUp() && mixxxDir.cd("Resources")) {
            // Release configuration
            qResourcePath = mixxxDir.absolutePath();
        } else {
            // TODO(rryan): What should we do here?
        }
#endif
    } else {
        //qDebug() << "Setting qResourcePath from location in resourcePath commandline arg:" << qResourcePath;
    }

    if (qResourcePath.isEmpty()) {
        reportCriticalErrorAndQuit("qConfigPath is empty, this can not be so -- did our developer forget to define one of __UNIX__, __WINDOWS__, __APPLE__??");
    }

    // If the directory does not end with a "/", add one
    if (!qResourcePath.endsWith("/")) {
        qResourcePath.append("/");
    }

    qDebug() << "Loading resources from " << qResourcePath;
    return qResourcePath;
}

QString computeSettingsPath(const QString& configFilename) {
    if (!configFilename.isEmpty()) {
        QFileInfo configFileInfo(configFilename);
        return configFileInfo.absoluteDir().absolutePath();
    }
    return QString();
}

}  // namespace
// static
ConfigKey ConfigKey::parseCommaSeparated(const QString& key) {
    int comma = key.indexOf(",");
    ConfigKey configKey(key.left(comma), key.mid(comma + 1));
    return configKey;
}

ConfigValue::ConfigValue(int iValue)
    : value(QString::number(iValue)) {
}

ConfigValue::ConfigValue(double dValue)
    : value(QString::number(dValue)) {
}

ConfigValueKbd::ConfigValueKbd(const QKeySequence& keys)
        : m_keys(std::move(keys)) {
    QTextStream(&value) << m_keys.toString();
}

template <class ValueType> ConfigObject<ValueType>::ConfigObject(const QString& file)
        : m_resourcePath(computeResourcePath()),
          m_settingsPath(computeSettingsPath(file)) {
    reopen(file);
}

template <class ValueType> ConfigObject<ValueType>::~ConfigObject() {
}

template <class ValueType>
void ConfigObject<ValueType>::set(const ConfigKey& k, const ValueType& v) {
    QWriteLocker lock(&m_valuesLock);
    m_values.insert(k, v);
}

template <class ValueType>
ValueType ConfigObject<ValueType>::get(const ConfigKey& k) const {
    QReadLocker lock(&m_valuesLock);
    return m_values.value(k);
}

template <class ValueType>
bool ConfigObject<ValueType>::exists(const ConfigKey& k) const {
    QReadLocker lock(&m_valuesLock);
    return m_values.contains(k);
}

template <class ValueType>
bool ConfigObject<ValueType>::remove(const ConfigKey& k) {
    QWriteLocker lock(&m_valuesLock);
    return m_values.remove(k) > 0;
}

template <class ValueType>
QString ConfigObject<ValueType>::getValueString(const ConfigKey& k) const {
    ValueType v = get(k);
    return v.value;
}

template <class ValueType> bool ConfigObject<ValueType>::parse() {
    // Open file for reading
    QFile configfile(m_filename);
    if (m_filename.length() < 1 || !configfile.open(QIODevice::ReadOnly)) {
        qDebug() << "ConfigObject: Could not read" << m_filename;
        return false;
    } else {
        //qDebug() << "ConfigObject: Parse" << m_filename;
        // Parse the file
        int group = 0;
        QString groupStr, line;
        QTextStream text(&configfile);
        text.setCodec("UTF-8");

        while (!text.atEnd()) {
            line = text.readLine().trimmed();
            if (line.length() != 0) {
                if (line.startsWith("[") && line.endsWith("]")) {
                    group++;
                    groupStr = line;
                    //qDebug() << "Group :" << groupStr;
                } else if (group > 0) {
                    QString key;
                    QTextStream(&line) >> key;
                    QString val = line.right(line.length() - key.length()); // finds the value string
                    val = val.trimmed();
                    //qDebug() << "control:" << key << "value:" << val;
                    ConfigKey k(groupStr, key);
                    ValueType m(val);
                    set(k, m);
                }
            }
        }
        configfile.close();
    }
    return true;
}

template <class ValueType> void ConfigObject<ValueType>::reopen(const QString& file) {
    m_filename = file;
    if (!m_filename.isEmpty()) {
        parse();
    }
}

/// Save the ConfigObject to disk.
/// Returns true on success
template<class ValueType>
bool ConfigObject<ValueType>::save() {
    QReadLocker lock(&m_valuesLock); // we only read the m_values here.
    QFile tmpFile(m_filename + kTempFilenameExtension);
    if (!QDir(QFileInfo(tmpFile).absolutePath()).exists()) {
        QDir().mkpath(QFileInfo(tmpFile).absolutePath());
    }
    if (!tmpFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Could not write config file: " << tmpFile.fileName();
        return false;
    }
    QTextStream stream(&tmpFile);
    stream.setCodec("UTF-8");

    QString group = "";

    // Since it is legit to have a ConfigObject with 0 values, checking
    // the stream.pos alone will yield wrong warnings. We therefore estimate
    // a minimum length as an additional safety check.
    qint64 minLength = 0;
    for (auto i = m_values.constBegin(); i != m_values.constEnd(); ++i) {
        //qDebug() << "group:" << it.key().group << "item" << it.key().item << "val" << it.value()->value;
        if (i.key().group != group) {
            group = i.key().group;
            stream << "\n"
                   << group << "\n";
            minLength += i.key().group.length() + 2;
        }
        stream << i.key().item << " " << i.value().value << "\n";
        minLength += i.key().item.length() + i.value().value.length() + 1;
    }

    stream.flush();
    // the stream is usually longer, depending on the amount of encoded data.
    if (stream.pos() < minLength || QFileInfo(tmpFile).size() != stream.pos()) {
        qWarning().nospace() << "Error while writing configuration file: " << tmpFile.fileName();
        return false;
    }

    tmpFile.close();
    if (tmpFile.error() !=
            QFile::NoError) { //could be better... should actually say what the error was..
        qWarning().nospace() << "Error while writing configuration file: "
                             << tmpFile.fileName() << ": " << tmpFile.errorString();
        return false;
    }

    QFile oldConfig(m_filename);
    if (!oldConfig.remove()) {
        qWarning().nospace() << "Could not remove old config file: "
                             << oldConfig.fileName() << ": " << oldConfig.errorString();
        return false;
    }
    if (!tmpFile.rename(m_filename)) {
        qWarning().nospace() << "Could not rename tmp file to config file: "
                             << tmpFile.fileName() << ": " << tmpFile.errorString();
        return false;
    }

    return true;
}

template<class ValueType>
QSet<QString> ConfigObject<ValueType>::getGroups() {
    QWriteLocker lock(&m_valuesLock);
    QSet<QString> groups;
    for (const ConfigKey& key : m_values.keys()) {
        groups.insert(key.group);
    }
    return groups;
}

template<class ValueType>
QList<ConfigKey> ConfigObject<ValueType>::getKeysWithGroup(QString group) const {
    QWriteLocker lock(&m_valuesLock);
    QList<ConfigKey> filteredList;
    for (const ConfigKey& key : m_values.keys()) {
        if (key.group == group) {
            filteredList.append(key);
        }
    }
    return filteredList;
}

template <class ValueType> ConfigObject<ValueType>::ConfigObject(const QDomNode& node) {
    if (!node.isNull() && node.isElement()) {
        QDomNode ctrl = node.firstChild();

        while (!ctrl.isNull()) {
            if (ctrl.nodeName() == "control") {
                QString group = XmlParse::selectNodeQString(ctrl, "group");
                QString key = XmlParse::selectNodeQString(ctrl, "key");
                ConfigKey k(group, key);
                ValueType m(ctrl);
                set(k, m);
            }
            ctrl = ctrl.nextSibling();
        }
    }
}

template <class ValueType>
QMultiHash<ValueType, ConfigKey> ConfigObject<ValueType>::transpose() const {
    QReadLocker lock(&m_valuesLock);

    QMultiHash<ValueType, ConfigKey> transposedHash;
    for (auto it = m_values.constBegin(); it != m_values.constEnd(); ++it) {
        transposedHash.insert(it.value(), it.key());
    }
    return transposedHash;
}

template class ConfigObject<ConfigValue>;
template class ConfigObject<ConfigValueKbd>;

template <> template <>
void ConfigObject<ConfigValue>::setValue(
        const ConfigKey& key, const QString& value) {
    set(key, ConfigValue(value));
}

template <> template <>
void ConfigObject<ConfigValue>::setValue(
        const ConfigKey& key, const bool& value) {
    set(key, value ? ConfigValue("1") : ConfigValue("0"));
}

template <> template <>
void ConfigObject<ConfigValue>::setValue(
        const ConfigKey& key, const int& value) {
    set(key, ConfigValue(QString::number(value)));
}

template <> template <>
void ConfigObject<ConfigValue>::setValue(
        const ConfigKey& key, const double& value) {
    set(key, ConfigValue(QString::number(value)));
}

template<>
template<>
void ConfigObject<ConfigValue>::setValue(
        const ConfigKey& key, const mixxx::RgbColor::optional_t& value) {
    if (!value) {
        remove(key);
        return;
    }
    set(key, ConfigValue(mixxx::RgbColor::toQString(value)));
}

template<>
template<>
void ConfigObject<ConfigValue>::setValue(
        const ConfigKey& key, const mixxx::RgbColor& value) {
    set(key, ConfigValue(mixxx::RgbColor::toQString(value)));
}

template <> template <>
bool ConfigObject<ConfigValue>::getValue(
        const ConfigKey& key, const bool& default_value) const {
    const ConfigValue value = get(key);
    if (value.isNull()) {
        return default_value;
    }
    bool ok;
    auto result = value.value.toInt(&ok);
    return ok ? result != 0 : default_value;
}

template <> template <>
int ConfigObject<ConfigValue>::getValue(
        const ConfigKey& key, const int& default_value) const {
    const ConfigValue value = get(key);
    if (value.isNull()) {
        return default_value;
    }
    bool ok;
    auto result = value.value.toInt(&ok);
    return ok ? result : default_value;
}

template <> template <>
double ConfigObject<ConfigValue>::getValue(
        const ConfigKey& key, const double& default_value) const {
    const ConfigValue value = get(key);
    if (value.isNull()) {
        return default_value;
    }
    bool ok;
    auto result = value.value.toDouble(&ok);
    return ok ? result : default_value;
}

template<>
template<>
mixxx::RgbColor::optional_t ConfigObject<ConfigValue>::getValue(
        const ConfigKey& key, const mixxx::RgbColor::optional_t& default_value) const {
    const ConfigValue value = get(key);
    if (value.isNull()) {
        return default_value;
    }
    return mixxx::RgbColor::fromQString(value.value, default_value);
}

template<>
template<>
mixxx::RgbColor::optional_t ConfigObject<ConfigValue>::getValue(const ConfigKey& key) const {
    return getValue(key, mixxx::RgbColor::optional_t(std::nullopt));
}

template<>
template<>
mixxx::RgbColor ConfigObject<ConfigValue>::getValue(
        const ConfigKey& key, const mixxx::RgbColor& default_value) const {
    const mixxx::RgbColor::optional_t value = getValue(key, mixxx::RgbColor::optional_t(std::nullopt));
    if (!value) {
        return default_value;
    }
    return *value;
}

template<>
template<>
mixxx::RgbColor ConfigObject<ConfigValue>::getValue(const ConfigKey& key) const {
    return getValue(key, mixxx::RgbColor(0));
}

// For string literal default
template <>
QString ConfigObject<ConfigValue>::getValue(
        const ConfigKey& key, const char* default_value) const {
    const ConfigValue value = get(key);
    if (value.isNull()) {
        return QString(default_value);
    }
    return value.value;
}

template <>
QString ConfigObject<ConfigValueKbd>::getValue(
        const ConfigKey& key, const char* default_value) const {
    const ConfigValueKbd value = get(key);
    if (value.isNull()) {
        return QString(default_value);
    }
    return value.value;
}

template <> template <>
QString ConfigObject<ConfigValue>::getValue(
        const ConfigKey& key, const QString& default_value) const {
    const ConfigValue value = get(key);
    if (value.isNull()) {
        return default_value;
    }
    return value.value;
}

template <> template <>
QString ConfigObject<ConfigValueKbd>::getValue(
        const ConfigKey& key, const QString& default_value) const {
    const ConfigValueKbd value = get(key);
    if (value.isNull()) {
        return default_value;
    }
    return value.value;
}
