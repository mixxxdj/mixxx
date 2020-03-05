// cue.cpp
// Created 10/26/2009 by RJ Ryan (rryan@mit.edu)

#include "track/cue.h"

#include <QMutexLocker>
#include <QtDebug>

#include "engine/engine.h"
#include "util/assert.h"
#include "util/color/color.h"

namespace {

inline std::optional<double> samplePositionToMillis(
        double samplePosition,
        mixxx::AudioSignal::SampleRate sampleRate) {
    DEBUG_ASSERT(sampleRate.valid());
    if (samplePosition == Cue::kNoPosition) {
        return std::nullopt;
    }
    const auto sampleRateKhz = sampleRate / 1000.0;
    DEBUG_ASSERT(sampleRateKhz > 0.0);
    return samplePosition / (sampleRateKhz * mixxx::kEngineChannelCount);
}

}

//static
void CuePointer::deleteLater(Cue* pCue) {
    if (pCue) {
        pCue->deleteLater();
    }
}

Cue::Cue(
        TrackId trackId)
        : m_bDirty(false),
          m_iId(-1),
          m_trackId(trackId),
          m_type(mixxx::CueType::Invalid),
          m_sampleStartPosition(Cue::kNoPosition),
          m_sampleEndPosition(Cue::kNoPosition),
          m_iHotCue(Cue::kNoHotCue),
          m_color(Color::kPredefinedColorsSet.noColor) {
    DEBUG_ASSERT(!m_label.isNull());
}

Cue::Cue(
        int id,
        TrackId trackId,
        mixxx::CueType type,
        double position,
        double length,
        int hotCue,
        QString label,
        PredefinedColorPointer color)
        : m_bDirty(false),
          m_iId(id),
          m_trackId(trackId),
          m_type(type),
          m_sampleStartPosition(position),
          m_iHotCue(hotCue),
          m_label(label),
          m_color(color) {
    DEBUG_ASSERT(!m_label.isNull());
    if (length) {
        if (position != Cue::kNoPosition) {
            m_sampleEndPosition = position + length;
        } else {
            m_sampleEndPosition = length;
        }
    } else {
        m_sampleEndPosition = Cue::kNoPosition;
    }
}

Cue::Cue(
        TrackId trackId,
        mixxx::AudioSignal::SampleRate sampleRate,
        const mixxx::CueInfo& cueInfo)
        : m_bDirty(false),
          m_iId(-1),
          m_trackId(trackId),
          m_type(cueInfo.getType()),
          m_sampleStartPosition(Cue::kNoPosition),
          m_sampleEndPosition(Cue::kNoPosition),
          m_iHotCue(Cue::kNoHotCue),
          m_label(cueInfo.getLabel()),
          m_color(Color::kPredefinedColorsSet.predefinedColorFromRgbColor(cueInfo.getColor())) {
    DEBUG_ASSERT(!m_label.isNull());
    DEBUG_ASSERT(sampleRate.valid());

    const double sampleRateKhz = sampleRate / 1000.0;
    const double millisecsToSamplesFactor = sampleRateKhz * mixxx::kEngineChannelCount;
    DEBUG_ASSERT(millisecsToSamplesFactor > 0);

    if (cueInfo.getStartPositionMillis()) {
        m_sampleStartPosition = (*cueInfo.getStartPositionMillis()) * millisecsToSamplesFactor;
    }

    if (cueInfo.getEndPositionMillis()) {
        m_sampleEndPosition = (*cueInfo.getEndPositionMillis()) * millisecsToSamplesFactor;
    }

    if (cueInfo.getHotCueNumber()) {
        m_iHotCue = *cueInfo.getHotCueNumber();
    }
}

mixxx::CueInfo Cue::getCueInfo(
        mixxx::AudioSignal::SampleRate sampleRate) const {
    QMutexLocker lock(&m_mutex);
    return mixxx::CueInfo(
            m_type,
            samplePositionToMillis(m_sampleStartPosition, sampleRate),
            samplePositionToMillis(m_sampleEndPosition, sampleRate),
            m_iHotCue == kNoHotCue ? std::nullopt : std::make_optional(m_iHotCue),
            m_label,
            m_color ? mixxx::RgbColor::fromQColor(m_color->m_defaultRgba) : std::nullopt);
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
    emit updated();
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
    emit updated();
}

mixxx::CueType Cue::getType() const {
    QMutexLocker lock(&m_mutex);
    return m_type;
}

void Cue::setType(mixxx::CueType type) {
    QMutexLocker lock(&m_mutex);
    m_type = type;
    m_bDirty = true;
    lock.unlock();
    emit updated();
}

double Cue::getPosition() const {
    QMutexLocker lock(&m_mutex);
    return m_sampleStartPosition;
}

void Cue::setStartPosition(double samplePosition) {
    QMutexLocker lock(&m_mutex);
    m_sampleStartPosition = samplePosition;
    m_bDirty = true;
    lock.unlock();
    emit updated();
}

void Cue::setEndPosition(double samplePosition) {
    QMutexLocker lock(&m_mutex);
    m_sampleEndPosition = samplePosition;
    m_bDirty = true;
    lock.unlock();
    emit updated();
}

double Cue::getLength() const {
    QMutexLocker lock(&m_mutex);
    if (m_sampleEndPosition == Cue::kNoPosition) {
        return 0;
    }
    if (m_sampleStartPosition == Cue::kNoPosition) {
        return m_sampleEndPosition;
    }
    return m_sampleEndPosition - m_sampleStartPosition;
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
    emit updated();
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
    emit updated();
}

PredefinedColorPointer Cue::getColor() const {
    QMutexLocker lock(&m_mutex);
    return m_color;
}

void Cue::setColor(const PredefinedColorPointer color) {
    QMutexLocker lock(&m_mutex);
    m_color = color;
    m_bDirty = true;
    lock.unlock();
    emit updated();
}

bool Cue::isDirty() const {
    QMutexLocker lock(&m_mutex);
    return m_bDirty;
}

void Cue::setDirty(bool dirty) {
    QMutexLocker lock(&m_mutex);
    m_bDirty = dirty;
}

double Cue::getEndPosition() const {
    QMutexLocker lock(&m_mutex);
    return m_sampleEndPosition;
}

bool operator==(const CuePosition& lhs, const CuePosition& rhs) {
    return lhs.getPosition() == rhs.getPosition();
}
