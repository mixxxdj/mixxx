/*
 * beatmap.cpp
 *
 *  Created on: 08/dic/2011
 *      Author: vittorio
 */

#include "track/beatmap.h"

#include <QMutexLocker>
#include <QtDebug>
#include <QtGlobal>
#include <algorithm>

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

bool BeatLessThan(const Beat& beat1, const Beat& beat2) {
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

mixxx::Bpm calculateNominalBpm(const BeatList& beats, mixxx::audio::SampleRate sampleRate) {
    QVector<mixxx::audio::FramePos> beatvect;
    beatvect.reserve(beats.size());
    for (const auto& beat : beats) {
        if (beat.enabled()) {
            beatvect.append(mixxx::audio::FramePos(beat.frame_position()));
        }
    }

    if (beatvect.size() < 2) {
        return {};
    }

    return BeatUtils::calculateBpm(beatvect, mixxx::audio::SampleRate(sampleRate));
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
        audio::SampleRate sampleRate,
        const QString& subVersion,
        BeatList beats,
        mixxx::Bpm nominalBpm)
        : m_subVersion(subVersion),
          m_sampleRate(sampleRate),
          m_nominalBpm(nominalBpm),
          m_beats(std::move(beats)) {
}

BeatMap::BeatMap(const BeatMap& other, BeatList beats, mixxx::Bpm nominalBpm)
        : m_subVersion(other.m_subVersion),
          m_sampleRate(other.m_sampleRate),
          m_nominalBpm(nominalBpm),
          m_beats(std::move(beats)) {
}

BeatMap::BeatMap(const BeatMap& other)
        : BeatMap(other, other.m_beats, other.m_nominalBpm) {
}

// static
BeatsPointer BeatMap::makeBeatMap(
        audio::SampleRate sampleRate,
        const QString& subVersion,
        const QByteArray& byteArray) {
    auto nominalBpm = mixxx::Bpm();
    BeatList beatList;

    track::io::BeatMap map;
    if (map.ParseFromArray(byteArray.constData(), byteArray.size())) {
        for (int i = 0; i < map.beat_size(); ++i) {
            const Beat& beat = map.beat(i);
            beatList.append(beat);
        }
        nominalBpm = calculateNominalBpm(beatList, sampleRate);
    } else {
        qDebug() << "ERROR: Could not parse BeatMap from QByteArray of size"
                 << byteArray.size();
    }
    return BeatsPointer(new BeatMap(sampleRate, subVersion, beatList, nominalBpm));
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
        const auto beatPos = mixxx::audio::FramePos(std::floor(originalBeatPos.value()));
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
    const auto nominalBpm = calculateNominalBpm(beatList, sampleRate);
    return BeatsPointer(new BeatMap(sampleRate, subVersion, beatList, nominalBpm));
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

audio::FramePos BeatMap::findNextBeat(audio::FramePos position) const {
    return findNthBeat(position, 1);
}

audio::FramePos BeatMap::findPrevBeat(audio::FramePos position) const {
    return findNthBeat(position, -1);
}

audio::FramePos BeatMap::findClosestBeat(audio::FramePos position) const {
    if (!isValid()) {
        return audio::kInvalidFramePos;
    }
    audio::FramePos prevBeatPosition;
    audio::FramePos nextBeatPosition;
    findPrevNextBeats(position, &prevBeatPosition, &nextBeatPosition, true);
    if (!prevBeatPosition.isValid()) {
        // If both positions are invalid, we correctly return an invalid position.
        return nextBeatPosition;
    }

    if (!nextBeatPosition.isValid()) {
        return prevBeatPosition;
    }

    // Both position are valid, return the closest position.
    return (nextBeatPosition - position > position - prevBeatPosition)
            ? prevBeatPosition
            : nextBeatPosition;
}

audio::FramePos BeatMap::findNthBeat(audio::FramePos position, int n) const {
    if (!isValid() || n == 0) {
        return audio::kInvalidFramePos;
    }

    Beat beat = beatFromFramePos(position);

    // it points at the first occurrence of beat or the next largest beat
    BeatList::const_iterator it =
            std::lower_bound(m_beats.constBegin(), m_beats.constEnd(), beat, BeatLessThan);

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

        // We are "on" this beat.
        if (abs(delta) < kFrameEpsilon) {
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
        next_beat = on_beat;
        previous_beat = on_beat;
    }

    if (n > 0) {
        for (; next_beat != m_beats.end(); ++next_beat) {
            if (!next_beat->enabled()) {
                continue;
            }
            if (n == 1) {
                return mixxx::audio::FramePos(next_beat->frame_position());
            }
            --n;
        }
    } else if (n < 0 && previous_beat != m_beats.end()) {
        for (; true; --previous_beat) {
            if (previous_beat->enabled()) {
                if (n == -1) {
                    return mixxx::audio::FramePos(previous_beat->frame_position());
                }
                ++n;
            }

            // Don't step before the start of the list.
            if (previous_beat == m_beats.begin()) {
                break;
            }
        }
    }
    return audio::kInvalidFramePos;
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

    Beat beat = beatFromFramePos(position);

    // it points at the first occurrence of beat or the next largest beat
    BeatList::const_iterator it =
            std::lower_bound(m_beats.constBegin(), m_beats.constEnd(), beat, BeatLessThan);

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

    Beat startBeat = beatFromFramePos(startPosition);
    Beat endBeat = beatFromFramePos(endPosition);

    BeatList::const_iterator curBeat =
            std::lower_bound(m_beats.constBegin(), m_beats.constEnd(),
                        startBeat, BeatLessThan);

    BeatList::const_iterator lastBeat =
            std::upper_bound(m_beats.constBegin(), m_beats.constEnd(), endBeat, BeatLessThan);

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
    audio::FramePos beatPosition = findNextBeat(startPosition);
    if (beatPosition <= endPosition) {
        return true;
    }
    return false;
}

mixxx::Bpm BeatMap::getBpm() const {
    if (!isValid()) {
        return {};
    }
    return m_nominalBpm;
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

BeatsPointer BeatMap::translate(audio::FrameDiff_t offset) const {
    // Converting to frame offset
    if (!isValid()) {
        return BeatsPointer(new BeatMap(*this));
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

    return BeatsPointer(new BeatMap(*this, beats, m_nominalBpm));
}

BeatsPointer BeatMap::scale(BpmScale scale) const {
    if (!isValid() || m_beats.isEmpty()) {
        return BeatsPointer(new BeatMap(*this));
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
        return BeatsPointer(new BeatMap(*this));
    }

    mixxx::Bpm bpm = calculateNominalBpm(beats, m_sampleRate);
    return BeatsPointer(new BeatMap(*this, beats, bpm));
}

BeatsPointer BeatMap::setBpm(mixxx::Bpm bpm) {
    Q_UNUSED(bpm);
    DEBUG_ASSERT(!"BeatMap::setBpm() not implemented");
    return BeatsPointer(new BeatMap(*this));

    /*
     * One of the problems of beattracking algorithms is the so called "octave error"
     * that is, calculated bpm is a power-of-two fraction of the bpm of the track.
     * But there is more. In an experiment, it had been proved that roughly 30% of the humans
     * fail to guess the correct bpm of a track by usually reporting it as the double or one
     * half of the correct one.
     * We can interpret it in two ways:
     * On one hand, a beattracking algorithm which totally avoid the octave error does not yet exists.
     * On the other hand, even if the algorithm guesses the correct bpm,
     * 30% of the users will perceive a different bpm and likely change it.
     * In this case, we assume that calculated beat markers are correctly placed. All
     * that we have to do is to delete or add some beat markers, while leaving others
     * so that the number of the beat markers per minute matches the new bpm.
     * We are jealous of our well-guessed beats since they belong to a time-expensive analysis.
     * When requested we simply turn them off instead of deleting them, so that they can be recollected.
     * If the new provided bpm is not a power-of-two fraction, we assume that the algorithm failed
     * at all to guess the bpm. I have no idea on how to deal with this.
     * If we assume that bpm does not change along the track, i.e. if we use
     * fixed tempo approximation (see analyzerbeat.*), this should coincide with the
     * method in beatgrid.cpp.
     *
     * - vittorio.
     */
}

} // namespace mixxx
