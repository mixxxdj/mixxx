#include "track/beats.h"

#include "track/beatutils.h"
#include "track/track.h"

namespace mixxx {

const QString BeatsInternal::BEAT_MAP_VERSION = "BeatMap-1.0";
const QString BeatsInternal::BEAT_GRID_1_VERSION = "BeatGrid-1.0";
const QString BeatsInternal::BEAT_GRID_2_VERSION = "BeatGrid-2.0";
const QString BeatsInternal::BEATS_VERSION = "Beats-1.0";

namespace {
inline bool TimeSignatureMarkerEarlier(
        const track::io::TimeSignatureMarker& marker1,
        const track::io::TimeSignatureMarker& marker2) {
    return marker1.downbeat_index() < marker2.downbeat_index();
}
constexpr int kSecondsPerMinute = 60;
constexpr double kBeatVicinityFactor = 0.1;

inline FrameDiff_t getBeatLengthFrames(Bpm bpm,
        double sampleRate,
        TimeSignature timeSignature = TimeSignature()) {
    return kSecondsPerMinute * sampleRate *
            (4.0 / timeSignature.getNoteValue()) / bpm.getValue();
}
} // namespace

Beats::Beats(const Track* track,
        const QVector<FramePos>& beats,
        const QVector<track::io::TimeSignatureMarker>& timeSignatureMarkers)
        : Beats(track) {
    if (beats.size() > 0) {
        // This causes BeatsInternal constructor to be called twice.
        // But it can't be included in ctor initializer list since
        // we already have a delegating constructor.
        m_beatsInternal.initWithAnalyzer(beats, timeSignatureMarkers);
    }
    slotTrackBeatsUpdated();
}

Beats::Beats(const Track* track, const QByteArray& byteArray)
        : Beats(track) {
    m_beatsInternal.initWithProtobuf(byteArray);
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

Beat Beats::findNextBeat(FramePos frame) const {
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

Beat Beats::findPrevBeat(FramePos frame) const {
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

Beat Beats::findNthBeat(FramePos frame, int n) const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.findNthBeat(frame, n);
}

std::unique_ptr<Beats::iterator> Beats::findBeats(
        FramePos startFrame, FramePos stopFrame) const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.findBeats(startFrame, stopFrame);
}

Bpm Beats::getBpm() const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.getBpm();
}

Bpm Beats::getBpmAroundPosition(FramePos curFrame, int n) const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.getBpmAroundPosition(curFrame, n);
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

void Beats::setAsDownbeat(int beatIndex) {
    QMutexLocker locker(&m_mutex);
    m_beatsInternal.setAsDownbeat(beatIndex);
    locker.unlock();
    emit updated();
}

QDebug operator<<(QDebug dbg, const BeatsPointer& arg) {
    dbg << arg->m_beatsInternal;
    return dbg;
}

QDebug operator<<(QDebug dbg, const BeatsInternal& arg) {
    QVector<FramePos> beatFramePositions;
    for (const auto& beat : arg.m_beats) {
        beatFramePositions.append(beat.getFramePosition());
    }
    dbg << "["
        << "Cached BPM:" << arg.m_bpm << "|"
        << "Number of beats:" << arg.m_beats.size() << "|"
        << "Beats:" << beatFramePositions << "]";
    return dbg;
}
Beat BeatsInternal::findNthBeat(FramePos frame, int n) const {
    if (!isValid() || n == 0) {
        return kInvalidBeat;
    }

    Beat beat(frame);
    // it points at the first occurrence of beat or the next largest beat
    BeatList::const_iterator it =
            std::lower_bound(m_beats.cbegin(), m_beats.cend(), beat);

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
        FrameDiff_t delta = it->getFramePosition() - beat.getFramePosition();

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
            if (n == 1) {
                // Return a sample offset
                return *next_beat;
            }
            --n;
        }
    } else if (n < 0 && previous_beat != m_beats.end()) {
        for (; true; --previous_beat) {
            if (n == -1) {
                // Return a sample offset
                return *previous_beat;
            }
            ++n;

            // Don't step before the start of the list.
            if (previous_beat == m_beats.begin()) {
                break;
            }
        }
    }
    return kInvalidBeat;
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

void BeatsInternal::initWithProtobuf(const QByteArray& byteArray) {
    track::io::Beats beatsProto;
    if (!beatsProto.ParseFromArray(byteArray.constData(), byteArray.size())) {
        qDebug() << "ERROR: Could not parse Beats from QByteArray of size"
                 << byteArray.size();
    }
    m_beatsProto = beatsProto;
    generateBeatsFromMarkers();
}

void BeatsInternal::initWithAnalyzer(const QVector<FramePos>& beats,
        const QVector<track::io::TimeSignatureMarker>& timeSignatureMarkers) {
    m_beatsProto.set_first_frame_position(beats.at(0).getValue());
    int bpmMarkerBeatIndex = 0;
    for (int i = 1; i < beats.size(); ++i) {
        VERIFY_OR_DEBUG_ASSERT(
                beats.at(i) > beats.at(i - 1) && beats.at(i) >= FramePos(0)) {
            qDebug() << "Beats not in increasing order or negative, discarding "
                        "beat"
                     << beats.at(i);
        }
        else {
            FrameDiff_t beatLength = beats.at(i) - beats.at(i - 1);
            Bpm immediateBpm(kSecondsPerMinute * m_iSampleRate / beatLength);
            if (m_beatsProto.bpm_markers().empty() ||
                    m_beatsProto.bpm_markers().rbegin()->bpm() !=
                            immediateBpm.getValue()) {
                track::io::BpmMarker bpmMarker;
                bpmMarker.set_beat_index(bpmMarkerBeatIndex);
                bpmMarker.set_bpm(immediateBpm.getValue());
                m_beatsProto.add_bpm_markers()->CopyFrom(bpmMarker);
            }
            bpmMarkerBeatIndex++;
        }
    }

    for (const auto& timeSignatureMarker : timeSignatureMarkers) {
        m_beatsProto.add_time_signature_markers()->CopyFrom(
                timeSignatureMarker);
    }

    VERIFY_OR_DEBUG_ASSERT(m_beatsProto.time_signature_markers_size() > 0) {
        // If the analyzer does not send time signature information, just assume 4/4
        // for the whole track and the first beat as downbeat.
        track::io::TimeSignatureMarker timeSignatureMarker;
        m_beatsProto.add_time_signature_markers()->CopyFrom(
                timeSignatureMarker);
    }

    generateBeatsFromMarkers();
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
        lastCountedBeat = findNthBeat(startFrame, iBeatsCounter).getFramePosition();
        if (lastCountedBeat == kInvalidFramePos) {
            break;
        }
    }
    return iBeatsCounter - 2;
}

QByteArray BeatsInternal::toProtobuf() const {
    std::string output;
    m_beatsProto.SerializeToString(&output);
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
    generateBeatsFromMarkers();
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
    for (int i = 0; i < m_beatsProto.bpm_markers_size(); ++i) {
        track::io::BpmMarker oldBpmMarker = m_beatsProto.bpm_markers().Get(i);
        m_beatsProto.mutable_bpm_markers()->Mutable(i)->set_bpm(
                oldBpmMarker.bpm() * multiple);
        m_beatsProto.mutable_bpm_markers()->Mutable(i)->set_beat_index(
                oldBpmMarker.beat_index() * multiple);
    }
}

void BeatsInternal::scaleFraction(uint fraction) {
    for (int i = 0; i < m_beatsProto.bpm_markers_size(); ++i) {
        track::io::BpmMarker oldBpmMarker = m_beatsProto.bpm_markers().Get(i);
        m_beatsProto.mutable_bpm_markers()->Mutable(i)->set_bpm(
                oldBpmMarker.bpm() / fraction);
        m_beatsProto.mutable_bpm_markers()->Mutable(i)->set_beat_index(
                oldBpmMarker.beat_index() / fraction);
    }
}

void BeatsInternal::updateBpm() {
    if (!isValid()) {
        m_bpm = Bpm();
        return;
    }
    Beat startBeat = m_beats.first();
    Beat stopBeat = m_beats.last();
    m_bpm = calculateBpm(startBeat, stopBeat);
}
Bpm BeatsInternal::calculateBpm(
        const Beat& startBeat, const Beat& stopBeat) const {
    if (startBeat > stopBeat) {
        return Bpm();
    }

    BeatList::const_iterator curBeat =
            std::lower_bound(m_beats.cbegin(), m_beats.cend(), startBeat);

    BeatList::const_iterator lastBeat =
            std::upper_bound(m_beats.cbegin(), m_beats.cend(), stopBeat);

    QVector<double> beatvect;
    for (; curBeat != lastBeat; ++curBeat) {
        const Beat& beat = *curBeat;
        beatvect.append(beat.getFramePosition().getValue());
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
        nthBeat = findNthBeat(nextBeat, fullBeats).getFramePosition();
    } else {
        nthBeat = findNthBeat(prevBeat, fullBeats - 1).getFramePosition();
    }

    if (nthBeat == kInvalidFramePos) {
        return fromFrame;
    }

    // Add the fraction of the beat
    if (fractionBeats != 0) {
        nextBeat = findNthBeat(nthBeat, 2).getFramePosition();
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
    Beat beat(frame);

    // it points at the first occurrence of beat or the next largest beat
    BeatList::const_iterator it =
            std::lower_bound(m_beats.cbegin(), m_beats.cend(), beat);

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
        qint32 delta = it->getFramePosition() - beat.getFramePosition();

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
        pNextBeatFrame->setValue(next_beat->getFramePosition().getValue());
        break;
    }
    if (previous_beat != m_beats.end()) {
        for (; true; --previous_beat) {
            pPrevBeatFrame->setValue(
                    previous_beat->getFramePosition().getValue());
            break;

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
    clearMarkers();
    m_beatsProto.set_first_frame_position(firstBeatFrame.getValue());
    m_beatsProto.set_first_downbeat_index(0);
    track::io::BpmMarker bpmMarker;
    bpmMarker.set_beat_index(0);
    bpmMarker.set_bpm(dBpm.getValue());
    m_beatsProto.add_bpm_markers()->CopyFrom(bpmMarker);
    generateBeatsFromMarkers();
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

    Beat startBeat(startFrame), stopBeat(stopFrame);

    BeatList::const_iterator firstBeat =
            std::lower_bound(m_beats.cbegin(), m_beats.cend(), startBeat);

    BeatList::const_iterator lastBeat =
            std::upper_bound(m_beats.cbegin(), m_beats.cend(), stopBeat);
    if (lastBeat >= m_beats.cbegin()) {
        lastBeat = m_beats.cend() - 1;
    }

    if (firstBeat >= lastBeat) {
        return std::unique_ptr<BeatsInternal::iterator>();
    }
    return std::make_unique<BeatsInternal::iterator>(firstBeat, lastBeat + 1);
}
Beat BeatsInternal::findNextBeat(FramePos frame) const {
    return findNthBeat(frame, 1);
}
Beat BeatsInternal::findPrevBeat(FramePos frame) const {
    return findNthBeat(frame, -1);
}
Bpm BeatsInternal::getBpmAroundPosition(FramePos curFrame, int n) const {
    if (!isValid()) {
        return Bpm();
    }

    // To make sure we are always counting n beats, iterate backward to the
    // lower bound, then iterate forward from there to the upper bound.
    // kInvalidFramePos indicates we went off the map -- count from the beginning.
    FramePos lower_bound = findNthBeat(curFrame, -n).getFramePosition();
    if (lower_bound == kInvalidFramePos) {
        lower_bound = m_beats.first().getFramePosition();
    }

    // If we hit the end of the beat map, recalculate the lower bound.
    FramePos upper_bound = findNthBeat(lower_bound, n * 2).getFramePosition();
    if (upper_bound == kInvalidFramePos) {
        upper_bound = m_beats.last().getFramePosition();
        lower_bound = findNthBeat(upper_bound, n * -2).getFramePosition();
        // Super edge-case -- the track doesn't have n beats!  Do the best
        // we can.
        if (lower_bound == kInvalidFramePos) {
            lower_bound = m_beats.first().getFramePosition();
        }
    }

    // TODO(JVC) We are extracting frame numbers to then construct beats.
    // Then in calculateBpm we are using the frame position to find
    // the beats to  use them to calculate. Seems inefficient
    // Will not make more sense to extract the Beats straight?
    // We can use getBpmRange and move the logic of calculateBpm there
    Beat startBeat(lower_bound), stopBeat(upper_bound);
    return calculateBpm(startBeat, stopBeat);
}

void BeatsInternal::setSignature(TimeSignature sig, int downbeatIndex) {
    if (!isValid()) {
        return;
    }
    track::io::TimeSignatureMarker markerToInsert;
    markerToInsert.set_downbeat_index(downbeatIndex);
    markerToInsert.mutable_time_signature()->set_beats_per_bar(
            sig.getBeatsPerBar());
    markerToInsert.mutable_time_signature()->set_note_value(sig.getNoteValue());
    QVector<track::io::TimeSignatureMarker> timeSignatureMarkersMutableCopy;
    copy(m_beatsProto.time_signature_markers().cbegin(),
            m_beatsProto.time_signature_markers().cend(),
            std::back_inserter(timeSignatureMarkersMutableCopy));

    track::io::TimeSignatureMarker searchBeforeMarker;
    searchBeforeMarker.set_downbeat_index(downbeatIndex);
    auto prevTimeSignatureMarker =
            std::lower_bound(timeSignatureMarkersMutableCopy.begin(),
                    timeSignatureMarkersMutableCopy.end(),
                    searchBeforeMarker,
                    TimeSignatureMarkerEarlier);
    if (prevTimeSignatureMarker->downbeat_index() == downbeatIndex) {
        prevTimeSignatureMarker->CopyFrom(markerToInsert);
    } else {
        timeSignatureMarkersMutableCopy.insert(
                prevTimeSignatureMarker, markerToInsert);
    }
    m_beatsProto.clear_time_signature_markers();
    for (const auto& timeSignatureMarker : timeSignatureMarkersMutableCopy) {
        m_beatsProto.add_time_signature_markers()->CopyFrom(
                timeSignatureMarker);
    }
    generateBeatsFromMarkers();
}

void BeatsInternal::translate(FrameDiff_t numFrames) {
    if (!isValid()) {
        return;
    }

    m_beatsProto.set_first_frame_position(
            m_beatsProto.first_frame_position() + numFrames);
    generateBeatsFromMarkers();
}

void BeatsInternal::setBpm(Bpm dBpm) {
    // Temporarily creating this adapter to generate beats from a given bpm assuming
    // uniform bpm.
    // TODO(hacksdump): A check for preferences will be added to only allow setting bpm
    //  when "Assume Constant Tempo" is checked.
    setGrid(dBpm, m_beats.first().getFramePosition());

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
    return m_beats.empty() ? kInvalidFramePos
                           : m_beats.front().getFramePosition();
}

FramePos BeatsInternal::getLastBeatPosition() const {
    return m_beats.empty() ? kInvalidFramePos
                           : m_beats.back().getFramePosition();
}
void BeatsInternal::generateBeatsFromMarkers() {
    VERIFY_OR_DEBUG_ASSERT(m_beatsProto.time_signature_markers_size() > 0) {
        // This marker will get the default values from the protobuf definitions,
        // beatIndex = 0 and timeSignature = 4/4.
        track::io::TimeSignatureMarker generatedTimeSignatureMarker;
        m_beatsProto.add_time_signature_markers()->CopyFrom(
                generatedTimeSignatureMarker);
    }

    VERIFY_OR_DEBUG_ASSERT(m_beatsProto.first_downbeat_index() <
            m_beatsProto.time_signature_markers()
                    .Get(0)
                    .time_signature()
                    .beats_per_bar()) {
        // Bring the offset within the limits of number of beats in bar.
        int currentDownbeatOffset = m_beatsProto.first_downbeat_index();
        int beatsPerBarAtStart = m_beatsProto.time_signature_markers()
                                         .Get(0)
                                         .time_signature()
                                         .beats_per_bar();
        int reducedDownbeatOffset = currentDownbeatOffset % beatsPerBarAtStart;
        m_beatsProto.set_first_downbeat_index(reducedDownbeatOffset);
    }

    // Clear redundant markers
    QList<track::io::TimeSignatureMarker> minimalTimeSignatureMarkers;
    for (const auto& timeSignatureMarker :
            m_beatsProto.time_signature_markers()) {
        if (minimalTimeSignatureMarkers.empty() ||
                TimeSignature(minimalTimeSignatureMarkers.constLast()
                                      .time_signature()) !=
                        TimeSignature(timeSignatureMarker.time_signature())) {
            minimalTimeSignatureMarkers.append(timeSignatureMarker);
        }
    }
    m_beatsProto.clear_time_signature_markers();
    for (const auto& minimalTimeSignatureMarker : minimalTimeSignatureMarkers) {
        m_beatsProto.add_time_signature_markers()->CopyFrom(
                minimalTimeSignatureMarker);
    }

    m_beats.clear();

    const FramePos trackLastFrame(m_iSampleRate * m_dDurationSeconds);
    int bpmMarkerIndex = 0;
    int timeSignatureMarkerIndex = 0;
    int barIndex = -1;
    int beatsPerBar = m_beatsProto.time_signature_markers()
                              .Get(0)
                              .time_signature()
                              .beats_per_bar();
    int barRelativeBeatIndex = (beatsPerBar -
                                       m_beatsProto.first_downbeat_index()) %
            beatsPerBar;
    Beat addedBeat(kInvalidFramePos);
    // TODO(hacksdump): Use markers for BPM and enable marker only on user edited markers.
    bool beatHasMarker;
    while (true) {
        beatHasMarker = false;
        auto currentBpmMarker = m_beatsProto.bpm_markers().Get(bpmMarkerIndex);
        Bpm currentBpm = Bpm(currentBpmMarker.bpm());
        auto currentTimeSignatureMarker =
                m_beatsProto.time_signature_markers().Get(
                        timeSignatureMarkerIndex);
        TimeSignature currentTimeSignature =
                TimeSignature(currentTimeSignatureMarker.time_signature());
        FrameDiff_t beatLength = getBeatLengthFrames(
                currentBpm, m_iSampleRate, currentTimeSignature);

        if (barRelativeBeatIndex % currentTimeSignature.getBeatsPerBar() == 0) {
            barIndex++;
            if (timeSignatureMarkerIndex <
                            m_beatsProto.time_signature_markers_size() - 1 &&
                    m_beatsProto.time_signature_markers()
                                    .Get(timeSignatureMarkerIndex + 1)
                                    .downbeat_index() == barIndex) {
                timeSignatureMarkerIndex++;
                currentTimeSignatureMarker =
                        m_beatsProto.time_signature_markers().Get(
                                timeSignatureMarkerIndex);
                currentTimeSignature = TimeSignature(
                        currentTimeSignatureMarker.time_signature());
                beatHasMarker = true;
            }
        }

        Beat::Type beatType =
                barRelativeBeatIndex % currentTimeSignature.getBeatsPerBar() ==
                        0
                ? Beat::DOWNBEAT
                : Beat::BEAT;
        if (bpmMarkerIndex < m_beatsProto.bpm_markers_size() - 1 &&
                m_beatsProto.bpm_markers()
                                .Get(bpmMarkerIndex + 1)
                                .beat_index() == m_beats.size()) {
            bpmMarkerIndex++;
        }
        FramePos beatFramePosition = m_beats.empty()
                ? FramePos(m_beatsProto.first_frame_position())
                : (m_beats.last().getFramePosition() + beatLength);
        addedBeat = Beat(beatFramePosition,
                beatType,
                currentTimeSignature,
                m_beats.size(),
                barIndex,
                barRelativeBeatIndex,
                beatHasMarker);
        barRelativeBeatIndex = (barRelativeBeatIndex + 1) %
                currentTimeSignature.getBeatsPerBar();
        if (addedBeat.getFramePosition() <= trackLastFrame) {
            m_beats.append(addedBeat);
        } else {
            break;
        }
    }
    updateBpm();
}
void BeatsInternal::clearMarkers() {
    m_beatsProto.clear_first_frame_position();
    m_beatsProto.clear_first_downbeat_index();
    m_beatsProto.clear_bpm_markers();
    m_beatsProto.clear_time_signature_markers();
}

void BeatsInternal::setAsDownbeat(int beatIndex) {
    auto beat = getBeatAtIndex(beatIndex);
    m_beatsProto.set_first_downbeat_index(m_beatsProto.first_downbeat_index() +
            beat.getBarRelativeBeatIndex());
    generateBeatsFromMarkers();
}

} // namespace mixxx
