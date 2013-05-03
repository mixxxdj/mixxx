#include <QMutexLocker>
#include <QtDebug>

#include "mathstuff.h"
#include "track/keys.h"

using mixxx::track::io::key::ChromaticKey;
using mixxx::track::io::key::KeyMap;

Keys::Keys(const QByteArray* pByteArray) {
    if (pByteArray) {
        readByteArray(pByteArray);
    }
}

Keys::Keys(const KeyChangeList& key_changes,
           mixxx::track::io::key::Source source) {
    createFromKeyChanges(key_changes);
    m_keyMap.set_source(source);
}

Keys::Keys(const Keys& other)
        : m_subVersion(other.m_subVersion),
          m_keyMap(other.m_keyMap) {
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

void Keys::setSubVersion(QString subVersion) {
    QMutexLocker locker(&m_mutex);
    m_subVersion = subVersion;
}

bool Keys::isValid() const {
    return m_keyMap.has_global_key();
}

ChromaticKey Keys::getGlobalKey() const {
    return m_keyMap.global_key();
}

QString Keys::getGlobalKeyText() const {
    return QString::fromStdString(m_keyMap.global_key_text());
}

void Keys::createFromKeyChanges(const KeyChangeList& key_changes) {
    for (KeyChangeList::const_iterator it = key_changes.begin();
         it != key_changes.end(); ++it) {
        // Key position is in frames. Do not accept fractional frames.
        double frame = floorf(it->second);

        KeyMap::KeyChange* pChange = m_keyMap.add_key_change();
        pChange->set_key(it->first);
        pChange->set_frame_position(frame);
    }
}

void Keys::readByteArray(const QByteArray* pByteArray) {
    if (!m_keyMap.ParseFromArray(pByteArray->constData(), pByteArray->size())) {
        qDebug() << "ERROR: Could not parse Keys from QByteArray of size"
                 << pByteArray->size();
        return;
    }
}

