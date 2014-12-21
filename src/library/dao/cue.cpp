// cue.cpp
// Created 10/26/2009 by RJ Ryan (rryan@mit.edu)

#include <QMutexLocker>
#include <QtDebug>

#include "library/dao/cue.h"
#include "util/assert.h"

Cue::~Cue() {
    qDebug() << "~Cue()" << m_iId;
}

Cue::Cue(int trackId)
        : m_bDirty(false),
          m_iId(-1),
          m_iTrackId(trackId),
          m_type(INVALID),
          m_iPosition(-1),
          m_iLength(0),
          m_iHotCue(-1),
          m_label("") {
    //qDebug() << "Cue(int)";
}


Cue::Cue(int id, int trackId, Cue::CueType type, int position, int length,
         int hotCue, QString label)
        : m_bDirty(false),
          m_iId(id),
          m_iTrackId(trackId),
          m_type(type),
          m_iPosition(position),
          m_iLength(length),
          m_iHotCue(hotCue),
          m_label(label) {
    //qDebug() << "Cue(...)";
}

int Cue::getId() {
    QMutexLocker lock(&m_mutex);
    int id = m_iId;
    return id;
}

void Cue::setId(int cueId) {
    QMutexLocker lock(&m_mutex);
    m_iId = cueId;
    m_bDirty = true;
    lock.unlock();
    emit(updated());
}

int Cue::getTrackId() {
    QMutexLocker lock(&m_mutex);
    int trackId = m_iTrackId;
    return trackId;
}

void Cue::setTrackId(int trackId) {
    QMutexLocker lock(&m_mutex);
    m_iTrackId = trackId;
    m_bDirty = true;
    lock.unlock();
    emit(updated());
}

Cue::CueType Cue::getType() {
    QMutexLocker lock(&m_mutex);
    Cue::CueType type = m_type;
    return type;
}

void Cue::setType(Cue::CueType type) {
    QMutexLocker lock(&m_mutex);
    m_type = type;
    m_bDirty = true;
    lock.unlock();
    emit(updated());
}

int Cue::getPosition() {
    QMutexLocker lock(&m_mutex);
    int position = m_iPosition;
    return position;
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

int Cue::getLength() {
    QMutexLocker lock(&m_mutex);
    int length = m_iLength;
    return length;
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

int Cue::getHotCue() {
    QMutexLocker lock(&m_mutex);
    int hotCue = m_iHotCue;
    return hotCue;
}

void Cue::setHotCue(int hotCue) {
    QMutexLocker lock(&m_mutex);
    // TODO(XXX) enforce uniqueness?
    m_iHotCue = hotCue;
    m_bDirty = true;
    lock.unlock();
    emit(updated());
}

const QString Cue::getLabel() {
    QMutexLocker lock(&m_mutex);
    QString label = m_label;
    lock.unlock();
    return label;
}

void Cue::setLabel(const QString label) {
    //qDebug() << "setLabel()" << m_label << "-" << label;
    QMutexLocker lock(&m_mutex);
    m_label = label;
    m_bDirty = true;
    lock.unlock();
    emit(updated());
}

bool Cue::isDirty() {
    QMutexLocker lock(&m_mutex);
    bool dirty = m_bDirty;
    return dirty;
}

void Cue::setDirty(bool dirty) {
    QMutexLocker lock(&m_mutex);
    m_bDirty = dirty;
}
