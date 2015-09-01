// cue.cpp
// Created 10/26/2009 by RJ Ryan (rryan@mit.edu)

#include <QMutexLocker>
#include <QtDebug>

#include "library/dao/cue.h"
#include "util/assert.h"

Cue::~Cue() {
    qDebug() << "~Cue()" << m_iId;
}

Cue::Cue(TrackId trackId)
        : m_bDirty(false),
          m_iId(-1),
          m_trackId(trackId),
          m_type(INVALID),
          m_iPosition(-1),
          m_iLength(0),
          m_iHotCue(-1),
          m_label("") {
}


Cue::Cue(int id, TrackId trackId, Cue::CueType type, int position, int length,
         int hotCue, QString label)
        : m_bDirty(false),
          m_iId(id),
          m_trackId(trackId),
          m_type(type),
          m_iPosition(position),
          m_iLength(length),
          m_iHotCue(hotCue),
          m_label(label) {
}

int Cue::getId() const {
    QMutexLocker lock(&m_mutex);
    return m_iId;
}

void Cue::setId(int cueId) {
    QMutexLocker lock(&m_mutex);
    m_iId = cueId;
    m_bDirty = true;
    lock.unlock();
    emit(updated());
}

TrackId Cue::getTrackId() const {
    QMutexLocker lock(&m_mutex);
    return m_trackId;
}

void Cue::setTrackId(TrackId trackId) {
    QMutexLocker lock(&m_mutex);
    m_trackId = trackId;
    m_bDirty = true;
    lock.unlock();
    emit(updated());
}

Cue::CueType Cue::getType() const {
    QMutexLocker lock(&m_mutex);
    return m_type;
}

void Cue::setType(Cue::CueType type) {
    QMutexLocker lock(&m_mutex);
    m_type = type;
    m_bDirty = true;
    lock.unlock();
    emit(updated());
}

int Cue::getPosition() const {
    QMutexLocker lock(&m_mutex);
    return m_iPosition;
}

void Cue::setPosition(int position) {
    DEBUG_ASSERT_AND_HANDLE(position % 2 == 0) {
        return;
    }
    QMutexLocker lock(&m_mutex);
    m_iPosition = position;
    m_bDirty = true;
    lock.unlock();
    emit(updated());
}

int Cue::getLength() const {
    QMutexLocker lock(&m_mutex);
    return m_iLength;
}

void Cue::setLength(int length) {
    DEBUG_ASSERT_AND_HANDLE(length % 2 == 0) {
        return;
    }
    QMutexLocker lock(&m_mutex);
    m_iLength = length;
    m_bDirty = true;
    lock.unlock();
    emit(updated());
}

int Cue::getHotCue() const {
    QMutexLocker lock(&m_mutex);
    return m_iHotCue;
}

void Cue::setHotCue(int hotCue) {
    QMutexLocker lock(&m_mutex);
    // TODO(XXX) enforce uniqueness?
    m_iHotCue = hotCue;
    m_bDirty = true;
    lock.unlock();
    emit(updated());
}

QString Cue::getLabel() const {
    QMutexLocker lock(&m_mutex);
    return m_label;
}

void Cue::setLabel(const QString label) {
    //qDebug() << "setLabel()" << m_label << "-" << label;
    QMutexLocker lock(&m_mutex);
    m_label = label;
    m_bDirty = true;
    lock.unlock();
    emit(updated());
}

bool Cue::isDirty() const {
    QMutexLocker lock(&m_mutex);
    return m_bDirty;
}

void Cue::setDirty(bool dirty) {
    QMutexLocker lock(&m_mutex);
    m_bDirty = dirty;
}
