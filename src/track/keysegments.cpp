#include "track/keysegments.h"

#include <QMutexLocker>

#include "moc_keysegments.cpp"

void KeySegmentPointer::deleteLater(KeySegments* pSegment) {
    if (pSegment) {
        pSegment->deleteLater();
    }
}

KeySegmentPointer::KeySegmentPointer(KeySegments* pSegment)
        : std::shared_ptr<KeySegments>(pSegment, deleteLater) {
}

KeySegments::KeySegments(
        DbId id,
        double startTime,
        double duration,
        int keyId,
        const QString& keyText,
        double rangeStart,
        double rangeEnd,
        const QString& type,
        double confidence)
        : m_bDirty(false),
          m_dbId(id),
          m_startTime(startTime),
          m_duration(duration),
          m_keyId(keyId),
          m_keyText(keyText),
          m_rangeStart(rangeStart),
          m_rangeEnd(rangeEnd),
          m_type(type),
          m_confidence(confidence) {
    DEBUG_ASSERT(m_dbId.isValid());
}

KeySegments::KeySegments(
        double startTime,
        double duration,
        int keyId,
        const QString& keyText,
        double rangeStart,
        double rangeEnd,
        const QString& type,
        double confidence)
        : m_bDirty(true),
          m_startTime(startTime),
          m_duration(duration),
          m_keyId(keyId),
          m_keyText(keyText),
          m_rangeStart(rangeStart),
          m_rangeEnd(rangeEnd),
          m_type(type),
          m_confidence(confidence) {
}

DbId KeySegments::getId() const {
    QMutexLocker locker(&m_mutex);
    return m_dbId;
}

void KeySegments::setId(DbId id) {
    QMutexLocker locker(&m_mutex);
    m_dbId = id;
}

bool KeySegments::isDirty() const {
    QMutexLocker locker(&m_mutex);
    return m_bDirty;
}

void KeySegments::setDirty(bool dirty) {
    QMutexLocker locker(&m_mutex);
    m_bDirty = dirty;
}

double KeySegments::getStartTime() const {
    QMutexLocker locker(&m_mutex);
    return m_startTime;
}

void KeySegments::setStartTime(double startTime) {
    QMutexLocker locker(&m_mutex);
    if (m_startTime == startTime) {
        return;
    }
    m_startTime = startTime;
    m_bDirty = true;
    locker.unlock();
    emit updated();
}

double KeySegments::getDuration() const {
    QMutexLocker locker(&m_mutex);
    return m_duration;
}

void KeySegments::setDuration(double duration) {
    QMutexLocker locker(&m_mutex);
    if (m_duration == duration) {
        return;
    }
    m_duration = duration;
    m_bDirty = true;
    locker.unlock();
    emit updated();
}

int KeySegments::getKeyId() const {
    QMutexLocker locker(&m_mutex);
    return m_keyId;
}

void KeySegments::setKeyId(int keyId) {
    QMutexLocker locker(&m_mutex);
    if (m_keyId == keyId) {
        return;
    }
    m_keyId = keyId;
    m_bDirty = true;
    locker.unlock();
    emit updated();
}

QString KeySegments::getKeyText() const {
    QMutexLocker locker(&m_mutex);
    return m_keyText;
}

void KeySegments::setKeyText(const QString& keyText) {
    QMutexLocker locker(&m_mutex);
    if (m_keyText == keyText) {
        return;
    }
    m_keyText = keyText;
    m_bDirty = true;
    locker.unlock();
    emit updated();
}

double KeySegments::getRangeStart() const {
    QMutexLocker locker(&m_mutex);
    return m_rangeStart;
}

void KeySegments::setRangeStart(double rangeStart) {
    QMutexLocker locker(&m_mutex);
    if (m_rangeStart == rangeStart) {
        return;
    }
    m_rangeStart = rangeStart;
    m_bDirty = true;
    locker.unlock();
    emit updated();
}

double KeySegments::getRangeEnd() const {
    QMutexLocker locker(&m_mutex);
    return m_rangeEnd;
}

void KeySegments::setRangeEnd(double rangeEnd) {
    QMutexLocker locker(&m_mutex);
    if (m_rangeEnd == rangeEnd) {
        return;
    }
    m_rangeEnd = rangeEnd;
    m_bDirty = true;
    locker.unlock();
    emit updated();
}

QString KeySegments::getType() const {
    QMutexLocker locker(&m_mutex);
    return m_type;
}

void KeySegments::setType(const QString& type) {
    QMutexLocker locker(&m_mutex);
    if (m_type == type) {
        return;
    }
    m_type = type;
    m_bDirty = true;
    locker.unlock();
    emit updated();
}

double KeySegments::getConfidence() const {
    QMutexLocker locker(&m_mutex);
    return m_confidence;
}

void KeySegments::setConfidence(double confidence) {
    QMutexLocker locker(&m_mutex);
    if (m_confidence == confidence) {
        return;
    }
    m_confidence = confidence;
    m_bDirty = true;
    locker.unlock();
    emit updated();
}
