#include <QMutexLocker>
#include <QtDebug>

#include "track/keys.h"
#include "track/keyutils.h"

using mixxx::track::io::key::ChromaticKey;
using mixxx::track::io::key::KeyMap;

Keys::Keys(const QByteArray* pByteArray) {
    if (pByteArray) {
        readByteArray(pByteArray);
    }
}

Keys::Keys(const KeyMap& keyMap)
        : m_keyMap(keyMap) {
}

Keys::Keys(const Keys& other)
        : m_subVersion(other.m_subVersion),
          m_keyMap(other.m_keyMap) {
}

Keys::~Keys() {
}

Keys& Keys::operator=(const Keys& other) {
    m_keyMap = other.m_keyMap;
    m_subVersion = other.m_subVersion;
    return *this;
}

QByteArray* Keys::toByteArray() const {
    QMutexLocker locker(&m_mutex);
    std::string output;
    m_keyMap.SerializeToString(&output);
    QByteArray* pByteArray = new QByteArray(output.data(), output.length());
    return pByteArray;
}

QString Keys::getVersion() const {
    return KEY_MAP_VERSION;
}

QString Keys::getSubVersion() const {
    QMutexLocker locker(&m_mutex);
    return m_subVersion;
}

void Keys::setSubVersion(const QString& subVersion) {
    QMutexLocker locker(&m_mutex);
    m_subVersion = subVersion;
}

bool Keys::isValid() const {
    return m_keyMap.global_key() != mixxx::track::io::key::INVALID ||
            m_keyMap.global_key_text().length() > 0;
}

ChromaticKey Keys::getGlobalKey() const {
    return m_keyMap.global_key();
}

QString Keys::getGlobalKeyText() const {
    return QString::fromStdString(m_keyMap.global_key_text());
}

void Keys::readByteArray(const QByteArray* pByteArray) {
    if (!m_keyMap.ParseFromArray(pByteArray->constData(), pByteArray->size())) {
        qDebug() << "ERROR: Could not parse Keys from QByteArray of size"
                 << pByteArray->size();
        return;
    }
}
