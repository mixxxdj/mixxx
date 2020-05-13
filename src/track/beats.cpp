
#include "track/beats.h"

#include "track/beatutils.h"

namespace mixxx {

namespace {
const int kFrameSize = 2;

bool BeatLessThan(const track::io::Beat& beat1, const track::io::Beat& beat2) {
    return beat1.frame_position() < beat2.frame_position();
}

double framesToSamples(const int frames) {
    return frames * kFrameSize;
}

} // namespace

Beats::Beats(const Track* track, const QVector<double>& beats, SINT iSampleRate)
        : Beats(track, iSampleRate) {
    if (beats.size() > 0) {
        createFromBeatVector(beats);
    }
}

Beats::Beats(const Track* track, const QByteArray& byteArray, SINT iSampleRate)
        : Beats(track, iSampleRate) {
    readByteArray(byteArray);
    setGlobalBpm(calculateBpm(m_beats.first(), m_beats.last()));
}

Beats::Beats(const Track* track, SINT iSampleRate)
        : m_mutex(QMutex::Recursive),
          m_track(track),
          m_iSampleRate(iSampleRate == 0 ? m_track->getSampleRate() : iSampleRate),
          m_dCachedBpm(0),
          m_dLastFrame(0) {
    // TODO(JVC) iSampleRate == 0 creates problems. Apparently only in tests

    // BeatMap should live in the same thread as the track it is associated
    // with.
    moveToThread(track->thread());
}

// TODO(JVC) Do we really need a copy constructor??
Beats::Beats(const Beats& other)
        : m_mutex(QMutex::Recursive),
          m_track(other.m_track),
          m_subVersion(other.m_subVersion),
          m_iSampleRate(other.m_iSampleRate),
          m_isTempoConst(other.m_isTempoConst),
          m_globalBpm(other.m_globalBpm),
          m_dCachedBpm(other.m_dCachedBpm),
          m_dLastFrame(other.m_dLastFrame),
          m_beats(other.m_beats) {
    moveToThread(m_track->thread());
}

void Beats::createFromBeatVector(const QVector<double>& beats) {
    if (beats.isEmpty()) {
        return;
    }
    double previous_beatpos = -1;
    track::io::Beat beat;

    foreach (double beatpos, beats) {
        // beatpos is in frames. Do not accept fractional frames.
        beatpos = floor(beatpos);
        if (beatpos <= previous_beatpos || beatpos < 0) {
            qDebug() << "kBeatMap::createFromVector: beats not in increasing order or negative";
            qDebug() << "discarding beat " << beatpos;
        } else {
            beat.set_frame_position(beatpos);
            m_beats.append(beat);
            previous_beatpos = beatpos;
        }
    }
    onBeatlistChanged();
}

int Beats::numBeatsInRangeNew(FrameNum startFrame, FrameNum endFrame) {
    double dLastCountedBeat = 0.0;
    int iBeatsCounter;
    for (iBeatsCounter = 1; dLastCountedBeat < endFrame; iBeatsCounter++) {
        dLastCountedBeat = findNthBeat(startFrame, iBeatsCounter);
        if (dLastCountedBeat == -1) {
            break;
        }
    }
    return iBeatsCounter - 2;
};

QByteArray Beats::toProtobuff() const {
    QMutexLocker locker(&m_mutex);
    // No guarantees BeatLists are made of a data type which located adjacent
    // items in adjacent memory locations.
    track::io::BeatMap map;

    for (int i = 0; i < m_beats.size(); ++i) {
        map.add_beat()->CopyFrom(m_beats[i]);
    }

    std::string output;
    map.SerializeToString(&output);
    return QByteArray(output.data(), output.length());
}

BeatsPointer Beats::clone() const {
    QMutexLocker locker(&m_mutex);
    // TODO(JVC)
    BeatsPointer other(new Beats(*this));
    return other;
}

QString Beats::getVersion() const {
    QMutexLocker locker(&m_mutex);
    return BEAT_MAP_VERSION;
}

QString Beats::getSubVersion() const {
    QMutexLocker locker(&m_mutex);
    return m_subVersion;
}

double Beats::findNextBeatNew(FrameNum frame) const {
    return findNthBeatNew(frame, 1);
}

void Beats::setSubVersion(QString subVersion) {
    m_subVersion = subVersion;
}

void Beats::setGridNew(double dBpm, FrameNum firstBeatFrame) {
    QMutexLocker lock(&m_mutex);
    if (dBpm < 0) {
        dBpm = 0.0;
    }

    // If the track duration is not know assume 120 seconds, useful for tests
    // TODO(JVC) - Check if there are other cases that can be affected.
    auto trackDuration = m_track->getDuration();
    auto trackLength = (trackDuration == 0 ? 120 : trackDuration) * m_iSampleRate;

    m_beats.clear();

    track::io::Beat beat;
    beat.set_frame_position(firstBeatFrame);
    for (FrameNum pos = firstBeatFrame; pos <= trackLength; pos += m_iSampleRate * (60 / dBpm)) {
        beat.set_frame_position(pos);
        m_beats.push_back(beat);
    }

    onBeatlistChanged();
}

double Beats::findNBeatsFromSampleNew(double fromSample, double beats) const {
    double nthBeat;
    double prevBeat;
    double nextBeat;

    if (!findPrevNextBeats(fromSample, &prevBeat, &nextBeat)) {
        return fromSample;
    }
    double fromFractionBeats = (fromSample - prevBeat) / (nextBeat - prevBeat);
    double beatsFromPrevBeat = fromFractionBeats + beats;

    int fullBeats = static_cast<int>(beatsFromPrevBeat);
    double fractionBeats = beatsFromPrevBeat - fullBeats;

    // Add the length between this beat and the fullbeats'th beat
    // to the end position
    if (fullBeats > 0) {
        nthBeat = findNthBeat(nextBeat, fullBeats);
    } else {
        nthBeat = findNthBeat(prevBeat, fullBeats - 1);
    }

    if (nthBeat == -1) {
        return fromSample;
    }

    // Add the fraction of the beat
    if (fractionBeats != 0) {
        nextBeat = findNthBeat(nthBeat, 2);
        if (nextBeat == -1) {
            return fromSample;
        }
        nthBeat += (nextBeat - nthBeat) * fractionBeats;
    }

    return nthBeat;
};

void Beats::onBeatlistChanged() {
    if (!isValid()) {
        m_dLastFrame = 0;
        m_dCachedBpm = 0;
        return;
    }
    m_dLastFrame = m_beats.last().frame_position();
    track::io::Beat startBeat = m_beats.first();
    track::io::Beat stopBeat = m_beats.last();
    m_dCachedBpm = calculateBpm(startBeat, stopBeat);
}

bool Beats::isValid() const {
    return m_iSampleRate > 0 && m_beats.size() > 0;
}

double Beats::calculateBpm(const track::io::Beat& startBeat, const track::io::Beat& stopBeat) const {
    if (startBeat.frame_position() > stopBeat.frame_position()) {
        return -1;
    }

    BeatList::const_iterator curBeat =
            std::lower_bound(m_beats.cbegin(), m_beats.cend(), startBeat, BeatLessThan);

    BeatList::const_iterator lastBeat =
            std::upper_bound(m_beats.cbegin(), m_beats.cend(), stopBeat, BeatLessThan);

    QVector<double> beatvect;
    for (; curBeat != lastBeat; ++curBeat) {
        const track::io::Beat& beat = *curBeat;
        if (beat.enabled()) {
            beatvect.append(beat.frame_position());
        }
    }

    if (beatvect.isEmpty()) {
        return -1;
    }

    return BeatUtils::calculateBpm(beatvect, m_iSampleRate, 0, 9999);
}

double Beats::findPrevBeatNew(FrameNum frame) const {
    return findNthBeatNew(frame, -1);
}

bool Beats::findPrevNextBeatsNew(FrameNum frame,
        FrameNum* pPrevBeatFrame,
        FrameNum* pNextBeatFrame) const {
    QMutexLocker locker(&m_mutex);

    if (!isValid()) {
        *pPrevBeatFrame = -1;
        *pNextBeatFrame = -1;
        return false;
    }

    track::io::Beat beat;
    beat.set_frame_position(frame);

    // it points at the first occurrence of beat or the next largest beat
    BeatList::const_iterator it =
            std::lower_bound(m_beats.cbegin(), m_beats.cend(), beat, BeatLessThan);

    // If the position is within 1/10th of a second of the next or previous
    // beat, pretend we are on that beat.
    const double kFrameEpsilon = 0.1 * m_iSampleRate;

    // Back-up by one.
    if (it != m_beats.begin()) {
        --it;
    }

    // Scan forward to find whether we are on a beat.
    BeatList::const_iterator on_beat = m_beats.cend();
    BeatList::const_iterator previous_beat = m_beats.cend();
    BeatList::const_iterator next_beat = m_beats.cend();
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

    *pPrevBeatFrame = -1;
    *pNextBeatFrame = -1;

    for (; next_beat != m_beats.end(); ++next_beat) {
        if (!next_beat->enabled()) {
            continue;
        }
        *pNextBeatFrame = next_beat->frame_position();
        break;
    }
    if (previous_beat != m_beats.end()) {
        for (; true; --previous_beat) {
            if (previous_beat->enabled()) {
                *pPrevBeatFrame = previous_beat->frame_position();
                break;
            }

            // Don't step before the start of the list.
            if (previous_beat == m_beats.begin()) {
                break;
            }
        }
    }
    return *pPrevBeatFrame != -1 && *pNextBeatFrame != -1;
}

double Beats::findClosestBeatNew(FrameNum frames) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return -1;
    }
    double prevBeat;
    double nextBeat;
    findPrevNextBeatsNew(frames, &prevBeat, &nextBeat);
    if (prevBeat == -1) {
        // If both values are -1, we correctly return -1.
        return nextBeat;
    } else if (nextBeat == -1) {
        return prevBeat;
    }
    return (nextBeat - frames > frames - prevBeat) ? prevBeat : nextBeat;
}

double Beats::findNthBeatNew(FrameNum frame, int n) const {
    QMutexLocker locker(&m_mutex);

    if (!isValid() || n == 0) {
        return -1;
    }

    track::io::Beat beat;
    beat.set_frame_position(frame);

    // it points at the first occurrence of beat or the next largest beat
    BeatList::const_iterator it =
            std::lower_bound(m_beats.cbegin(), m_beats.cend(), beat, BeatLessThan);

    // If the position is within 1/10th of a second of the next or previous
    // beat, pretend we are on that beat.
    const double kFrameEpsilon = 0.1 * m_iSampleRate;

    // Back-up by one.
    if (it != m_beats.begin()) {
        --it;
    }

    // Scan forward to find whether we are on a beat.
    BeatList::const_iterator on_beat = m_beats.cend();
    BeatList::const_iterator previous_beat = m_beats.cend();
    BeatList::const_iterator next_beat = m_beats.cend();
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

    // If we are within epsilon frames of a beat then the immediately next and
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
                return next_beat->frame_position();
            }
            --n;
        }
    } else if (n < 0 && previous_beat != m_beats.end()) {
        for (; true; --previous_beat) {
            if (previous_beat->enabled()) {
                if (n == -1) {
                    // Return a sample offset
                    return previous_beat->frame_position();
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

std::unique_ptr<BeatIterator> Beats::findBeatsNew(FrameNum startFrame, FrameNum stopFrame) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid() || startFrame > stopFrame) {
        return std::unique_ptr<BeatIterator>();
    }

    track::io::Beat startBeat, stopBeat;
    startBeat.set_frame_position(startFrame);
    stopBeat.set_frame_position(stopFrame);

    BeatList::const_iterator firstBeat =
            std::lower_bound(m_beats.cbegin(), m_beats.cend(), startBeat, BeatLessThan);

    BeatList::const_iterator lastBeat =
            std::upper_bound(m_beats.cbegin(), m_beats.cend(), stopBeat, BeatLessThan);
    if (lastBeat >= m_beats.cbegin()) {
        lastBeat = m_beats.cend() - 1;
    }

    if (firstBeat >= lastBeat) {
        return std::unique_ptr<BeatIterator>();
    }
    return std::make_unique<BeatIterator>(firstBeat, lastBeat);
}

bool Beats::hasBeatInRangeNew(double startSample, double stopSample) const {
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

double Beats::getBpmNew() const {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return -1;
    }
    return m_dCachedBpm;
}
double Beats::getBpmRangeNew(FrameNum startFrame, FrameNum stopFrame) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return -1;
    }
    track::io::Beat startBeat, stopBeat;
    startBeat.set_frame_position(startFrame);
    stopBeat.set_frame_position(stopFrame);
    return calculateBpm(startBeat, stopBeat);
}

double Beats::getBpmAroundPositionNew(FrameNum curFrame, int n) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return -1;
    }

    // To make sure we are always counting n beats, iterate backward to the
    // lower bound, then iterate forward from there to the upper bound.
    // a value of -1 indicates we went off the map -- count from the beginning.
    double lower_bound = findNthBeatNew(curFrame, -n);
    if (lower_bound == -1) {
        lower_bound = m_beats.first().frame_position();
    }

    // If we hit the end of the beat map, recalculate the lower bound.
    //double upper_bound = findNthBeat(lower_bound, n * 2);
    double upper_bound = findNthBeatNew(lower_bound, n);
    if (upper_bound == -1) {
        upper_bound = m_beats.last().frame_position();
        //lower_bound = findNthBeat(upper_bound, n * -2);
        lower_bound = findNthBeatNew(upper_bound, n * -1);
        // Super edge-case -- the track doesn't have n beats!  Do the best
        // we can.
        if (lower_bound == -1) {
            lower_bound = m_beats.first().frame_position();
        }
    }

    track::io::Beat startBeat, stopBeat;
    startBeat.set_frame_position(lower_bound);
    stopBeat.set_frame_position(upper_bound);
    return calculateBpm(startBeat, stopBeat);
}

void Beats::addBeatNew(FrameNum beatFrame) {
    QMutexLocker locker(&m_mutex);
    track::io::Beat beat;
    beat.set_frame_position(beatFrame);
    BeatList::iterator it = std::lower_bound(
            m_beats.begin(), m_beats.end(), beat, BeatLessThan);

    // Don't insert a duplicate beat. TODO(XXX) determine what epsilon to
    // consider a beat identical to another.
    if (it->frame_position() == beat.frame_position()) {
        return;
    }

    m_beats.insert(it, beat);
    onBeatlistChanged();
    locker.unlock();
    emit updated();
}

void Beats::removeBeatNew(FrameNum beatFrame) {
    QMutexLocker locker(&m_mutex);
    track::io::Beat beat;
    beat.set_frame_position(beatFrame);
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

TimeSignature Beats::getSignatureNew(FrameNum frame) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return kNullTimeSignature;
    }

    auto result = kDefaultTimeSignature;

    // Special case, when looking for initial TimeSignature
    if (frame == 0) {
        auto beat = m_beats.cbegin();
        if (beat->has_signature()) {
            result.setBeats(beat->signature().beats());
            result.setNoteValue(beat->signature().note_value());
        }
    } else {
        // Scans the list of beats to find the last time signature change before the sample
        for (auto beat = m_beats.begin(); beat != m_beats.end() && beat->frame_position() < frame; beat++) {
            if (beat->has_signature()) {
                result.setBeats(beat->signature().beats());
                result.setNoteValue(beat->signature().note_value());
            }
        }
    }
    return result;
}

void Beats::setSignatureNew(TimeSignature sig, FrameNum frame) {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return;
    }

    // Moves to the beat before the sample
    BeatList::iterator beat = m_beats.begin();
    for (; beat != m_beats.end() && beat->frame_position() < frame; ++beat)
        ;

    // If at the end, change nothing
    if (beat == m_beats.end()) {
        return;
    }

    // Adjust position if not at the first beat
    if (beat != m_beats.begin()) {
        beat--;
    }

    // Sets the TimeSignature value
    beat->mutable_signature()->set_beats(sig.getBeats());
    beat->mutable_signature()->set_note_value(sig.getNoteValue());
    locker.unlock();
    emit(updated());
}

void Beats::setDownBeatNew(FrameNum frame) {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return;
    }

    double closest_sample = findClosestBeat(frame);

    // Set the proper type for the remaining beats on the track or to the next phrasebeat
    int beat_counter = 0;
    std::unique_ptr<BeatIterator> beat = findBeats(closest_sample, (m_beats.last().frame_position() - 1) * kFrameSize);
    while (beat->hasNext()) {
        beat->next();
        if (beat->isPhrase()) {
            break;
        } else if (beat_counter % getSignature(frame).getBeats() == 0) {
            beat->makeBar();
        } else {
            beat->makeBeat();
        }

        beat_counter++;
    }
    onBeatlistChanged();
    locker.unlock();
    emit(updated());
}

void Beats::translate(FrameNum numFrames) {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return;
    }

    for (BeatList::iterator it = m_beats.begin();
            it != m_beats.end();) {
        double newpos = it->frame_position() + numFrames;
        if (newpos >= 0) {
            it->set_frame_position(newpos);
            ++it;
        } else {
            it = m_beats.erase(it);
        }
    }
    onBeatlistChanged();
    locker.unlock();
    emit updated();
}

void Beats::scale(enum BPMScale scale) {
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

void Beats::scaleDouble() {
    track::io::Beat prevBeat = m_beats.first();
    // Skip the first beat to preserve the first beat in a measure
    BeatList::iterator it = m_beats.begin() + 1;
    for (; it != m_beats.end(); ++it) {
        // Need to not accrue fractional frames.
        int distance = it->frame_position() - prevBeat.frame_position();
        track::io::Beat beat;
        beat.set_frame_position(prevBeat.frame_position() + distance / 2);
        it = m_beats.insert(it, beat);
        prevBeat = (++it)[0];
    }
}

void Beats::scaleTriple() {
    track::io::Beat prevBeat = m_beats.first();
    // Skip the first beat to preserve the first beat in a measure
    BeatList::iterator it = m_beats.begin() + 1;
    for (; it != m_beats.end(); ++it) {
        // Need to not accrue fractional frames.
        int distance = it->frame_position() - prevBeat.frame_position();
        track::io::Beat beat;
        beat.set_frame_position(prevBeat.frame_position() + distance / 3);
        it = m_beats.insert(it, beat);
        ++it;
        beat.set_frame_position(prevBeat.frame_position() + distance * 2 / 3);
        it = m_beats.insert(it, beat);
        prevBeat = (++it)[0];
    }
}

void Beats::scaleQuadruple() {
    track::io::Beat prevBeat = m_beats.first();
    // Skip the first beat to preserve the first beat in a measure
    BeatList::iterator it = m_beats.begin() + 1;
    for (; it != m_beats.end(); ++it) {
        // Need to not accrue fractional frames.
        int distance = it->frame_position() - prevBeat.frame_position();
        track::io::Beat beat;
        for (int i = 1; i <= 3; i++) {
            beat.set_frame_position(prevBeat.frame_position() + distance * i / 4);
            it = m_beats.insert(it, beat);
            ++it;
        }
        prevBeat = it[0];
    }
}

void Beats::scaleHalve() {
    // Skip the first beat to preserve the first beat in a measure
    BeatList::iterator it = m_beats.begin() + 1;
    for (; it != m_beats.end(); ++it) {
        it = m_beats.erase(it);
        if (it == m_beats.end()) {
            break;
        }
    }
}

void Beats::scaleThird() {
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

void Beats::scaleFourth() {
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

// TODO(JVC) If we use a Beatmap we can't just set the BPM
void Beats::setBpm(double dBpm) {
    Q_UNUSED(dBpm);
    DEBUG_ASSERT(!"Beats::setBpm() not implemented");
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

bool Beats::readByteArray(const QByteArray& byteArray) {
    track::io::BeatMap map;
    if (!map.ParseFromArray(byteArray.constData(), byteArray.size())) {
        qDebug() << "ERROR: Could not parse kBeatMap from QByteArray of size"
                 << byteArray.size();
        return false;
    }
    for (int i = 0; i < map.beat_size(); ++i) {
        const track::io::Beat& beat = map.beat(i);
        m_beats.append(beat);
    }
    onBeatlistChanged();
    return true;
}

FrameNum Beats::getFirstBeatPosition() const {
    return (m_beats.size() == 0) ? -1 : m_beats.front().frame_position();
}

FrameNum Beats::getLastBeatPosition() const {
    return (m_beats.size() == 0) ? -1 : m_beats.back().frame_position();
}

QDebug operator<<(QDebug dbg, const BeatsPointer& arg) {
    dbg << "Beats State\n";
    dbg << "\tm_subVersion:" << arg->m_subVersion << "\n";
    dbg << "\tm_iSampleRate:" << arg->m_iSampleRate << "\n";
    dbg << "\tm_dCachedBpm:" << arg->m_dCachedBpm << "\n";
    dbg << "\tm_dLastFrame:" << arg->m_dLastFrame << "\n";
    dbg << "Beats content(size: " << arg->m_beats.size() << ":\n";
    for (auto beat : arg->m_beats) {
        dbg << "pos:" << beat.frame_position() << "\n";
    }
    return dbg;
}

} // namespace mixxx
