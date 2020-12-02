#include <QMutexLocker>
#include <QtDebug>

#include "track/keys.h"

using mixxx::track::io::key::ChromaticKey;
using mixxx::track::io::key::KeyMap;

Keys::Keys(const QByteArray* pByteArray) {
    if (nullptr != pByteArray) {
        readByteArray(*pByteArray);
    }
}

Keys::Keys(const KeyMap& keyMap)
        : m_keyMap(keyMap) {
}

QByteArray Keys::toByteArray() const {
    std::string output;
    m_keyMap.SerializeToString(&output);
    return QByteArray(output.data(), static_cast<int>(output.length()));
}

const QString& Keys::getSubVersion() const {
    return m_subVersion;
}

void Keys::setSubVersion(const QString& subVersion) {
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

bool Keys::readByteArray(const QByteArray& byteArray) {
    if (!m_keyMap.ParseFromArray(byteArray.constData(), byteArray.size())) {
        qDebug() << "ERROR: Could not parse Keys from QByteArray of size"
                 << byteArray.size();
        return false;
    }
    return true;
}
