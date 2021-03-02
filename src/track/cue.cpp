#include "track/cue.h"

#include <QMutexLocker>
#include <QtDebug>

#include "engine/engine.h"
#include "moc_cue.cpp"
#include "util/assert.h"
#include "util/color/color.h"
#include "util/color/predefinedcolorpalettes.h"

namespace {

inline std::optional<double> positionSamplesToMillis(
        double positionSamples,
        mixxx::audio::SampleRate sampleRate) {
    VERIFY_OR_DEBUG_ASSERT(sampleRate.isValid()) {
        return std::nullopt;
    }
    if (positionSamples == Cue::kNoPosition) {
        return std::nullopt;
    }
    // Try to avoid rounding errors
    return (positionSamples * 1000) / (sampleRate * mixxx::kEngineChannelCount);
}

inline double positionMillisToSamples(
        std::optional<double> positionMillis,
        mixxx::audio::SampleRate sampleRate) {
    VERIFY_OR_DEBUG_ASSERT(sampleRate.isValid()) {
        return Cue::kNoPosition;
    }
    if (!positionMillis) {
        return Cue::kNoPosition;
    }
    // Try to avoid rounding errors
    return (*positionMillis * sampleRate * mixxx::kEngineChannelCount) / 1000;
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
        double position,
        double length,
        int hotCue,
        const QString& label,
        mixxx::RgbColor color)
        : m_bDirty(false), // clear flag after loading from database
          m_dbId(id),
          m_type(type),
          m_sampleStartPosition(position),
          m_iHotCue(hotCue),
          m_label(label),
          m_color(color) {
    DEBUG_ASSERT(m_dbId.isValid());
    if (length != 0) {
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
        const mixxx::CueInfo& cueInfo,
        mixxx::audio::SampleRate sampleRate,
        bool setDirty)
        : m_bDirty(setDirty),
          m_type(cueInfo.getType()),
          m_sampleStartPosition(
                  positionMillisToSamples(
                          cueInfo.getStartPositionMillis(),
                          sampleRate)),
          m_sampleEndPosition(
                  positionMillisToSamples(
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
        double sampleStartPosition,
        double sampleEndPosition)
        : m_bDirty(true), // not yet in database, needs to be saved
          m_type(type),
          m_sampleStartPosition(sampleStartPosition),
          m_sampleEndPosition(sampleEndPosition),
          m_iHotCue(hotCueIndex),
          m_color(mixxx::PredefinedColorPalettes::kDefaultCueColor) {
    DEBUG_ASSERT(!m_dbId.isValid());
}

mixxx::CueInfo Cue::getCueInfo(
        mixxx::audio::SampleRate sampleRate) const {
    QMutexLocker lock(&m_mutex);
    return mixxx::CueInfo(
            m_type,
            positionSamplesToMillis(m_sampleStartPosition, sampleRate),
            positionSamplesToMillis(m_sampleEndPosition, sampleRate),
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

double Cue::getPosition() const {
    QMutexLocker lock(&m_mutex);
    return m_sampleStartPosition;
}

void Cue::setStartPosition(double samplePosition) {
    QMutexLocker lock(&m_mutex);
    if (m_sampleStartPosition == samplePosition) {
        return;
    }
    m_sampleStartPosition = samplePosition;
    m_bDirty = true;
    lock.unlock();
    emit updated();
}

void Cue::setEndPosition(double samplePosition) {
    QMutexLocker lock(&m_mutex);
    if (m_sampleEndPosition == samplePosition) {
        return;
    }
    m_sampleEndPosition = samplePosition;
    m_bDirty = true;
    lock.unlock();
    emit updated();
}

void Cue::setStartAndEndPosition(
        double sampleStartPosition,
        double sampleEndPosition) {
    QMutexLocker lock(&m_mutex);
    if (m_sampleStartPosition == sampleStartPosition &&
            m_sampleEndPosition == sampleEndPosition) {
        return;
    }
    m_sampleStartPosition = sampleStartPosition;
    m_sampleEndPosition = sampleEndPosition;
    m_bDirty = true;
    lock.unlock();
    emit updated();
}

Cue::StartAndEndPositions Cue::getStartAndEndPosition() const {
    QMutexLocker lock(&m_mutex);
    return {m_sampleStartPosition, m_sampleEndPosition};
}

void Cue::shiftPositionFrames(double frameOffset) {
    QMutexLocker lock(&m_mutex);
    if (m_sampleStartPosition != kNoPosition) {
        m_sampleStartPosition += frameOffset * mixxx::kEngineChannelCount;
    }
    if (m_sampleEndPosition != kNoPosition) {
        m_sampleEndPosition += frameOffset * mixxx::kEngineChannelCount;
    }
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

double Cue::getEndPosition() const {
    QMutexLocker lock(&m_mutex);
    return m_sampleEndPosition;
}

bool operator==(const CuePosition& lhs, const CuePosition& rhs) {
    return lhs.getPosition() == rhs.getPosition();
}
