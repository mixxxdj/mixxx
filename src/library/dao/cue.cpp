// cue.cpp
// Created 10/26/2009 by RJ Ryan (rryan@mit.edu)

#include <QMutexLocker>
#include <QtDebug>

#include "library/dao/cue.h"
#include "util/assert.h"

namespace {
    const QColor kDefaultColor = QColor("#FF0000");
    const QString kDefaultLabel = ""; // empty string, not null
}

Cue::~Cue() {
    //qDebug() << "~Cue()" << m_iId;
}

Cue::Cue(TrackId trackId)
        : m_bDirty(false),
          m_iId(-1),
          m_trackId(trackId),
          m_type(INVALID),
          m_samplePosition(-1.0),
          m_length(0.0),
          m_iHotCue(-1),
          m_label(kDefaultLabel),
          m_color(kDefaultColor) {
    DEBUG_ASSERT(!m_label.isNull());
}


Cue::Cue(int id, TrackId trackId, Cue::CueType type, double position, double length,
         int hotCue, QString label, QColor color)
        : m_bDirty(false),
          m_iId(id),
          m_trackId(trackId),
          m_type(type),
          m_samplePosition(position),
          m_length(length),
          m_iHotCue(hotCue),
          m_label(label),
          m_color(color) {
    DEBUG_ASSERT(!m_label.isNull());
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

double Cue::getPosition() const {
    QMutexLocker lock(&m_mutex);
    return m_samplePosition;
}

void Cue::setPosition(double samplePosition) {
    QMutexLocker lock(&m_mutex);
    m_samplePosition = samplePosition;
    m_bDirty = true;
    lock.unlock();
    emit(updated());
}

double Cue::getLength() const {
    QMutexLocker lock(&m_mutex);
    return m_length;
}

void Cue::setLength(double length) {
    QMutexLocker lock(&m_mutex);
    m_length = length;
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
    DEBUG_ASSERT(!label.isNull());
    QMutexLocker lock(&m_mutex);
    m_label = label;
    m_bDirty = true;
    lock.unlock();
    emit(updated());
}

QColor Cue::getColor() const {
    QMutexLocker lock(&m_mutex);
    return m_color;
}

void Cue::setColor(const QColor color) {
    QMutexLocker lock(&m_mutex);
    m_color = color;
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
