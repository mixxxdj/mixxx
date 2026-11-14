#include "track/bpmsegments.h"

#include <QMutexLocker>

#include "moc_bpmsegments.cpp"

// static
void BpmSegmentPointer::deleteLater(BpmSegments* pSegment) {
    if (pSegment) {
        pSegment->deleteLater();
    }
}

BpmSegmentPointer::BpmSegmentPointer(BpmSegments* pSegment)
        : std::shared_ptr<BpmSegments>(pSegment, deleteLater) {
}

BpmSegments::BpmSegments(
        DbId id,
        double startTime,
        double duration,
        double bpmStart,
        double bpmEnd,
        double rangeStart,
        double rangeEnd,
        const QString& type)
        : m_bDirty(false),
          m_dbId(id),
          m_startTime(startTime),
          m_duration(duration),
          m_bpmStart(bpmStart),
          m_bpmEnd(bpmEnd),
          m_rangeStart(rangeStart),
          m_rangeEnd(rangeEnd),
          m_type(type) {
    DEBUG_ASSERT(m_dbId.isValid());
}

BpmSegments::BpmSegments(
        double startTime,
        double duration,
        double bpmStart,
        double bpmEnd,
        double rangeStart,
        double rangeEnd,
        const QString& type)
        : m_bDirty(true),
          m_startTime(startTime),
          m_duration(duration),
          m_bpmStart(bpmStart),
          m_bpmEnd(bpmEnd),
          m_rangeStart(rangeStart),
          m_rangeEnd(rangeEnd),
          m_type(type) {
}

DbId BpmSegments::getId() const {
    QMutexLocker locker(&m_mutex);
    return m_dbId;
}

void BpmSegments::setId(DbId id) {
    QMutexLocker locker(&m_mutex);
    m_dbId = id;
}

bool BpmSegments::isDirty() const {
    QMutexLocker locker(&m_mutex);
    return m_bDirty;
}

void BpmSegments::setDirty(bool dirty) {
    QMutexLocker locker(&m_mutex);
    m_bDirty = dirty;
}

double BpmSegments::getStartTime() const {
    QMutexLocker locker(&m_mutex);
    return m_startTime;
}

void BpmSegments::setStartTime(double startTime) {
    QMutexLocker locker(&m_mutex);
    if (m_startTime == startTime) {
        return;
    }
    m_startTime = startTime;
    m_bDirty = true;
    emit updated();
}

double BpmSegments::getDuration() const {
    QMutexLocker locker(&m_mutex);
    return m_duration;
}

void BpmSegments::setDuration(double duration) {
    QMutexLocker locker(&m_mutex);
    if (m_duration == duration) {
        return;
    }
    m_duration = duration;
    m_bDirty = true;
    emit updated();
}

double BpmSegments::getBpmStart() const {
    QMutexLocker locker(&m_mutex);
    return m_bpmStart;
}

void BpmSegments::setBpmStart(double bpmStart) {
    QMutexLocker locker(&m_mutex);
    if (m_bpmStart == bpmStart) {
        return;
    }
    m_bpmStart = bpmStart;
    m_bDirty = true;
    emit updated();
}

double BpmSegments::getBpmEnd() const {
    QMutexLocker locker(&m_mutex);
    return m_bpmEnd;
}

void BpmSegments::setBpmEnd(double bpmEnd) {
    QMutexLocker locker(&m_mutex);
    if (m_bpmEnd == bpmEnd) {
        return;
    }
    m_bpmEnd = bpmEnd;
    m_bDirty = true;
    emit updated();
}

double BpmSegments::getRangeStart() const {
    QMutexLocker locker(&m_mutex);
    return m_rangeStart;
}

void BpmSegments::setRangeStart(double rangeStart) {
    QMutexLocker locker(&m_mutex);
    if (m_rangeStart == rangeStart) {
        return;
    }
    m_rangeStart = rangeStart;
    m_bDirty = true;
    emit updated();
}

double BpmSegments::getRangeEnd() const {
    QMutexLocker locker(&m_mutex);
    return m_rangeEnd;
}

void BpmSegments::setRangeEnd(double rangeEnd) {
    QMutexLocker locker(&m_mutex);
    if (m_rangeEnd == rangeEnd) {
        return;
    }
    m_rangeEnd = rangeEnd;
    m_bDirty = true;
    emit updated();
}

QString BpmSegments::getType() const {
    QMutexLocker locker(&m_mutex);
    return m_type;
}

void BpmSegments::setType(const QString& type) {
    QMutexLocker locker(&m_mutex);
    if (m_type == type) {
        return;
    }
    m_type = type;
    m_bDirty = true;
    emit updated();
}
