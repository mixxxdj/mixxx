#include "track/cue.h"

#include "audio/frame.h"
#include "moc_cue.cpp"
#include "util/assert.h"
#include "util/color/predefinedcolorpalettes.h"
#include "util/compatibility/qmutex.h"

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

// static
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
        mixxx::RgbColor color,
        double stem1vol,
        double stem2vol,
        double stem3vol,
        double stem4vol)
        : m_bDirty(false), // clear flag after loading from database
          m_dbId(id),
          m_type(type),
          m_startPosition(position),
          m_iHotCue(hotCue),
          m_label(label),
          m_color(color),
          m_stem1vol(stem1vol),
          m_stem2vol(stem2vol),
          m_stem3vol(stem3vol),
          m_stem4vol(stem4vol) {
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
          m_iHotCue(cueInfo.getHotCueIndex().value_or(kNoHotCue)),
          m_label(cueInfo.getLabel()),
          m_color(cueInfo.getColor().value_or(mixxx::PredefinedColorPalettes::kDefaultCueColor)),
          m_stem1vol(cueInfo.getStem1vol().value_or(kNoHotCue)),
          m_stem2vol(cueInfo.getStem2vol().value_or(kNoHotCue)),
          m_stem3vol(cueInfo.getStem3vol().value_or(kNoHotCue)),
          m_stem4vol(cueInfo.getStem4vol().value_or(kNoHotCue)) {
    DEBUG_ASSERT(!m_dbId.isValid());
}

/// Initialize new cue points
Cue::Cue(
        mixxx::CueType type,
        int hotCueIndex,
        mixxx::audio::FramePos startPosition,
        mixxx::audio::FramePos endPosition,
        mixxx::RgbColor color,
        double stem1vol,
        double stem2vol,
        double stem3vol,
        double stem4vol)
        : m_bDirty(true), // not yet in database, needs to be saved
          m_type(type),
          m_startPosition(startPosition),
          m_endPosition(endPosition),
          m_iHotCue(hotCueIndex),
          m_color(color),
          m_stem1vol(stem1vol),
          m_stem2vol(stem2vol),
          m_stem3vol(stem3vol),
          m_stem4vol(stem4vol) {
    DEBUG_ASSERT(m_iHotCue == kNoHotCue || m_iHotCue >= mixxx::kFirstHotCueIndex);
    DEBUG_ASSERT(m_startPosition.isValid() || m_endPosition.isValid());
    DEBUG_ASSERT(!m_dbId.isValid());
}

mixxx::CueInfo Cue::getCueInfo(
        mixxx::audio::SampleRate sampleRate) const {
    const auto lock = lockMutex(&m_mutex);
    return mixxx::CueInfo(
            m_type,
            positionFramesToMillis(m_startPosition, sampleRate),
            positionFramesToMillis(m_endPosition, sampleRate),
            m_iHotCue == kNoHotCue ? std::nullopt : std::make_optional(m_iHotCue),
            m_label,
            m_color,
            m_stem1vol == kNoHotCue ? std::nullopt : std::make_optional(m_stem1vol),
            m_stem2vol == kNoHotCue ? std::nullopt : std::make_optional(m_stem2vol),
            m_stem3vol == kNoHotCue ? std::nullopt : std::make_optional(m_stem3vol),
            m_stem4vol == kNoHotCue ? std::nullopt : std::make_optional(m_stem4vol));
}

DbId Cue::getId() const {
    const auto lock = lockMutex(&m_mutex);
    return m_dbId;
}

void Cue::setId(DbId cueId) {
    const auto lock = lockMutex(&m_mutex);
    m_dbId = cueId;
    // Neither mark as dirty nor do emit the updated() signal.
    // This function is only called after adding the Cue object
    // to the database. The id is not visible for anyone else.
    // Unintended side effects with the LibraryScanner occur
    // when adding new tracks that have their cue points stored
    // in Serato marker tags!!
}

mixxx::CueType Cue::getType() const {
    const auto lock = lockMutex(&m_mutex);
    return m_type;
}

void Cue::setType(mixxx::CueType type) {
    auto lock = lockMutex(&m_mutex);
    if (m_type == type) {
        return;
    }
    m_type = type;
    m_bDirty = true;
    lock.unlock();
    emit updated();
}

mixxx::audio::FramePos Cue::getPosition() const {
    const auto lock = lockMutex(&m_mutex);
    return m_startPosition;
}

void Cue::setStartPosition(mixxx::audio::FramePos position) {
    auto lock = lockMutex(&m_mutex);
    if (m_startPosition == position) {
        return;
    }
    m_startPosition = position;
    m_bDirty = true;
    lock.unlock();
    emit updated();
}

void Cue::setEndPosition(mixxx::audio::FramePos position) {
    auto lock = lockMutex(&m_mutex);
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
    auto lock = lockMutex(&m_mutex);
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
    const auto lock = lockMutex(&m_mutex);
    return {m_startPosition, m_endPosition};
}

void Cue::shiftPositionFrames(mixxx::audio::FrameDiff_t frameOffset) {
    auto lock = lockMutex(&m_mutex);
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
    const auto lock = lockMutex(&m_mutex);
    if (!m_endPosition.isValid()) {
        return 0;
    }
    if (!m_startPosition.isValid()) {
        return m_endPosition.value();
    }
    return m_endPosition - m_startPosition;
}

void Cue::setHotCue(int n) {
    VERIFY_OR_DEBUG_ASSERT(n >= mixxx::kFirstHotCueIndex) {
        return;
    }
    const auto lock = lockMutex(&m_mutex);
    if (m_iHotCue == n) {
        return;
    }
    m_iHotCue = n;
}

int Cue::getHotCue() const {
    const auto lock = lockMutex(&m_mutex);
    return m_iHotCue;
}

double Cue::getStem1vol() const {
    const auto lock = lockMutex(&m_mutex);
    return m_stem1vol;
}

double Cue::getStem2vol() const {
    const auto lock = lockMutex(&m_mutex);
    return m_stem2vol;
}

double Cue::getStem3vol() const {
    const auto lock = lockMutex(&m_mutex);
    return m_stem3vol;
}

double Cue::getStem4vol() const {
    const auto lock = lockMutex(&m_mutex);
    return m_stem4vol;
}

void Cue::setStem1vol(double stem1vol) {
    auto lock = lockMutex(&m_mutex);
    if (m_stem1vol == stem1vol) {
        return;
    }
    //    if (stem1vol < -100 || stem1vol > 100) {
    //        return;
    //    }
    m_stem1vol = stem1vol;
    m_bDirty = true;
    lock.unlock();
    emit updated();
}

void Cue::setStem2vol(double stem2vol) {
    auto lock = lockMutex(&m_mutex);
    if (m_stem2vol == stem2vol) {
        return;
    }
    //    if (stem2vol < -100 || stem2vol > 100) {
    //        return;
    //    }
    m_stem2vol = stem2vol;
    m_bDirty = true;
    lock.unlock();
    emit updated();
}

void Cue::setStem3vol(double stem3vol) {
    auto lock = lockMutex(&m_mutex);
    if (m_stem3vol == stem3vol) {
        return;
    }
    //    if (stem3vol < -100 || stem1vo3 > 100) {
    //        return;
    //    }
    m_stem3vol = stem3vol;
    m_bDirty = true;
    lock.unlock();
    emit updated();
}

void Cue::setStem4vol(double stem4vol) {
    auto lock = lockMutex(&m_mutex);
    if (m_stem4vol == stem4vol) {
        return;
    }
    //    if (stem4vol < -100 || stem4vol > 100) {
    //        return;
    //    }
    m_stem4vol = stem4vol;
    m_bDirty = true;
    lock.unlock();
    emit updated();
}

QString Cue::getLabel() const {
    const auto lock = lockMutex(&m_mutex);
    return m_label;
}

void Cue::setLabel(const QString& label) {
    auto lock = lockMutex(&m_mutex);
    if (m_label == label) {
        return;
    }
    m_label = label;
    m_bDirty = true;
    lock.unlock();
    emit updated();
}

mixxx::RgbColor Cue::getColor() const {
    const auto lock = lockMutex(&m_mutex);
    return m_color;
}

void Cue::setColor(mixxx::RgbColor color) {
    auto lock = lockMutex(&m_mutex);
    if (m_color == color) {
        return;
    }
    m_color = color;
    m_bDirty = true;
    lock.unlock();
    emit updated();
}

bool Cue::isDirty() const {
    const auto lock = lockMutex(&m_mutex);
    return m_bDirty;
}

void Cue::setDirty(bool dirty) {
    const auto lock = lockMutex(&m_mutex);
    m_bDirty = dirty;
}

mixxx::audio::FramePos Cue::getEndPosition() const {
    const auto lock = lockMutex(&m_mutex);
    return m_endPosition;
}
