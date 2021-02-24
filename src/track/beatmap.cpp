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

const int kFrameSize = 2;

inline double samplesToFrames(const double samples) {
    return floor(samples / kFrameSize);
}

inline double framesToSamples(const int frames) {
    return frames * kFrameSize;
}

bool BeatLessThan(const Beat& beat1, const Beat& beat2) {
    return beat1.frame_position() < beat2.frame_position();
}

double calculateNominalBpm(const BeatList& beats, SINT sampleRate) {
    QVector<double> beatvect;
    beatvect.reserve(beats.size());
    for (const auto& beat : beats) {
        if (beat.enabled()) {
            beatvect.append(beat.frame_position());
        }
    }

    if (beatvect.size() < 2) {
        return -1;
    }

    return BeatUtils::calculateBpm(beatvect, sampleRate, 0, 9999);
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

    double next() override {
        double beat = framesToSamples(m_currentBeat->frame_position());
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

BeatMap::BeatMap(const Track& track, SINT iSampleRate)
        : m_mutex(QMutex::Recursive),
          m_iSampleRate(iSampleRate > 0 ? iSampleRate : track.getSampleRate()),
          m_nominalBpm(0) {
    // BeatMap should live in the same thread as the track it is associated
    // with.
    moveToThread(track.thread());
}

BeatMap::BeatMap(const Track& track, SINT iSampleRate,
                 const QByteArray& byteArray)
    : BeatMap(track, iSampleRate) {
    readByteArray(byteArray);
}

BeatMap::BeatMap(const Track& track, SINT iSampleRate,
                 const QVector<double>& beats)
        : BeatMap(track, iSampleRate) {
    if (beats.size() > 0) {
        createFromBeatVector(beats);
    }
}

BeatMap::BeatMap(const BeatMap& other)
        : m_mutex(QMutex::Recursive),
          m_subVersion(other.m_subVersion),
          m_iSampleRate(other.m_iSampleRate),
          m_nominalBpm(other.m_nominalBpm),
          m_beats(other.m_beats) {
    moveToThread(other.thread());
}

QByteArray BeatMap::toByteArray() const {
    QMutexLocker locker(&m_mutex);
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

BeatsPointer BeatMap::clone() const {
    QMutexLocker locker(&m_mutex);
    BeatsPointer other(new BeatMap(*this));
    return other;
}

bool BeatMap::readByteArray(const QByteArray& byteArray) {
    mixxx::track::io::BeatMap map;
    if (!map.ParseFromArray(byteArray.constData(), byteArray.size())) {
        qDebug() << "ERROR: Could not parse BeatMap from QByteArray of size"
                << byteArray.size();
        return false;
    }
    for (int i = 0; i < map.beat_size(); ++i) {
        const Beat& beat = map.beat(i);
        m_beats.append(beat);
    }
    onBeatlistChanged();
    return true;
}

void BeatMap::createFromBeatVector(const QVector<double>& beats) {
    if (beats.isEmpty()) {
       return;
    }
    double previous_beatpos = -1;
    Beat beat;

    foreach (double beatpos, beats) {
        // beatpos is in frames. Do not accept fractional frames.
        beatpos = floor(beatpos);
        if (beatpos <= previous_beatpos || beatpos < 0) {
            qDebug() << "BeatMap::createFromVector: beats not in increasing order or negative";
            qDebug() << "discarding beat " << beatpos;
        } else {
            beat.set_frame_position(static_cast<google::protobuf::int32>(beatpos));
            m_beats.append(beat);
            previous_beatpos = beatpos;
        }
    }
    onBeatlistChanged();
}

QString BeatMap::getVersion() const {
    QMutexLocker locker(&m_mutex);
    return BEAT_MAP_VERSION;
}

QString BeatMap::getSubVersion() const {
    QMutexLocker locker(&m_mutex);
    return m_subVersion;
}

void BeatMap::setSubVersion(const QString& subVersion) {
    m_subVersion = subVersion;
}

bool BeatMap::isValid() const {
    return m_iSampleRate > 0 && m_beats.size() > 0;
}

double BeatMap::findNextBeat(double dSamples) const {
    return findNthBeat(dSamples, 1);
}

double BeatMap::findPrevBeat(double dSamples) const {
    return findNthBeat(dSamples, -1);
}

double BeatMap::findClosestBeat(double dSamples) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return -1;
    }
    double prevBeat;
    double nextBeat;
    findPrevNextBeats(dSamples, &prevBeat, &nextBeat);
    if (prevBeat == -1) {
        // If both values are -1, we correctly return -1.
        return nextBeat;
    } else if (nextBeat == -1) {
        return prevBeat;
    }
    return (nextBeat - dSamples > dSamples - prevBeat) ? prevBeat : nextBeat;
}

double BeatMap::findNthBeat(double dSamples, int n) const {
    QMutexLocker locker(&m_mutex);

    if (!isValid() || n == 0) {
        return -1;
    }

    Beat beat;
    // Reduce sample offset to a frame offset.
    beat.set_frame_position(static_cast<google::protobuf::int32>(samplesToFrames(dSamples)));

    // it points at the first occurrence of beat or the next largest beat
    BeatList::const_iterator it =
            std::lower_bound(m_beats.constBegin(), m_beats.constEnd(), beat, BeatLessThan);

    // If the position is within 1/10th of a second of the next or previous
    // beat, pretend we are on that beat.
    const double kFrameEpsilon = 0.1 * m_iSampleRate;

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
                // Return a sample offset
                return framesToSamples(next_beat->frame_position());
            }
            --n;
        }
    } else if (n < 0 && previous_beat != m_beats.end()) {
        for (; true; --previous_beat) {
            if (previous_beat->enabled()) {
                if (n == -1) {
                    // Return a sample offset
                    return framesToSamples(previous_beat->frame_position());
                }
                ++n;
            }

            // Don't step before the start of the list.
            if (previous_beat == m_beats.begin()) {
                break;
            }
        }
    }
    return -1;
}

bool BeatMap::findPrevNextBeats(double dSamples,
                                double* dpPrevBeatSamples,
                                double* dpNextBeatSamples) const {
    QMutexLocker locker(&m_mutex);

    if (!isValid()) {
        *dpPrevBeatSamples = -1;
        *dpNextBeatSamples = -1;
        return false;
    }

    Beat beat;
    // Reduce sample offset to a frame offset.
    beat.set_frame_position(static_cast<google::protobuf::int32>(samplesToFrames(dSamples)));

    // it points at the first occurrence of beat or the next largest beat
    BeatList::const_iterator it =
            std::lower_bound(m_beats.constBegin(), m_beats.constEnd(), beat, BeatLessThan);

    // If the position is within 1/10th of a second of the next or previous
    // beat, pretend we are on that beat.
    const double kFrameEpsilon = 0.1 * m_iSampleRate;

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
        previous_beat = on_beat;
        next_beat = on_beat + 1;
    }

    *dpPrevBeatSamples = -1;
    *dpNextBeatSamples = -1;

    for (; next_beat != m_beats.end(); ++next_beat) {
        if (!next_beat->enabled()) {
            continue;
        }
        *dpNextBeatSamples = framesToSamples(next_beat->frame_position());
        break;
    }
    if (previous_beat != m_beats.end()) {
        for (; true; --previous_beat) {
            if (previous_beat->enabled()) {
                *dpPrevBeatSamples = framesToSamples(previous_beat->frame_position());
                break;
            }

            // Don't step before the start of the list.
            if (previous_beat == m_beats.begin()) {
                break;
            }
        }
    }
    return *dpPrevBeatSamples != -1 && *dpNextBeatSamples != -1;
}

std::unique_ptr<BeatIterator> BeatMap::findBeats(double startSample, double stopSample) const {
    QMutexLocker locker(&m_mutex);
    //startSample and stopSample are sample offsets, converting them to
    //frames
    if (!isValid() || startSample > stopSample) {
        return std::unique_ptr<BeatIterator>();
    }

    Beat startBeat, stopBeat;
    startBeat.set_frame_position(
            static_cast<google::protobuf::int32>(samplesToFrames(startSample)));
    stopBeat.set_frame_position(static_cast<google::protobuf::int32>(samplesToFrames(stopSample)));

    BeatList::const_iterator curBeat =
            std::lower_bound(m_beats.constBegin(), m_beats.constEnd(),
                        startBeat, BeatLessThan);

    BeatList::const_iterator lastBeat =
            std::upper_bound(m_beats.constBegin(), m_beats.constEnd(),
                        stopBeat, BeatLessThan);

    if (curBeat >= lastBeat) {
        return std::unique_ptr<BeatIterator>();
    }
    return std::make_unique<BeatMapIterator>(curBeat, lastBeat);
}

bool BeatMap::hasBeatInRange(double startSample, double stopSample) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid() || startSample > stopSample) {
        return false;
    }
    double curBeat = findNextBeat(startSample);
    if (curBeat <= stopSample) {
        return true;
    }
    return false;
}

double BeatMap::getBpm() const {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return -1;
    }
    return m_nominalBpm;
}

// Note: Also called from the engine thread
double BeatMap::getBpmAroundPosition(double curSample, int n) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return -1;
    }

    // To make sure we are always counting n beats, iterate backward to the
    // lower bound, then iterate forward from there to the upper bound.
    // a value of -1 indicates we went off the map -- count from the beginning.
    double lowerSample = findNthBeat(curSample, -n);
    if (lowerSample == -1) {
        lowerSample = framesToSamples(m_beats.first().frame_position());
    }

    // If we hit the end of the beat map, recalculate the lower bound.
    double upperSample = findNthBeat(lowerSample, n * 2);
    if (upperSample == -1) {
        upperSample = framesToSamples(m_beats.last().frame_position());
        lowerSample = findNthBeat(upperSample, n * -2);
        // Super edge-case -- the track doesn't have n beats!  Do the best
        // we can.
        if (lowerSample == -1) {
            lowerSample = framesToSamples(m_beats.first().frame_position());
        }
    }

    double lowerFrame = samplesToFrames(lowerSample);
    double upperFrame = samplesToFrames(upperSample);

    VERIFY_OR_DEBUG_ASSERT(lowerFrame < upperFrame) {
        return -1;
    }

    const int kFrameEpsilon = m_iSampleRate / 20;

    int numberOfBeats = 0;
    for (const auto& beat : m_beats) {
        double pos = beat.frame_position() + kFrameEpsilon;
        if (pos > upperFrame) {
            break;
        }
        if (pos > lowerFrame) {
            numberOfBeats++;
        }
    }

    return BeatUtils::calculateAverageBpm(numberOfBeats, m_iSampleRate, lowerFrame, upperFrame);
}

void BeatMap::addBeat(double dBeatSample) {
    QMutexLocker locker(&m_mutex);
    Beat beat;
    beat.set_frame_position(static_cast<google::protobuf::int32>(samplesToFrames(dBeatSample)));
    BeatList::iterator it = std::lower_bound(
        m_beats.begin(), m_beats.end(), beat, BeatLessThan);

    // Don't insert a duplicate beat. TODO(XXX) determine what epsilon to
    // consider a beat identical to another.
    if (it != m_beats.end() && it->frame_position() == beat.frame_position()) {
        return;
    }

    m_beats.insert(it, beat);
    onBeatlistChanged();
    locker.unlock();
    emit updated();
}

void BeatMap::removeBeat(double dBeatSample) {
    QMutexLocker locker(&m_mutex);
    Beat beat;
    beat.set_frame_position(static_cast<google::protobuf::int32>(samplesToFrames(dBeatSample)));
    BeatList::iterator it = std::lower_bound(
        m_beats.begin(), m_beats.end(), beat, BeatLessThan);

    // In case there are duplicates, remove every instance of dBeatSample
    // TODO(XXX) add invariant checks against this
    // TODO(XXX) determine what epsilon to consider a beat identical to another
    while (it->frame_position() == beat.frame_position()) {
        it = m_beats.erase(it);
    }
    onBeatlistChanged();
    locker.unlock();
    emit updated();
}

void BeatMap::translate(double dNumSamples) {
    QMutexLocker locker(&m_mutex);
    // Converting to frame offset
    if (!isValid()) {
        return;
    }

    double dNumFrames = samplesToFrames(dNumSamples);
    for (BeatList::iterator it = m_beats.begin();
         it != m_beats.end(); ) {
        double newpos = it->frame_position() + dNumFrames;
        if (newpos >= 0) {
            it->set_frame_position(static_cast<google::protobuf::int32>(newpos));
            ++it;
        } else {
            it = m_beats.erase(it);
        }
    }
    onBeatlistChanged();
    locker.unlock();
    emit updated();
}

void BeatMap::scale(enum BPMScale scale) {

    QMutexLocker locker(&m_mutex);
    if (!isValid() || m_beats.isEmpty()) {
        return;
    }

    switch (scale) {
    case DOUBLE:
        // introduce a new beat into every gap
        scaleDouble();
        break;
    case HALVE:
        // remove every second beat
        scaleHalve();
        break;
    case TWOTHIRDS:
        // introduce a new beat into every gap
        scaleDouble();
        // remove every second and third beat
        scaleThird();
        break;
    case THREEFOURTHS:
        // introduce two beats into every gap
        scaleTriple();
        // remove every second third and forth beat
        scaleFourth();
        break;
    case FOURTHIRDS:
        // introduce three beats into every gap
        scaleQuadruple();
        // remove every second third and forth beat
        scaleThird();
        break;
    case THREEHALVES:
        // introduce two beats into every gap
        scaleTriple();
        // remove every second beat
        scaleHalve();
        break;
    default:
        DEBUG_ASSERT(!"scale value invalid");
        return;
    }
    onBeatlistChanged();
    locker.unlock();
    emit updated();
}

void BeatMap::scaleDouble() {
    Beat prevBeat = m_beats.first();
    // Skip the first beat to preserve the first beat in a measure
    BeatList::iterator it = m_beats.begin() + 1;
    for (; it != m_beats.end(); ++it) {
        // Need to not accrue fractional frames.
        int distance = it->frame_position() - prevBeat.frame_position();
        Beat beat;
        beat.set_frame_position(prevBeat.frame_position() + distance / 2);
        it = m_beats.insert(it, beat);
        prevBeat = (++it)[0];
    }
}

void BeatMap::scaleTriple() {
    Beat prevBeat = m_beats.first();
    // Skip the first beat to preserve the first beat in a measure
    BeatList::iterator it = m_beats.begin() + 1;
    for (; it != m_beats.end(); ++it) {
        // Need to not accrue fractional frames.
        int distance = it->frame_position() - prevBeat.frame_position();
        Beat beat;
        beat.set_frame_position(prevBeat.frame_position() + distance / 3);
        it = m_beats.insert(it, beat);
        ++it;
        beat.set_frame_position(prevBeat.frame_position() + distance * 2 / 3);
        it = m_beats.insert(it, beat);
        prevBeat = (++it)[0];
    }
}

void BeatMap::scaleQuadruple() {
    Beat prevBeat = m_beats.first();
    // Skip the first beat to preserve the first beat in a measure
    BeatList::iterator it = m_beats.begin() + 1;
    for (; it != m_beats.end(); ++it) {
        // Need to not accrue fractional frames.
        int distance = it->frame_position() - prevBeat.frame_position();
        Beat beat;
        for (int i = 1; i <= 3; i++) {
            beat.set_frame_position(prevBeat.frame_position() + distance * i / 4);
            it = m_beats.insert(it, beat);
            ++it;
        }
        prevBeat = it[0];
    }
}

void BeatMap::scaleHalve() {
    // Skip the first beat to preserve the first beat in a measure
    BeatList::iterator it = m_beats.begin() + 1;
    for (; it != m_beats.end(); ++it) {
        it = m_beats.erase(it);
        if (it == m_beats.end()) {
            break;
        }
    }
}

void BeatMap::scaleThird() {
    // Skip the first beat to preserve the first beat in a measure
    BeatList::iterator it = m_beats.begin() + 1;
    for (; it != m_beats.end(); ++it) {
        it = m_beats.erase(it);
        if (it == m_beats.end()) {
            break;
        }
        it = m_beats.erase(it);
        if (it == m_beats.end()) {
            break;
        }
    }
}

void BeatMap::scaleFourth() {
    // Skip the first beat to preserve the first beat in a measure
    BeatList::iterator it = m_beats.begin() + 1;
    for (; it != m_beats.end(); ++it) {
        it = m_beats.erase(it);
        if (it == m_beats.end()) {
            break;
        }
        it = m_beats.erase(it);
        if (it == m_beats.end()) {
            break;
        }
        it = m_beats.erase(it);
        if (it == m_beats.end()) {
            break;
        }
    }
}

void BeatMap::setBpm(double dBpm) {
    Q_UNUSED(dBpm);
    DEBUG_ASSERT(!"BeatMap::setBpm() not implemented");
    return;

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

void BeatMap::onBeatlistChanged() {
    if (!isValid()) {
        m_nominalBpm = 0;
        return;
    }
    m_nominalBpm = calculateNominalBpm(m_beats, m_iSampleRate);
}

} // namespace mixxx
