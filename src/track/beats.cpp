#include "track/beats.h"

#include "track/beatutils.h"
#include "track/track.h"

namespace mixxx {

const QString BeatsInternal::BEAT_MAP_VERSION = "BeatMap-1.0";
const QString BeatsInternal::BEAT_GRID_1_VERSION = "BeatGrid-1.0";
const QString BeatsInternal::BEAT_GRID_2_VERSION = "BeatGrid-2.0";
const QString BeatsInternal::BEATS_VERSION = "Beats-1.0";

namespace {
inline bool BeatLessThan(const track::io::Beat& beat1, const track::io::Beat& beat2) {
    return beat1.frame_position() < beat2.frame_position();
}
inline bool TimeSignatureMarkerEarlier(
        const track::io::TimeSignatureMarker& marker1,
        const track::io::TimeSignatureMarker& marker2) {
    return marker1.beat_index() < marker2.beat_index();
}
constexpr int kSecondsPerMinute = 60;
constexpr double kBeatVicinityFactor = 0.1;

inline FrameDiff_t getBeatLengthFrames(Bpm bpm, double sampleRate) {
    return kSecondsPerMinute * sampleRate / bpm.getValue();
}
} // namespace

Beats::Beats(const Track* track,
        const QVector<FramePos>& beats,
        const QVector<track::io::TimeSignatureMarker>& timeSignatureMarkers,
        const QVector<track::io::PhraseMarker>& phraseMarkers,
        const QVector<track::io::SectionMarker>& sectionMarkers)
        : Beats(track) {
    if (beats.size() > 0) {
        // This causes BeatsInternal constructor to be called twice.
        // But it can't be included in ctor initializer list since
        // we already have a delegating constructor.
        m_beatsInternal = BeatsInternal(
                beats, timeSignatureMarkers, phraseMarkers, sectionMarkers);
    }
    slotTrackBeatsUpdated();
}

Beats::Beats(const Track* track, const QByteArray& byteArray)
        : Beats(track) {
    m_beatsInternal = BeatsInternal(byteArray);
    slotTrackBeatsUpdated();
}

Beats::Beats(const Track* track)
        : m_mutex(QMutex::Recursive), m_track(track) {
    // BeatMap should live in the same thread as the track it is associated
    // with.
    slotTrackBeatsUpdated();
    connect(m_track, &Track::beatsUpdated, this, &Beats::slotTrackBeatsUpdated);
    connect(m_track, &Track::changed, this, &Beats::slotTrackBeatsUpdated);
    moveToThread(track->thread());
}

Beats::Beats(const Beats& other)
        : Beats(other.m_track) {
    m_beatsInternal = other.m_beatsInternal;
}

int Beats::numBeatsInRange(FramePos startFrame, FramePos endFrame) {
    return m_beatsInternal.numBeatsInRange(startFrame, endFrame);
};

QByteArray Beats::toProtobuf() const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.toProtobuf();
}

BeatsPointer Beats::clone() const {
    QMutexLocker locker(&m_mutex);
    BeatsPointer other(new Beats(*this));
    return other;
}

QString Beats::getVersion() const {
    return m_beatsInternal.getVersion();
}

QString Beats::getSubVersion() const {
    return m_beatsInternal.getSubVersion();
}

FramePos Beats::findNextBeat(FramePos frame) const {
    return m_beatsInternal.findNextBeat(frame);
}

void Beats::setSubVersion(const QString& subVersion) {
    m_beatsInternal.setSubVersion(subVersion);
}

void Beats::setGrid(Bpm bpm, FramePos firstBeatFrame) {
    QMutexLocker lock(&m_mutex);
    m_beatsInternal.setGrid(bpm, firstBeatFrame);
}

FramePos Beats::findNBeatsFromFrame(FramePos fromFrame, double beats) const {
    return m_beatsInternal.findNBeatsFromFrame(fromFrame, beats);
};

bool Beats::isValid() const {
    return m_beatsInternal.isValid();
}

Bpm Beats::calculateBpm(const track::io::Beat& startBeat,
        const track::io::Beat& stopBeat) const {
    return m_beatsInternal.calculateBpm(startBeat, stopBeat);
}

FramePos Beats::findPrevBeat(FramePos frame) const {
    return m_beatsInternal.findPrevBeat(frame);
}

bool Beats::findPrevNextBeats(FramePos frame,
        FramePos* pPrevBeatFrame,
        FramePos* pNextBeatFrame) const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.findPrevNextBeats(
            frame, pPrevBeatFrame, pNextBeatFrame);
}

FramePos Beats::findClosestBeat(FramePos frame) const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.findClosestBeat(frame);
}

FramePos Beats::findNthBeat(FramePos frame, int n) const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.findNthBeat(frame, n);
}

std::unique_ptr<Beats::iterator> Beats::findBeats(
        FramePos startFrame, FramePos stopFrame) const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.findBeats(startFrame, stopFrame);
}

bool Beats::hasBeatInRange(FramePos startFrame, FramePos stopFrame) const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.hasBeatInRange(startFrame, stopFrame);
}

Bpm Beats::getBpm() const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.getBpm();
}

double Beats::getBpmRange(FramePos startFrame, FramePos stopFrame) const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.getBpmRange(startFrame, stopFrame);
}

Bpm Beats::getBpmAroundPosition(FramePos curFrame, int n) const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.getBpmAroundPosition(curFrame, n);
}

TimeSignature Beats::getSignature(int beatIndex) const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.getSignature(beatIndex);
}

void Beats::setSignature(TimeSignature sig, int beatIndex) {
    QMutexLocker locker(&m_mutex);
    m_beatsInternal.setSignature(sig, beatIndex);
    locker.unlock();
    emit(updated());
}

void Beats::translate(FrameDiff_t numFrames) {
    QMutexLocker locker(&m_mutex);
    m_beatsInternal.translate(numFrames);
    locker.unlock();
    emit updated();
}

void Beats::scale(enum BeatsInternal::BPMScale scale) {
    QMutexLocker locker(&m_mutex);
    m_beatsInternal.scale(scale);
    locker.unlock();
    emit updated();
}

void Beats::setBpm(Bpm bpm) {
    m_beatsInternal.setBpm(bpm);
}

FramePos Beats::getFirstBeatPosition() const {
    return m_beatsInternal.getFirstBeatPosition();
}

FramePos Beats::getLastBeatPosition() const {
    return m_beatsInternal.getLastBeatPosition();
}

SINT Beats::getSampleRate() const {
    return m_track->getSampleRate();
}

QDebug operator<<(QDebug dbg, const BeatsPointer& arg) {
    dbg << arg->m_beatsInternal;
    return dbg;
}

QDebug operator<<(QDebug dbg, const BeatsInternal& arg) {
    QVector<FramePos> beatFramePositions;
    for (const auto& beat : arg.m_beats) {
        beatFramePositions.append(FramePos(beat.frame_position()));
    }
    dbg << "["
        << "Cached BPM:" << arg.m_bpm << "|"
        << "Number of beats:" << arg.m_beats.size() << "|"
        << "Beats:" << beatFramePositions << "]";
    return dbg;
}
FramePos BeatsInternal::findNthBeat(FramePos frame, int n) const {
    if (!isValid() || n == 0) {
        return kInvalidFramePos;
    }

    track::io::Beat beat;
    beat.set_frame_position(frame.getValue());

    // it points at the first occurrence of beat or the next largest beat
    BeatList::const_iterator it = std::lower_bound(
            m_beats.cbegin(), m_beats.cend(), beat, BeatLessThan);

    // If the position is within 1/10th of the average beat length,
    // pretend we are on that beat.
    const double kFrameEpsilon =
            kBeatVicinityFactor * getBeatLengthFrames(getBpm(), m_iSampleRate);

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
                return FramePos(next_beat->frame_position());
            }
            --n;
        }
    } else if (n < 0 && previous_beat != m_beats.end()) {
        for (; true; --previous_beat) {
            if (previous_beat->enabled()) {
                if (n == -1) {
                    // Return a sample offset
                    return FramePos(previous_beat->frame_position());
                }
                ++n;
            }

            // Don't step before the start of the list.
            if (previous_beat == m_beats.begin()) {
                break;
            }
        }
    }
    return kInvalidFramePos;
}
Bpm BeatsInternal::getBpm() const {
    if (!isValid()) {
        return Bpm();
    }
    return m_bpm;
}
bool BeatsInternal::isValid() const {
    return m_iSampleRate > 0 && !m_beats.empty();
}
void BeatsInternal::setSampleRate(int sampleRate) {
    m_iSampleRate = sampleRate;
    updateBpm();
}
BeatsInternal::BeatsInternal()
        : m_iSampleRate(0), m_dDurationSeconds(0) {
}

BeatsInternal::BeatsInternal(const QByteArray& byteArray) {
    track::io::Beats beatsProto;
    if (!beatsProto.ParseFromArray(byteArray.constData(), byteArray.size())) {
        qDebug() << "ERROR: Could not parse Beats from QByteArray of size"
                 << byteArray.size();
    }
    for (int i = 0; i < beatsProto.beat_size(); ++i) {
        const track::io::Beat& beat = beatsProto.beat(i);
        m_beats.append(beat);
    }

    VERIFY_OR_DEBUG_ASSERT(beatsProto.time_signature_markers_size() > 0) {
        // This marker will get the default values from the protobuf definitions,
        // beatIndex = 0 and timeSignature = 4/4.
        track::io::TimeSignatureMarker generatedTimeSignatureMarker;
        beatsProto.add_time_signature_markers()->CopyFrom(
                generatedTimeSignatureMarker);
    }
    for (int i = 0; i < beatsProto.time_signature_markers_size(); ++i) {
        const track::io::TimeSignatureMarker& timeSignatureMarker =
                beatsProto.time_signature_markers(i);
        m_timeSignatureMarkers.append(timeSignatureMarker);
    }
    updateBpm();
}

BeatsInternal::BeatsInternal(const QVector<FramePos>& beats,
        const QVector<track::io::TimeSignatureMarker>& timeSignatureMarkers,
        const QVector<track::io::PhraseMarker>& phraseMarkers,
        const QVector<track::io::SectionMarker>& sectionMarkers) {
    Q_UNUSED(phraseMarkers);
    Q_UNUSED(sectionMarkers);
    FramePos previousBeatPos = kInvalidFramePos;
    track::io::Beat protoBeat;

    for (const auto& beat : beats) {
        VERIFY_OR_DEBUG_ASSERT(beat > previousBeatPos && beat >= FramePos(0)) {
            qDebug() << "Beats not in increasing order or negative, discarding beat" << beat;
        }
        else {
            protoBeat.set_frame_position(beat.getValue());
            m_beats.append(protoBeat);
            previousBeatPos = beat;
        }
    }

    VERIFY_OR_DEBUG_ASSERT(timeSignatureMarkers.size() > 0) {
        // If the analyzer does not send time signature information, just assume 4/4
        // for the whole track and the first beat as downbeat.
        setSignature(TimeSignature(), 0);
    }
    else {
        m_timeSignatureMarkers = timeSignatureMarkers;
    }

    updateBpm();
}

void Beats::slotTrackBeatsUpdated() {
    m_beatsInternal.setSampleRate(m_track->getSampleRate());
    m_beatsInternal.setDurationSeconds(m_track->getDuration());
}

int BeatsInternal::numBeatsInRange(
        FramePos startFrame, FramePos endFrame) const {
    FramePos lastCountedBeat(0.0);
    int iBeatsCounter;
    for (iBeatsCounter = 1; lastCountedBeat < endFrame; iBeatsCounter++) {
        lastCountedBeat = findNthBeat(startFrame, iBeatsCounter);
        if (lastCountedBeat == kInvalidFramePos) {
            break;
        }
    }
    return iBeatsCounter - 2;
}

QByteArray BeatsInternal::toProtobuf() const {
    // No guarantees BeatLists are made of a data type which located adjacent
    // items in adjacent memory locations.
    track::io::Beats beatsProto;

    for (const auto& beat : m_beats) {
        beatsProto.add_beat()->CopyFrom(beat);
    }

    for (const auto& timeSignatureMarker : m_timeSignatureMarkers) {
        beatsProto.add_time_signature_markers()->CopyFrom(
                timeSignatureMarker);
    }

    std::string output;
    beatsProto.SerializeToString(&output);
    return QByteArray(output.data(), output.length());
}

void BeatsInternal::setSubVersion(const QString& subVersion) {
    m_subVersion = subVersion;
}
QString BeatsInternal::getVersion() const {
    return BEATS_VERSION;
}
QString BeatsInternal::getSubVersion() const {
    return m_subVersion;
}
void BeatsInternal::scale(BeatsInternal::BPMScale scale) {
    if (!isValid()) {
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
    updateBpm();
}

void BeatsInternal::scaleDouble() {
    scaleMultiple(2);
}

void BeatsInternal::scaleTriple() {
    scaleMultiple(3);
}

void BeatsInternal::scaleQuadruple() {
    scaleMultiple(4);
}

void BeatsInternal::scaleHalve() {
    scaleFraction(2);
}

void BeatsInternal::scaleThird() {
    scaleFraction(3);
}

void BeatsInternal::scaleFourth() {
    scaleFraction(4);
}

void BeatsInternal::scaleMultiple(uint multiple) {
    track::io::Beat prevBeat = m_beats.first();
    // Skip the first beat to preserve the first beat in a measure
    BeatList::iterator it = m_beats.begin() + 1;
    for (; it != m_beats.end(); ++it) {
        // Need to not accrue fractional frames.
        int distance = it->frame_position() - prevBeat.frame_position();
        track::io::Beat beat;
        for (uint i = 1; i <= multiple - 1; i++) {
            beat.set_frame_position(
                    prevBeat.frame_position() + distance * i / multiple);
            it = m_beats.insert(it, beat);
            ++it;
        }
        prevBeat = it[0];
    }
}

void BeatsInternal::scaleFraction(uint fraction) {
    // Skip the first beat to preserve the first beat in a measure
    BeatList::iterator it = m_beats.begin() + 1;
    for (; it != m_beats.end(); ++it) {
        for (uint i = 1; i <= fraction - 1; i++) {
            it = m_beats.erase(it);
            if (it == m_beats.end()) {
                return;
            }
        }
    }
}

void BeatsInternal::updateBpm() {
    if (!isValid()) {
        m_bpm = Bpm();
        return;
    }
    track::io::Beat startBeat = m_beats.first();
    track::io::Beat stopBeat = m_beats.last();
    m_bpm = calculateBpm(startBeat, stopBeat);
}
Bpm BeatsInternal::calculateBpm(const track::io::Beat& startBeat,
        const track::io::Beat& stopBeat) const {
    if (startBeat.frame_position() > stopBeat.frame_position()) {
        return Bpm();
    }

    BeatList::const_iterator curBeat = std::lower_bound(
            m_beats.cbegin(), m_beats.cend(), startBeat, BeatLessThan);

    BeatList::const_iterator lastBeat = std::upper_bound(
            m_beats.cbegin(), m_beats.cend(), stopBeat, BeatLessThan);

    QVector<double> beatvect;
    for (; curBeat != lastBeat; ++curBeat) {
        const track::io::Beat& beat = *curBeat;
        if (beat.enabled()) {
            beatvect.append(beat.frame_position());
        }
    }

    if (beatvect.isEmpty()) {
        return Bpm();
    }

    return BeatUtils::calculateBpm(beatvect, m_iSampleRate, 0, 9999);
}
FramePos BeatsInternal::findNBeatsFromFrame(
        FramePos fromFrame, double beats) const {
    FramePos nthBeat;
    FramePos prevBeat;
    FramePos nextBeat;

    if (!findPrevNextBeats(fromFrame, &prevBeat, &nextBeat)) {
        return fromFrame;
    }
    double fromFractionBeats = (fromFrame - prevBeat) / (nextBeat - prevBeat);
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

    if (nthBeat == kInvalidFramePos) {
        return fromFrame;
    }

    // Add the fraction of the beat
    if (fractionBeats != 0) {
        nextBeat = findNthBeat(nthBeat, 2);
        if (nextBeat == kInvalidFramePos) {
            return fromFrame;
        }
        nthBeat += (nextBeat - nthBeat) * fractionBeats;
    }

    return nthBeat;
}
bool BeatsInternal::findPrevNextBeats(FramePos frame,
        FramePos* pPrevBeatFrame,
        FramePos* pNextBeatFrame) const {
    if (pPrevBeatFrame == nullptr || pNextBeatFrame == nullptr) {
        return false;
    }

    if (!isValid()) {
        *pPrevBeatFrame = kInvalidFramePos;
        *pNextBeatFrame = kInvalidFramePos;
        return false;
    }
    track::io::Beat beat;
    beat.set_frame_position(frame.getValue());

    // it points at the first occurrence of beat or the next largest beat
    BeatList::const_iterator it = std::lower_bound(
            m_beats.cbegin(), m_beats.cend(), beat, BeatLessThan);

    // If the position is within 1/10th of the average beat length,
    // pretend we are on that beat.
    const double kFrameEpsilon =
            kBeatVicinityFactor * getBeatLengthFrames(getBpm(), m_iSampleRate);

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

    *pPrevBeatFrame = kInvalidFramePos;
    *pNextBeatFrame = kInvalidFramePos;

    for (; next_beat != m_beats.end(); ++next_beat) {
        if (!next_beat->enabled()) {
            continue;
        }
        pNextBeatFrame->setValue(next_beat->frame_position());
        break;
    }
    if (previous_beat != m_beats.end()) {
        for (; true; --previous_beat) {
            if (previous_beat->enabled()) {
                pPrevBeatFrame->setValue(previous_beat->frame_position());
                break;
            }

            // Don't step before the start of the list.
            if (previous_beat == m_beats.begin()) {
                break;
            }
        }
    }
    return *pPrevBeatFrame != kInvalidFramePos &&
            *pNextBeatFrame != kInvalidFramePos;
}
void BeatsInternal::setGrid(Bpm dBpm, FramePos firstBeatFrame) {
    FramePos trackLastFrame = FramePos(m_dDurationSeconds * m_iSampleRate);
    m_beats.clear();
    track::io::Beat beat;
    beat.set_frame_position(firstBeatFrame.getValue());
    for (FramePos frame = firstBeatFrame; frame <= trackLastFrame;
            frame += getBeatLengthFrames(dBpm, m_iSampleRate)) {
        beat.set_frame_position(frame.getValue());
        m_beats.push_back(beat);
    }

    updateBpm();
}
FramePos BeatsInternal::findClosestBeat(FramePos frame) const {
    if (!isValid()) {
        return kInvalidFramePos;
    }
    FramePos prevBeat;
    FramePos nextBeat;
    findPrevNextBeats(frame, &prevBeat, &nextBeat);
    if (prevBeat == kInvalidFramePos) {
        // If both values are invalid, we correctly return kInvalidFramePos.
        return nextBeat;
    } else if (nextBeat == kInvalidFramePos) {
        return prevBeat;
    }
    return (nextBeat - frame > frame - prevBeat) ? prevBeat : nextBeat;
}

std::unique_ptr<BeatsInternal::iterator> BeatsInternal::findBeats(
        FramePos startFrame, FramePos stopFrame) const {
    if (!isValid() || startFrame > stopFrame) {
        return std::unique_ptr<BeatsInternal::iterator>();
    }

    track::io::Beat startBeat, stopBeat;
    startBeat.set_frame_position(startFrame.getValue());
    stopBeat.set_frame_position(stopFrame.getValue());

    BeatList::const_iterator firstBeat = std::lower_bound(
            m_beats.cbegin(), m_beats.cend(), startBeat, BeatLessThan);

    BeatList::const_iterator lastBeat = std::upper_bound(
            m_beats.cbegin(), m_beats.cend(), stopBeat, BeatLessThan);
    if (lastBeat >= m_beats.cbegin()) {
        lastBeat = m_beats.cend() - 1;
    }

    if (firstBeat >= lastBeat) {
        return std::unique_ptr<BeatsInternal::iterator>();
    }
    return std::make_unique<BeatsInternal::iterator>(firstBeat, lastBeat + 1);
}
bool BeatsInternal::hasBeatInRange(
        FramePos startFrame, FramePos stopFrame) const {
    if (!isValid() || startFrame > stopFrame) {
        return false;
    }
    FramePos curBeat = findNextBeat(startFrame);
    if (curBeat <= stopFrame) {
        return true;
    }
    return false;
}
FramePos BeatsInternal::findNextBeat(FramePos frame) const {
    return findNthBeat(frame, 1);
}
FramePos BeatsInternal::findPrevBeat(FramePos frame) const {
    return findNthBeat(frame, -1);
}
double BeatsInternal::getBpmRange(
        FramePos startFrame, FramePos stopFrame) const {
    if (!isValid()) {
        return -1;
    }
    track::io::Beat startBeat, stopBeat;
    startBeat.set_frame_position(startFrame.getValue());
    stopBeat.set_frame_position(stopFrame.getValue());
    return calculateBpm(startBeat, stopBeat).getValue();
}
Bpm BeatsInternal::getBpmAroundPosition(FramePos curFrame, int n) const {
    if (!isValid()) {
        return Bpm();
    }

    // To make sure we are always counting n beats, iterate backward to the
    // lower bound, then iterate forward from there to the upper bound.
    // kInvalidFramePos indicates we went off the map -- count from the beginning.
    FramePos lower_bound = findNthBeat(curFrame, -n);
    if (lower_bound == kInvalidFramePos) {
        lower_bound = FramePos(m_beats.first().frame_position());
    }

    // If we hit the end of the beat map, recalculate the lower bound.
    FramePos upper_bound = findNthBeat(lower_bound, n * 2);
    if (upper_bound == kInvalidFramePos) {
        upper_bound = FramePos(m_beats.last().frame_position());
        lower_bound = findNthBeat(upper_bound, n * -2);
        // Super edge-case -- the track doesn't have n beats!  Do the best
        // we can.
        if (lower_bound == kInvalidFramePos) {
            lower_bound = FramePos(m_beats.first().frame_position());
        }
    }

    // TODO(JVC) We are extracting frame numbers to then construct beats.
    // Then in calculateBpm we are using the frame position to find
    // the beats to  use them to calculate. Seems inefficient
    // Will not make more sense to extract the Beats straight?
    // We can use getBpmRange and move the logic of calculateBpm there
    track::io::Beat startBeat, stopBeat;
    startBeat.set_frame_position(lower_bound.getValue());
    stopBeat.set_frame_position(upper_bound.getValue());
    return calculateBpm(startBeat, stopBeat);
}

TimeSignature BeatsInternal::getSignature(int beatIndex) const {
    if (!isValid()) {
        return kNullTimeSignature;
    }

    if (beatIndex == -1) {
        return kNullTimeSignature;
    }

    // Find the time signature marker just before (or on) this frame
    // TODO: This is linear search. Optimize to binary search.
    TimeSignature timeSignature;
    for (const auto& timeSignatureMarker : m_timeSignatureMarkers) {
        if (timeSignatureMarker.beat_index() <= beatIndex) {
            timeSignature = TimeSignature(timeSignatureMarker.time_signature());
        } else {
            break;
        }
    }
    return timeSignature;
}

void BeatsInternal::setSignature(
        const TimeSignature& signature, int beatIndex) {
    // TODO(hacksdump): Handle note values other than 1/4.
    if (!isValid()) {
        return;
    }

    // Signature can only be set on a downbeat. Check if beatIndex falls on a downbeat.
    // Allow normal flow if there are no time signature markers before beatIndex.
    if (!isDownbeat(beatIndex) && hasTimeSignatureMarkerBefore(beatIndex)) {
        return;
    }
    track::io::TimeSignatureMarker markerToInsert;
    markerToInsert.set_beat_index(beatIndex);
    markerToInsert.mutable_time_signature()->set_beats_per_bar(
            signature.getBeatsPerBar());
    markerToInsert.mutable_time_signature()->set_note_value(
            signature.getNoteValue());
    auto prevTimeSignatureMarker =
            std::lower_bound(m_timeSignatureMarkers.begin(),
                    m_timeSignatureMarkers.end(),
                    markerToInsert,
                    TimeSignatureMarkerEarlier);
    m_timeSignatureMarkers.insert(prevTimeSignatureMarker, markerToInsert);
}

bool BeatsInternal::isDownbeat(int beatIndex) {
    TimeSignature timeSignatureBefore;
    int timeSignatureBeforeMarkerIndex = 0;
    for (const auto& timeSignatureMarker : m_timeSignatureMarkers) {
        if (timeSignatureMarker.beat_index() < beatIndex) {
            timeSignatureBefore =
                    TimeSignature(timeSignatureMarker.time_signature());
            timeSignatureBeforeMarkerIndex = timeSignatureMarker.beat_index();
        } else {
            break;
        }
    }
    int beatCountFromPreviousTimeSignatureMarker =
            beatIndex - timeSignatureBeforeMarkerIndex;
    return beatCountFromPreviousTimeSignatureMarker %
            timeSignatureBefore.getBeatsPerBar() ==
            0;
}

bool BeatsInternal::hasTimeSignatureMarkerBefore(int beatIndex) {
    track::io::TimeSignatureMarker searchBeforeMarker;
    searchBeforeMarker.set_beat_index(beatIndex);
    auto prevTimeSignatureMarker =
            std::lower_bound(m_timeSignatureMarkers.cbegin(),
                    m_timeSignatureMarkers.cend(),
                    searchBeforeMarker,
                    TimeSignatureMarkerEarlier);
    return prevTimeSignatureMarker != m_timeSignatureMarkers.cend();
}

void BeatsInternal::translate(FrameDiff_t numFrames) {
    if (!isValid()) {
        return;
    }

    for (BeatList::iterator it = m_beats.begin(); it != m_beats.end();) {
        FramePos newpos = FramePos(it->frame_position()) + numFrames;
        it->set_frame_position(newpos.getValue());
        ++it;
    }
    updateBpm();
}

void BeatsInternal::setBpm(Bpm dBpm) {
    // Temporarily creating this adapter to generate beats from a given bpm assuming
    // uniform bpm.
    // TODO(hacksdump): A check for preferences will be added to only allow setting bpm
    //  when "Assume Constant Tempo" is checked.
    setGrid(dBpm, FramePos(m_beats.first().frame_position()));

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

FramePos BeatsInternal::getFirstBeatPosition() const {
    return m_beats.empty() ? kInvalidFramePos : FramePos(m_beats.front().frame_position());
}

FramePos BeatsInternal::getLastBeatPosition() const {
    return m_beats.empty() ? kInvalidFramePos : FramePos(m_beats.back().frame_position());
}

} // namespace mixxx
