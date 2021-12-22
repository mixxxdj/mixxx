/*
 * beatmap.cpp
 *
 *  Created on: 08/dic/2011
 *      Author: vittorio
 */

#include "track/beatmap.h"

#include <QtDebug>
#include <QtGlobal>
#include <algorithm>
#include <iterator>

#include "track/beatutils.h"
#include "track/track.h"
#include "util/math.h"

using mixxx::track::io::Beat;

namespace {

constexpr int kMinNumberOfBeats = 2; // a map needs at least two beats to have a tempo

inline Beat beatFromFramePos(mixxx::audio::FramePos beatPosition) {
    DEBUG_ASSERT(beatPosition.isValid());
    // Because the protobuf Beat object stores integers internally, all
    // fractional positions are lost.
    DEBUG_ASSERT(!beatPosition.isFractional());
    Beat beat;
    beat.set_frame_position(static_cast<google::protobuf::int32>(beatPosition.value()));
    return beat;
}

inline bool beatLessThan(const Beat& beat1, const Beat& beat2) {
    return beat1.frame_position() < beat2.frame_position();
}

void scaleDouble(BeatList* pBeats) {
    Beat prevBeat = pBeats->first();
    // Skip the first beat to preserve the first beat in a measure
    BeatList::iterator it = pBeats->begin() + 1;
    for (; it != pBeats->end(); ++it) {
        // Need to not accrue fractional frames.
        int distance = it->frame_position() - prevBeat.frame_position();
        Beat beat;
        beat.set_frame_position(prevBeat.frame_position() + distance / 2);
        it = pBeats->insert(it, beat);
        prevBeat = (++it)[0];
    }
}

void scaleTriple(BeatList* pBeats) {
    Beat prevBeat = pBeats->first();
    // Skip the first beat to preserve the first beat in a measure
    BeatList::iterator it = pBeats->begin() + 1;
    for (; it != pBeats->end(); ++it) {
        // Need to not accrue fractional frames.
        int distance = it->frame_position() - prevBeat.frame_position();
        Beat beat;
        beat.set_frame_position(prevBeat.frame_position() + distance / 3);
        it = pBeats->insert(it, beat);
        ++it;
        beat.set_frame_position(prevBeat.frame_position() + distance * 2 / 3);
        it = pBeats->insert(it, beat);
        prevBeat = (++it)[0];
    }
}

void scaleQuadruple(BeatList* pBeats) {
    Beat prevBeat = pBeats->first();
    // Skip the first beat to preserve the first beat in a measure
    BeatList::iterator it = pBeats->begin() + 1;
    for (; it != pBeats->end(); ++it) {
        // Need to not accrue fractional frames.
        int distance = it->frame_position() - prevBeat.frame_position();
        Beat beat;
        for (int i = 1; i <= 3; i++) {
            beat.set_frame_position(prevBeat.frame_position() + distance * i / 4);
            it = pBeats->insert(it, beat);
            ++it;
        }
        prevBeat = it[0];
    }
}

void scaleHalve(BeatList* pBeats) {
    // Skip the first beat to preserve the first beat in a measure
    BeatList::iterator it = pBeats->begin() + 1;
    for (; it != pBeats->end(); ++it) {
        it = pBeats->erase(it);
        if (it == pBeats->end()) {
            break;
        }
    }
}

void scaleThird(BeatList* pBeats) {
    // Skip the first beat to preserve the first beat in a measure
    BeatList::iterator it = pBeats->begin() + 1;
    for (; it != pBeats->end(); ++it) {
        it = pBeats->erase(it);
        if (it == pBeats->end()) {
            break;
        }
        it = pBeats->erase(it);
        if (it == pBeats->end()) {
            break;
        }
    }
}

void scaleFourth(BeatList* pBeats) {
    // Skip the first beat to preserve the first beat in a measure
    BeatList::iterator it = pBeats->begin() + 1;
    for (; it != pBeats->end(); ++it) {
        it = pBeats->erase(it);
        if (it == pBeats->end()) {
            break;
        }
        it = pBeats->erase(it);
        if (it == pBeats->end()) {
            break;
        }
        it = pBeats->erase(it);
        if (it == pBeats->end()) {
            break;
        }
    }
}

} // namespace

namespace mixxx {

class BeatMapIterator : public BeatIterator {
  public:
    BeatMapIterator(const BeatList::const_iterator& start, const BeatList::const_iterator& end)
            : m_currentBeat(start),
              m_endBeat(end) {
        // Advance to the first enabled beat.
        while (m_currentBeat != m_endBeat && !m_currentBeat->enabled()) {
            ++m_currentBeat;
        }
    }

    bool hasNext() const override {
        return m_currentBeat != m_endBeat;
    }

    audio::FramePos next() override {
        const auto beat = mixxx::audio::FramePos(m_currentBeat->frame_position());
        ++m_currentBeat;
        while (m_currentBeat != m_endBeat && !m_currentBeat->enabled()) {
            ++m_currentBeat;
        }
        return beat;
    }

  private:
    BeatList::const_iterator m_currentBeat;
    BeatList::const_iterator m_endBeat;
};

BeatMap::BeatMap(
        MakeSharedTag,
        audio::SampleRate sampleRate,
        const QString& subVersion,
        BeatList beats)
        : m_subVersion(subVersion),
          m_sampleRate(sampleRate),
          m_beats(std::move(beats)) {
}

BeatMap::BeatMap(
        MakeSharedTag,
        const BeatMap& other,
        BeatList beats)
        : BeatMap(
                  MakeSharedTag{},
                  other.m_sampleRate,
                  other.m_subVersion,
                  std::move(beats)) {
}

BeatMap::BeatMap(
        MakeSharedTag,
        const BeatMap& other)
        : BeatMap(MakeSharedTag{}, other, other.m_beats) {
}

// static
BeatsPointer BeatMap::fromByteArray(
        audio::SampleRate sampleRate,
        const QString& subVersion,
        const QByteArray& byteArray) {
    BeatList beatList;

    track::io::BeatMap map;
    if (map.ParseFromArray(byteArray.constData(), byteArray.size())) {
        for (int i = 0; i < map.beat_size(); ++i) {
            const Beat& beat = map.beat(i);
            beatList.append(beat);
        }
    } else {
        qDebug() << "ERROR: Could not parse BeatMap from QByteArray of size"
                 << byteArray.size();
    }
    return std::make_shared<BeatMap>(MakeSharedTag{}, sampleRate, subVersion, beatList);
}

// static
BeatsPointer BeatMap::makeBeatMap(
        audio::SampleRate sampleRate,
        const QString& subVersion,
        const QVector<mixxx::audio::FramePos>& beats) {
    BeatList beatList;

    mixxx::audio::FramePos previousBeatPos = mixxx::audio::kInvalidFramePos;

    for (const mixxx::audio::FramePos& originalBeatPos : beats) {
        VERIFY_OR_DEBUG_ASSERT(originalBeatPos.isValid()) {
            qWarning() << "BeatMap::makeBeatMap: Beats is invalid, discarding beat";
            continue;
        }

        // Do not accept fractional frames.
        const auto beatPos = originalBeatPos.toLowerFrameBoundary();
        if (previousBeatPos.isValid() && beatPos <= previousBeatPos) {
            qWarning() << "BeatMap::makeBeatMap: Beats not in increasing "
                          "order, discarding beat "
                       << beatPos;
            continue;
        }

        Beat beat = beatFromFramePos(beatPos);
        beatList.append(beat);
        previousBeatPos = beatPos;
    }
    return std::make_shared<BeatMap>(MakeSharedTag{}, sampleRate, subVersion, beatList);
}

QByteArray BeatMap::toByteArray() const {
    // No guarantees BeatLists are made of a data type which located adjacent
    // items in adjacent memory locations.
    mixxx::track::io::BeatMap map;

    for (int i = 0; i < m_beats.size(); ++i) {
        map.add_beat()->CopyFrom(m_beats[i]);
    }

    std::string output;
    map.SerializeToString(&output);
    return QByteArray(output.data(), static_cast<int>(output.length()));
}

QString BeatMap::getVersion() const {
    return BEAT_MAP_VERSION;
}

QString BeatMap::getSubVersion() const {
    return m_subVersion;
}

bool BeatMap::isValid() const {
    return m_sampleRate.isValid() && m_beats.size() >= kMinNumberOfBeats;
}

audio::FramePos BeatMap::findNthBeat(audio::FramePos position, int n) const {
    if (!isValid() || n == 0) {
        return audio::kInvalidFramePos;
    }

    const bool searchForward = n > 0;
    int numBeatsLeft = std::abs(n);

    // Beats are stored as full frame positions, so when searching forwards the
    // smallest possible beat position we can find is the upper frame boundary
    // of the search position.
    //
    // For searching backwards, the same applies for the lower frame boundary.
    const auto searchFromPosition = searchForward
            ? position.toUpperFrameBoundary()
            : position.toLowerFrameBoundary();
    const Beat searchFromBeat = beatFromFramePos(searchFromPosition);
    auto it = std::lower_bound(m_beats.cbegin(), m_beats.cend(), searchFromBeat, beatLessThan);

    if (searchForward) {
        // Search in forward direction
        numBeatsLeft--;

        while (it != m_beats.cend() && numBeatsLeft > 0) {
            if (it->enabled()) {
                numBeatsLeft--;
            }
            it++;
        }
    } else {
        // Search in backward direction
        numBeatsLeft--;

        if (it == m_beats.cend() || it->frame_position() > searchFromBeat.frame_position()) {
            // We may be one beat behind the searchFromBeat. In this case, we advance
            // the reverse iterator by one beat.
            if (it == m_beats.cbegin()) {
                return audio::kInvalidFramePos;
            }
            it--;
        }

        while (it != m_beats.cbegin() && numBeatsLeft > 0) {
            if (it->enabled()) {
                numBeatsLeft--;
            }
            it--;
        }
    }

    if (numBeatsLeft > 0 || it == m_beats.cend()) {
        return audio::kInvalidFramePos;
    }

    const auto foundBeatPosition = mixxx::audio::FramePos(it->frame_position());

    DEBUG_ASSERT(foundBeatPosition >= searchFromPosition || !searchForward);
    DEBUG_ASSERT(foundBeatPosition <= searchFromPosition || searchForward);
    return foundBeatPosition;
}

bool BeatMap::findPrevNextBeats(audio::FramePos position,
        audio::FramePos* prevBeatPosition,
        audio::FramePos* nextBeatPosition,
        bool snapToNearBeats) const {
    *prevBeatPosition = audio::kInvalidFramePos;
    *nextBeatPosition = audio::kInvalidFramePos;
    if (!isValid() || !position.isValid()) {
        return false;
    }

    Beat beat = beatFromFramePos(position.toNearestFrameBoundary());

    // it points at the first occurrence of beat or the next largest beat
    BeatList::const_iterator it =
            std::lower_bound(m_beats.constBegin(), m_beats.constEnd(), beat, beatLessThan);

    // If the position is within 1/10th of a second of the next or previous
    // beat, pretend we are on that beat.
    const double kFrameEpsilon = 0.1 * m_sampleRate;

    // Back-up by one.
    if (it != m_beats.begin()) {
        --it;
    }

    // Scan forward to find whether we are on a beat.
    BeatList::const_iterator on_beat = m_beats.constEnd();
    BeatList::const_iterator previous_beat = m_beats.constEnd();
    BeatList::const_iterator next_beat = m_beats.constEnd();
    for (; it != m_beats.end(); ++it) {
        qint32 delta = it->frame_position() - beat.frame_position();

        if ((!snapToNearBeats && (delta == 0)) ||
                (snapToNearBeats && (abs(delta) < kFrameEpsilon))) {
            // We are "on" this beat.
            on_beat = it;
            break;
        }

        if (delta < 0) {
            // If we are not on the beat and delta < 0 then this beat comes
            // before our current position.
            previous_beat = it;
        } else {
            // If we are past the beat and we aren't on it then this beat comes
            // after our current position.
            next_beat = it;
            // Stop because we have everything we need now.
            break;
        }
    }

    // If we are within epsilon samples of a beat then the immediately next and
    // previous beats are the beat we are on.
    if (on_beat != m_beats.end()) {
        previous_beat = on_beat;
        next_beat = on_beat + 1;
    }

    for (; next_beat != m_beats.end(); ++next_beat) {
        if (!next_beat->enabled()) {
            continue;
        }
        *nextBeatPosition = mixxx::audio::FramePos(next_beat->frame_position());
        break;
    }
    if (previous_beat != m_beats.end()) {
        for (; true; --previous_beat) {
            if (previous_beat->enabled()) {
                *prevBeatPosition = mixxx::audio::FramePos(previous_beat->frame_position());
                break;
            }

            // Don't step before the start of the list.
            if (previous_beat == m_beats.begin()) {
                break;
            }
        }
    }
    return prevBeatPosition->isValid() && nextBeatPosition->isValid();
}

std::unique_ptr<BeatIterator> BeatMap::findBeats(
        audio::FramePos startPosition, audio::FramePos endPosition) const {
    // FIXME: Should this be a VERIFY_OR_DEBUG_ASSERT?
    if (!isValid() || !startPosition.isValid() || !endPosition.isValid() ||
            startPosition > endPosition) {
        return std::unique_ptr<BeatIterator>();
    }

    // Beats can only appear a full frame positions. If the start position is
    // fractional, it needs to be rounded up to avoid finding a beat that
    // appears *before* the requested start position. For the end position, the
    // opposite applies, i.e. we need to round down to avoid finding a beat
    // *after* the requested end position.
    Beat startBeat = beatFromFramePos(startPosition.toUpperFrameBoundary());
    Beat endBeat = beatFromFramePos(endPosition.toLowerFrameBoundary());

    BeatList::const_iterator curBeat =
            std::lower_bound(m_beats.constBegin(), m_beats.constEnd(), startBeat, beatLessThan);

    BeatList::const_iterator lastBeat =
            std::upper_bound(m_beats.constBegin(), m_beats.constEnd(), endBeat, beatLessThan);

    if (curBeat >= lastBeat) {
        return std::unique_ptr<BeatIterator>();
    }
    return std::make_unique<BeatMapIterator>(curBeat, lastBeat);
}

bool BeatMap::hasBeatInRange(audio::FramePos startPosition, audio::FramePos endPosition) const {
    // FIXME: Should this be a VERIFY_OR_DEBUG_ASSERT?
    if (!isValid() || !startPosition.isValid() || !endPosition.isValid() ||
            startPosition > endPosition) {
        return false;
    }
    audio::FramePos beatPosition = findNextBeat(startPosition.toUpperFrameBoundary());

    // FIXME: The following assertion should always hold true, but it doesn't,
    // because the position matching in findNthBeat() is fuzzy. This should be
    // resolved, and moved to the calling code, to only use fuzzy matches when
    // actually desired.
    // DEBUG_ASSERT(!beatPosition.isValid() || beatPosition >= startPosition.toUpperFrameBoundary());
    if (beatPosition.isValid() && beatPosition <= endPosition.toLowerFrameBoundary()) {
        return true;
    }
    return false;
}

mixxx::Bpm BeatMap::getBpmInRange(
        audio::FramePos startPosition, audio::FramePos endPosition) const {
    if (!isValid()) {
        return {};
    }

    const Beat startBeat = beatFromFramePos(startPosition.toUpperFrameBoundary());
    const Beat endBeat = beatFromFramePos(endPosition.toLowerFrameBoundary());

    BeatList::const_iterator start = std::lower_bound(
            m_beats.constBegin(), m_beats.constEnd(), startBeat, beatLessThan);
    const BeatList::const_iterator end = std::upper_bound(
            m_beats.constBegin(), m_beats.constEnd(), endBeat, beatLessThan);

    const int numBeats = std::distance(start, end);
    if (numBeats < 2) {
        return {};
    }

    QVector<mixxx::audio::FramePos> beats;
    beats.reserve(numBeats);
    std::transform(start, end, std::back_inserter(beats), [](const Beat& beat) -> audio::FramePos {
        return audio::FramePos(beat.frame_position());
    });
    DEBUG_ASSERT(beats.size() == numBeats);

    return BeatUtils::calculateBpm(beats, m_sampleRate);
}

// Note: Also called from the engine thread
mixxx::Bpm BeatMap::getBpmAroundPosition(audio::FramePos position, int n) const {
    if (!isValid()) {
        return {};
    }

    // To make sure we are always counting n beats, iterate backward to the
    // lower bound, then iterate forward from there to the upper bound.
    // a value of -1 indicates we went off the map -- count from the beginning.
    audio::FramePos lowerFrame = findNthBeat(position, -n);
    if (!lowerFrame.isValid()) {
        lowerFrame = mixxx::audio::FramePos(m_beats.first().frame_position());
    }

    // If we hit the end of the beat map, recalculate the lower bound.
    audio::FramePos upperFrame = findNthBeat(lowerFrame, n * 2);
    if (!upperFrame.isValid()) {
        upperFrame = mixxx::audio::FramePos(m_beats.last().frame_position());
        lowerFrame = findNthBeat(upperFrame, n * -2);
        // Super edge-case -- the track doesn't have n beats!  Do the best
        // we can.
        if (!lowerFrame.isValid()) {
            lowerFrame = mixxx::audio::FramePos(m_beats.first().frame_position());
        }
    }

    VERIFY_OR_DEBUG_ASSERT(lowerFrame < upperFrame) {
        return {};
    }

    const int kFrameEpsilon = m_sampleRate / 20;

    int numberOfBeats = 0;
    for (const auto& beat : m_beats) {
        const auto pos = mixxx::audio::FramePos(beat.frame_position() + kFrameEpsilon);
        if (pos > upperFrame) {
            break;
        }
        if (pos > lowerFrame) {
            numberOfBeats++;
        }
    }

    return BeatUtils::calculateAverageBpm(
            numberOfBeats, m_sampleRate, lowerFrame, upperFrame);
}

std::optional<BeatsPointer> BeatMap::tryTranslate(audio::FrameDiff_t offset) const {
    if (!isValid()) {
        return std::nullopt;
    }

    BeatList beats = m_beats;
    for (BeatList::iterator it = beats.begin();
            it != beats.end();) {
        const auto oldPosition = mixxx::audio::FramePos(it->frame_position());
        mixxx::audio::FramePos newPosition = oldPosition + offset;

        // FIXME: Don't we allow negative positions?
        if (newPosition >= mixxx::audio::kStartFramePos) {
            it->set_frame_position(static_cast<google::protobuf::int32>(newPosition.value()));
            ++it;
        } else {
            it = beats.erase(it);
        }
    }

    return std::make_shared<BeatMap>(MakeSharedTag{}, *this, beats);
}

std::optional<BeatsPointer> BeatMap::tryScale(BpmScale scale) const {
    VERIFY_OR_DEBUG_ASSERT(isValid()) {
        return std::nullopt;
    }

    BeatList beats = m_beats;
    switch (scale) {
    case BpmScale::Double:
        // introduce a new beat into every gap
        scaleDouble(&beats);
        break;
    case BpmScale::Halve:
        // remove every second beat
        scaleHalve(&beats);
        break;
    case BpmScale::TwoThirds:
        // introduce a new beat into every gap
        scaleDouble(&beats);
        // remove every second and third beat
        scaleThird(&beats);
        break;
    case BpmScale::ThreeFourths:
        // introduce two beats into every gap
        scaleTriple(&beats);
        // remove every second third and forth beat
        scaleFourth(&beats);
        break;
    case BpmScale::FourThirds:
        // introduce three beats into every gap
        scaleQuadruple(&beats);
        // remove every second third and forth beat
        scaleThird(&beats);
        break;
    case BpmScale::ThreeHalves:
        // introduce two beats into every gap
        scaleTriple(&beats);
        // remove every second beat
        scaleHalve(&beats);
        break;
    default:
        DEBUG_ASSERT(!"scale value invalid");
        return std::nullopt;
    }

    return std::make_shared<BeatMap>(MakeSharedTag{}, *this, beats);
}

std::optional<BeatsPointer> BeatMap::trySetBpm(mixxx::Bpm bpm) const {
    VERIFY_OR_DEBUG_ASSERT(bpm.isValid()) {
        return std::nullopt;
    }

    const auto firstBeatPosition = mixxx::audio::FramePos(m_beats.first().frame_position());
    DEBUG_ASSERT(firstBeatPosition.isValid());

    return fromConstTempo(m_sampleRate, firstBeatPosition, bpm);
}

} // namespace mixxx
