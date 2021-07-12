#include "track/cue.h"

#include <QMutexLocker>
#include <QtDebug>

#include "audio/frame.h"
#include "engine/engine.h"
#include "moc_cue.cpp"
#include "util/assert.h"
#include "util/color/color.h"
#include "util/color/predefinedcolorpalettes.h"

namespace {

inline std::optional<double> positionFramesToMillis(
        mixxx::audio::FramePos position,
        mixxx::audio::SampleRate sampleRate) {
    VERIFY_OR_DEBUG_ASSERT(sampleRate.isValid()) {
        return std::nullopt;
    }
    if (!position.isValid()) {
        return std::nullopt;
    }
    // Try to avoid rounding errors
    return (position.value() * 1000) / sampleRate;
}

inline mixxx::audio::FramePos positionMillisToFrames(
        std::optional<double> positionMillis,
        mixxx::audio::SampleRate sampleRate) {
    VERIFY_OR_DEBUG_ASSERT(sampleRate.isValid()) {
        return mixxx::audio::kInvalidFramePos;
    }

    if (!positionMillis) {
        return mixxx::audio::kInvalidFramePos;
    }

    return mixxx::audio::FramePos((*positionMillis * sampleRate) / 1000);
}
} // namespace

//static
void CuePointer::deleteLater(Cue* pCue) {
    if (pCue) {
        pCue->deleteLater();
    }
}

Cue::Cue(
        DbId id,
        mixxx::CueType type,
        mixxx::audio::FramePos position,
        mixxx::audio::FrameDiff_t length,
        int hotCue,
        const QString& label,
        mixxx::RgbColor color)
        : m_bDirty(false), // clear flag after loading from database
          m_dbId(id),
          m_type(type),
          m_startPosition(position),
          m_iHotCue(hotCue),
          m_label(label),
          m_color(color) {
    DEBUG_ASSERT(m_dbId.isValid());
    if (length != 0) {
        if (position.isValid()) {
            m_endPosition = position + length;
        } else {
            m_endPosition = mixxx::audio::kStartFramePos + length;
        }
    }
}

Cue::Cue(
        const mixxx::CueInfo& cueInfo,
        mixxx::audio::SampleRate sampleRate,
        bool setDirty)
        : m_bDirty(setDirty),
          m_type(cueInfo.getType()),
          m_startPosition(
                  positionMillisToFrames(
                          cueInfo.getStartPositionMillis(),
                          sampleRate)),
          m_endPosition(
                  positionMillisToFrames(
                          cueInfo.getEndPositionMillis(),
                          sampleRate)),
          m_iHotCue(cueInfo.getHotCueIndex() ? *cueInfo.getHotCueIndex() : kNoHotCue),
          m_label(cueInfo.getLabel()),
          m_color(cueInfo.getColor().value_or(mixxx::PredefinedColorPalettes::kDefaultCueColor)) {
    DEBUG_ASSERT(!m_dbId.isValid());
}

/// Initialize new cue points
Cue::Cue(
        mixxx::CueType type,
        int hotCueIndex,
        mixxx::audio::FramePos startPosition,
        mixxx::audio::FramePos endPosition)
        : m_bDirty(true), // not yet in database, needs to be saved
          m_type(type),
          m_startPosition(startPosition),
          m_endPosition(endPosition),
          m_iHotCue(hotCueIndex),
          m_color(mixxx::PredefinedColorPalettes::kDefaultCueColor) {
    DEBUG_ASSERT(!m_dbId.isValid());
}

mixxx::CueInfo Cue::getCueInfo(
        mixxx::audio::SampleRate sampleRate) const {
    QMutexLocker lock(&m_mutex);
    return mixxx::CueInfo(
            m_type,
            positionFramesToMillis(m_startPosition, sampleRate),
            positionFramesToMillis(m_endPosition, sampleRate),
            m_iHotCue == kNoHotCue ? std::nullopt : std::make_optional(m_iHotCue),
            m_label,
            m_color);
}

DbId Cue::getId() const {
    QMutexLocker lock(&m_mutex);
    return m_dbId;
}

void Cue::setId(DbId cueId) {
    QMutexLocker lock(&m_mutex);
    m_dbId = cueId;
    // Neither mark as dirty nor do emit the updated() signal.
    // This function is only called after adding the Cue object
    // to the database. The id is not visible for anyone else.
    // Unintended side effects with the LibraryScanner occur
    // when adding new tracks that have their cue points stored
    // in Serato marker tags!!
}

mixxx::CueType Cue::getType() const {
    QMutexLocker lock(&m_mutex);
    return m_type;
}

void Cue::setType(mixxx::CueType type) {
    QMutexLocker lock(&m_mutex);
    if (m_type == type) {
        return;
    }
    m_type = type;
    m_bDirty = true;
    lock.unlock();
    emit updated();
}

mixxx::audio::FramePos Cue::getPosition() const {
    QMutexLocker lock(&m_mutex);
    return m_startPosition;
}

void Cue::setStartPosition(mixxx::audio::FramePos position) {
    QMutexLocker lock(&m_mutex);
    if (m_startPosition == position) {
        return;
    }
    m_startPosition = position;
    m_bDirty = true;
    lock.unlock();
    emit updated();
}

void Cue::setEndPosition(mixxx::audio::FramePos position) {
    QMutexLocker lock(&m_mutex);
    if (m_endPosition == position) {
        return;
    }
    m_endPosition = position;
    m_bDirty = true;
    lock.unlock();
    emit updated();
}

void Cue::setStartAndEndPosition(
        mixxx::audio::FramePos startPosition,
        mixxx::audio::FramePos endPosition) {
    QMutexLocker lock(&m_mutex);
    if (m_startPosition == startPosition &&
            m_endPosition == endPosition) {
        return;
    }
    m_startPosition = startPosition;
    m_endPosition = endPosition;
    m_bDirty = true;
    lock.unlock();
    emit updated();
}

Cue::StartAndEndPositions Cue::getStartAndEndPosition() const {
    QMutexLocker lock(&m_mutex);
    return {m_startPosition, m_endPosition};
}

void Cue::shiftPositionFrames(mixxx::audio::FrameDiff_t frameOffset) {
    QMutexLocker lock(&m_mutex);
    if (m_startPosition.isValid()) {
        m_startPosition += frameOffset;
    }
    if (m_endPosition.isValid()) {
        m_endPosition += frameOffset;
    }
    m_bDirty = true;
    lock.unlock();
    emit updated();
}

mixxx::audio::FrameDiff_t Cue::getLengthFrames() const {
    QMutexLocker lock(&m_mutex);
    if (!m_endPosition.isValid()) {
        return 0;
    }
    if (!m_startPosition.isValid()) {
        return m_endPosition.value();
    }
    return m_endPosition - m_startPosition;
}

int Cue::getHotCue() const {
    QMutexLocker lock(&m_mutex);
    return m_iHotCue;
}

QString Cue::getLabel() const {
    QMutexLocker lock(&m_mutex);
    return m_label;
}

void Cue::setLabel(const QString& label) {
    QMutexLocker lock(&m_mutex);
    if (m_label == label) {
        return;
    }
    m_label = label;
    m_bDirty = true;
    lock.unlock();
    emit updated();
}

mixxx::RgbColor Cue::getColor() const {
    QMutexLocker lock(&m_mutex);
    return m_color;
}

void Cue::setColor(mixxx::RgbColor color) {
    QMutexLocker lock(&m_mutex);
    if (m_color == color) {
        return;
    }
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

mixxx::audio::FramePos Cue::getEndPosition() const {
    QMutexLocker lock(&m_mutex);
    return m_endPosition;
}

bool operator==(const CuePosition& lhs, const CuePosition& rhs) {
    return lhs.getPosition() == rhs.getPosition();
}
