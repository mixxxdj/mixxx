#include "track/beats.h"

#include "track/beatutils.h"
#include "track/track.h"
#include "util/math.h"

namespace mixxx {

const QString BeatsInternal::kBeatMapVersion = "BeatMap-1.0";
const QString BeatsInternal::kBeatGridVersion1 = "BeatGrid-1.0";
const QString BeatsInternal::kBeatGridVersion2 = "BeatGrid-2.0";
const QString BeatsInternal::kBeatsVersion = "Beats-1.0";

namespace {
inline bool TimeSignatureMarkerEarlier(
        const track::io::TimeSignatureMarker& marker1,
        const track::io::TimeSignatureMarker& marker2) {
    return marker1.downbeat_index() < marker2.downbeat_index();
}
inline bool BpmMarkerEarlier(
        const track::io::BpmMarker& marker1,
        const track::io::BpmMarker& marker2) {
    return marker1.beat_index() < marker2.beat_index();
}
constexpr int kSecondsPerMinute = 60;
constexpr double kBeatVicinityFactor = 0.1;

inline FrameDiff_t getBeatLengthFrames(Bpm bpm,
        double sampleRate,
        TimeSignature timeSignature = TimeSignature()) {
    return kSecondsPerMinute * sampleRate *
            (4.0 / timeSignature.getNoteValue()) / bpm.getValue();
}

inline FrameDiff_t durationToFrames(audio::SampleRate sampleRate, double seconds) {
    return sampleRate * seconds;
}
} // namespace

void Beats::initWithAnalyzer(const QVector<FramePos>& beats,
        const QVector<track::io::TimeSignatureMarker>& timeSignatureMarkers) {
    QMutexLocker locker(&m_mutex);
    m_beatsInternal.initWithAnalyzer(beats, timeSignatureMarkers);
    emit updated();
}

void Beats::initWithProtobuf(const QByteArray& byteArray) {
    QMutexLocker locker(&m_mutex);
    m_beatsInternal.initWithProtobuf(byteArray);
    emit updated();
}

Beats::Beats(const BeatsInternal& internal)
        : m_mutex(QMutex::Recursive), m_beatsInternal(internal) {
}

int Beats::numBeatsInRange(FramePos startFrame, FramePos endFrame) const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.numBeatsInRange(startFrame, endFrame);
};

QByteArray Beats::toProtobuf() const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.toProtobuf();
}

QString Beats::getVersion() const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.getVersion();
}

QString Beats::getSubVersion() const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.getSubVersion();
}

std::optional<Beat> Beats::findNextBeat(FramePos frame) const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.findNextBeat(frame);
}

void Beats::setSubVersion(const QString& subVersion) {
    QMutexLocker locker(&m_mutex);
    m_beatsInternal.setSubVersion(subVersion);
}

void Beats::setGrid(Bpm bpm, FramePos firstBeatFrame) {
    QMutexLocker lock(&m_mutex);
    m_beatsInternal.setGrid(bpm, firstBeatFrame);
    emit updated();
}

FramePos Beats::findNBeatsFromFrame(FramePos fromFrame, double beats) const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.findNBeatsFromFrame(fromFrame, beats);
};

std::optional<Beat> Beats::findPrevBeat(FramePos frame) const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.findPrevBeat(frame);
}

QPair<std::optional<Beat>, std::optional<Beat>> Beats::findPrevNextBeats(FramePos frame) const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.findPrevNextBeats(frame);
}

FramePos Beats::findClosestBeat(FramePos frame) const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.findClosestBeat(frame);
}

std::optional<Beat> Beats::findNthBeat(FramePos frame, int offset) const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.findNthBeat(frame, offset);
}

std::optional<Beat> Beats::getBeatAtIndex(int index) const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.getBeatAtIndex(index);
}

Bpm Beats::getGlobalBpm() const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.getGlobalBpm();
}

Bpm Beats::getBpmAtPosition(FramePos curFrame) const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.getBpmAtPosition(curFrame);
}

void Beats::setSignature(TimeSignature sig, int beatIndex) {
    QMutexLocker locker(&m_mutex);
    m_beatsInternal.setSignature(sig, beatIndex);
    locker.unlock();
    emit updated();
}

void Beats::translateBySeconds(double seconds) {
    QMutexLocker locker(&m_mutex);
    m_beatsInternal.translateBySeconds(seconds);
    locker.unlock();
    emit updated();
}

void Beats::scale(BPMScale scale) {
    QMutexLocker locker(&m_mutex);
    m_beatsInternal.scale(scale);
    locker.unlock();
    emit updated();
}

void Beats::setBpm(Bpm bpm, int beatIndex) {
    QMutexLocker locker(&m_mutex);
    m_beatsInternal.setBpm(bpm, beatIndex);
    locker.unlock();
    emit updated();
}

int Beats::size() const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.size();
}

FramePos Beats::getFirstBeatPosition() const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.getFirstBeatPosition();
}

FramePos Beats::getLastBeatPosition() const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal.getLastBeatPosition();
}

std::optional<Beat> BeatsInternal::getBeatAtIndex(int index) const {
    if (!isValid()) {
        return std::nullopt;
    }

    if (index >= 0 && index < m_beats.size()) {
        return m_beats.at(index);
    }

    const int distanceFromFirstDownbeat = index - m_beatsProto.first_downbeat_index();
    if (index < 0) {
        const auto& firstBeat = m_beats.first();
        const int beatsPerBar = firstBeat.timeSignature().getBeatsPerBar();
        const FrameDiff_t beatLength = getBeatLengthFrames(
                firstBeat.bpm(), getSampleRate(), firstBeat.timeSignature());
        const int beatInBarIndex = clockModulo(
                distanceFromFirstDownbeat, beatsPerBar);
        const int barIndex =
                std::floor(static_cast<double>(distanceFromFirstDownbeat) /
                        beatsPerBar);
        Beat generatedPseudoBeat(
                firstBeat.framePosition() + index * beatLength,
                (beatInBarIndex == 0) ? BeatType::Downbeat : BeatType::Beat,
                firstBeat.timeSignature(),
                firstBeat.bpm(),
                index,
                barIndex,
                beatInBarIndex);
        return generatedPseudoBeat;
    }

    // index >= m_beats.size()
    const auto& lastBeat = m_beats.last();
    const int beatsPerBar = lastBeat.timeSignature().getBeatsPerBar();
    const FrameDiff_t beatLength = getBeatLengthFrames(
            lastBeat.bpm(), getSampleRate(), lastBeat.timeSignature());
    const int beatInBarIndex = clockModulo(
            distanceFromFirstDownbeat, beatsPerBar);
    const int barIndex =
            std::floor(static_cast<double>(distanceFromFirstDownbeat) /
                    beatsPerBar);
    Beat generatedPseudoBeat(
            lastBeat.framePosition() + (index - lastBeat.beatIndex()) * beatLength,
            (beatInBarIndex == 0) ? BeatType::Downbeat : BeatType::Beat,
            lastBeat.timeSignature(),
            lastBeat.bpm(),
            index,
            barIndex,
            beatInBarIndex);
    return generatedPseudoBeat;
}

void Beats::setAsDownbeat(int beatIndex) {
    QMutexLocker locker(&m_mutex);
    m_beatsInternal.setAsDownbeat(beatIndex);
    locker.unlock();
    emit updated();
}

BeatsInternal Beats::getInternal() const {
    QMutexLocker locker(&m_mutex);
    return m_beatsInternal;
}

QDebug operator<<(QDebug dbg, const BeatsPointer& arg) {
    QMutexLocker locker(&arg->m_mutex);
    dbg << arg->m_beatsInternal;
    return dbg;
}

QDebug operator<<(QDebug dbg, const BeatsInternal& arg) {
    QVector<FramePos> beatFramePositions;
    for (const auto& beat : arg.m_beats) {
        beatFramePositions.append(beat.framePosition());
    }
    dbg << "["
        << "Global BPM:" << arg.m_bpm << "|"
        << "Number of beats:" << arg.m_beats.size() << "|"
        << "Beats:" << beatFramePositions << "]";
    return dbg;
}

std::optional<Beat> BeatsInternal::findNthBeat(FramePos frame, int offset) const {
    if (!isValid() || offset == 0) {
        return std::nullopt;
    }

    // We will divide beats into three zones.
    // First: Real beats within the track
    // Second: Pseudo beats before the track
    // Third: Pseudo beats after the track

    if (getFirstBeatPosition() <= frame && frame <= getLastBeatPosition()) {
        Beat beat(frame);
        // it points at the first occurrence of beat or the next largest beat
        BeatList::const_iterator it =
                std::lower_bound(m_beats.cbegin(), m_beats.cend(), beat);

        // Back-up by one.
        if (it != m_beats.begin()) {
            --it;
        }

        // Scan forward to find whether we are on a beat.
        BeatList::const_iterator on_beat = m_beats.cend();
        BeatList::const_iterator previous_beat = m_beats.cend();
        BeatList::const_iterator next_beat = m_beats.cend();
        for (; it != m_beats.end(); ++it) {
            FrameDiff_t delta = it->framePosition() - beat.framePosition();
            // If the position is within a fraction of the local beat length,
            // pretend we are on that beat.
            const double kFrameEpsilon = kBeatVicinityFactor *
                    getBeatLengthFrames(
                            it->bpm(), getSampleRate(), it->timeSignature());

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

        if (offset > 0) {
            return getBeatAtIndex(next_beat->beatIndex() + offset - 1);
        } else { // n < 0
            return getBeatAtIndex(previous_beat->beatIndex() + offset + 1);
        }
    } else if (frame < getFirstBeatPosition()) {
        const auto& firstBeat = getBeatAtIndex(0);
        const FrameDiff_t beatLength = getBeatLengthFrames(firstBeat->bpm(),
                getSampleRate(),
                firstBeat->timeSignature());
        double beatFraction = (frame - getFirstBeatPosition()) / beatLength;
        int prevBeatIdx = floor(beatFraction);
        int nextBeatIdx = ceil(beatFraction);

        if (fabs(nextBeatIdx - beatFraction) < kBeatVicinityFactor) {
            // If we are going to pretend we were actually on nextBeat then prevBeat
            // needs to be re-calculated. Since it is floor(beatFraction), that's
            // the same as nextBeat.  We only use prevBeat so no need to increment
            // nextBeat.
            prevBeatIdx = nextBeatIdx;
        } else if (fabs(prevBeatIdx - beatFraction) < kBeatVicinityFactor) {
            // If we are going to pretend we were actually on prevBeat then nextBeat
            // needs to be re-calculated. Since it is ceil(beatFraction), that's
            // the same as prevBeat.  We will only use nextBeat so no need to
            // decrement prevBeat.
            nextBeatIdx = prevBeatIdx;
        }

        if (offset > 0) {
            return getBeatAtIndex(nextBeatIdx + offset - 1);
        } else { // n < 0
            return getBeatAtIndex(prevBeatIdx + offset + 1);
        }
    } else { // frame > getLastFramePosition()
        const int lastBeatIdx = size() - 1;
        const auto& lastBeat = getBeatAtIndex(lastBeatIdx);
        const FrameDiff_t beatLength = getBeatLengthFrames(
                lastBeat->bpm(), getSampleRate(), lastBeat->timeSignature());
        double beatFraction = (frame - getLastBeatPosition()) / beatLength;
        int prevBeatIdxRelativeToLast = floor(beatFraction);
        int nextBeatIdxRelativeToLast = ceil(beatFraction);

        if (fabs(nextBeatIdxRelativeToLast - beatFraction) < kBeatVicinityFactor) {
            prevBeatIdxRelativeToLast = nextBeatIdxRelativeToLast;
        } else if (fabs(prevBeatIdxRelativeToLast - beatFraction) < kBeatVicinityFactor) {
            nextBeatIdxRelativeToLast = prevBeatIdxRelativeToLast;
        }

        if (offset > 0) {
            return getBeatAtIndex(nextBeatIdxRelativeToLast + lastBeatIdx + offset - 1);
        } else { // n < 0
            return getBeatAtIndex(prevBeatIdxRelativeToLast + lastBeatIdx + offset + 1);
        }
    }
}

Bpm BeatsInternal::getGlobalBpm() const {
    if (!isValid()) {
        return Bpm();
    }
    return m_bpm;
}

bool BeatsInternal::isValid() const {
    return getSampleRate() > 0 && getDurationSeconds() > 0 && m_beatsProto.bpm_markers_size() > 0;
}

void BeatsInternal::updateStreamInfo(const mixxx::audio::StreamInfo& streamInfo) {
    if (m_streamInfo != streamInfo) {
        m_streamInfo = streamInfo;
        generateBeatsFromMarkers();
    }
}

BeatsInternal::BeatsInternal(const audio::StreamInfo& streamInfo) {
    updateStreamInfo(streamInfo);
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
    DEBUG_ASSERT(getSampleRate() > 0 && getDurationSeconds() > 0);
    clear();

    if (beats.empty()) {
        return;
    }

    auto lastBeatTime = Duration::fromSeconds(beats.last().getValue() / getSampleRate());
    DEBUG_ASSERT(m_streamInfo.getDuration() >= lastBeatTime);

    setFirstBeatFrame(beats.at(0));
    int bpmMarkerBeatIndex = 0;
    for (int i = 1; i < beats.size(); ++i) {
        VERIFY_OR_DEBUG_ASSERT(
                beats.at(i) > beats.at(i - 1) && beats.at(i) >= kStartFramePos) {
            qDebug() << "Beats not in increasing order or negative, discarding "
                        "beat"
                     << beats.at(i);
        }
        else {
            FrameDiff_t beatLength = beats.at(i) - beats.at(i - 1);
            // TODO(hacksdump): Take time signature (note length) into account while calculating BPM.
            Bpm immediateBpm(kSecondsPerMinute * getSampleRate() / beatLength);
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

    if (m_beatsProto.time_signature_markers_size() == 0) {
        // If the analyzer does not send time signature information, just assume 4/4
        // for the whole track and the first beat as downbeat.
        track::io::TimeSignatureMarker timeSignatureMarker;
        m_beatsProto.add_time_signature_markers()->CopyFrom(
                timeSignatureMarker);
    }

    generateBeatsFromMarkers();

    // The number of generated beats should not be less than the input beats.
    // They can be more since they can be extrapolated with the last bpm marker.
    DEBUG_ASSERT(beats.size() <= m_beats.size());

    // Check whether the generated beats match the input beats.
    for (int i = 0; i < beats.size(); ++i) {
        DEBUG_ASSERT(qFuzzyCompare(beats.at(i).getValue(),
                m_beats.at(i).framePosition().getValue()));
    }
}

void Beats::updateStreamInfo(const mixxx::audio::StreamInfo& streamInfo) {
    QMutexLocker locker(&m_mutex);
    m_beatsInternal.updateStreamInfo(streamInfo);
    emit updated();
}

void Beats::clear() {
    QMutexLocker locker(&m_mutex);
    m_beatsInternal.clear();
    emit updated();
}

int BeatsInternal::numBeatsInRange(
        FramePos startFrame, FramePos endFrame) const {
    FramePos lastCountedBeat(0.0);
    int iBeatsCounter;
    for (iBeatsCounter = 1; lastCountedBeat < endFrame; iBeatsCounter++) {
        lastCountedBeat = findNthBeat(startFrame, iBeatsCounter)->framePosition();
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
    return kBeatsVersion;
}

QString BeatsInternal::getSubVersion() const {
    return m_subVersion;
}

void BeatsInternal::scale(BPMScale scale) {
    if (!isValid()) {
        return;
    }

    switch (scale) {
    case BPMScale::Double:
        // introduce a new beat into every gap
        scaleDouble();
        break;
    case BPMScale::Halve:
        // remove every second beat
        scaleHalve();
        break;
    case BPMScale::TwoThirds:
        // introduce a new beat into every gap
        scaleDouble();
        // remove every second and third beat
        scaleThird();
        break;
    case BPMScale::ThreeFourths:
        // introduce two beats into every gap
        scaleTriple();
        // remove every second third and forth beat
        scaleFourth();
        break;
    case BPMScale::FourThirds:
        // introduce three beats into every gap
        scaleQuadruple();
        // remove every second third and forth beat
        scaleThird();
        break;
    case BPMScale::ThreeHalves:
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

void BeatsInternal::updateGlobalBpm() {
    if (!isValid()) {
        m_bpm = Bpm();
        return;
    }
    // If we only have one BPM value, there is no need to find mode BPM.
    // Just use the BPM value in protobuf.
    if (m_beatsProto.bpm_markers_size() == 1) {
        m_bpm = Bpm(m_beatsProto.bpm_markers().cbegin()->bpm());
    } else {
        QMap<double, double> bpmDuration;
        for (int i = 0; i < m_beats.size() - 1; i++) {
            const auto currentBpm = m_beats[i].bpm().getValue();
            const auto currentFramePosition = m_beats[i].framePosition();
            const auto nextFramePosition = m_beats[i + 1].framePosition();
            const auto duration = nextFramePosition - currentFramePosition;
            if (bpmDuration.contains(currentBpm)) {
                bpmDuration[currentBpm] += duration;
            } else {
                bpmDuration[currentBpm] = duration;
            }
        }
        const double modeBpm = std::max_element(bpmDuration.begin(),
                bpmDuration.end(),
                [](const double m1, const double m2) {
                    return m1 < m2;
                }).key();
        m_bpm = Bpm(modeBpm);
    }
}

FramePos BeatsInternal::findNBeatsFromFrame(
        FramePos fromFrame, double beats) const {
    FramePos nthBeat;

    const auto prevNextBeats = findPrevNextBeats(fromFrame);
    if (!prevNextBeats.first || !prevNextBeats.second) {
        return fromFrame;
    }

    auto prevBeat = prevNextBeats.first->framePosition();
    auto nextBeat = prevNextBeats.second->framePosition();
    double fromFractionBeats = (fromFrame - prevBeat) / (nextBeat - prevBeat);
    double beatsFromPrevBeat = fromFractionBeats + beats;

    int fullBeats = static_cast<int>(beatsFromPrevBeat);
    double fractionBeats = beatsFromPrevBeat - fullBeats;

    if (fullBeats > 0) {
        nthBeat = findNthBeat(nextBeat, fullBeats)->framePosition();
    } else {
        nthBeat = findNthBeat(prevBeat, fullBeats - 1)->framePosition();
    }

    if (nthBeat == kInvalidFramePos) {
        return fromFrame;
    }

    // Add the fraction of the beat
    if (fractionBeats != 0) {
        nextBeat = findNthBeat(nthBeat, 2)->framePosition();
        if (nextBeat == kInvalidFramePos) {
            return fromFrame;
        }
        nthBeat += (nextBeat - nthBeat) * fractionBeats;
    }

    return nthBeat;
}

QPair<std::optional<Beat>, std::optional<Beat>>
BeatsInternal::findPrevNextBeats(FramePos frame) const {
    if (!isValid()) {
        return qMakePair(std::nullopt, std::nullopt);
    }
    const auto& prevBeat = findPrevBeat(frame);
    QPair<std::optional<Beat>, std::optional<Beat>> prevNextBeats;
    prevNextBeats.first = prevBeat;
    prevNextBeats.second = getBeatAtIndex(prevBeat->beatIndex() + 1);
    return prevNextBeats;
}

void BeatsInternal::setGrid(Bpm dBpm, FramePos firstBeatFrame) {
    m_beatsProto.clear_bpm_markers();
    setFirstBeatFrame(firstBeatFrame);
    track::io::BpmMarker bpmMarker;
    bpmMarker.set_bpm(dBpm.getValue());
    m_beatsProto.add_bpm_markers()->CopyFrom(bpmMarker);
    generateBeatsFromMarkers();
}

FramePos BeatsInternal::findClosestBeat(FramePos frame) const {
    if (!isValid()) {
        return kInvalidFramePos;
    }
    const auto prevNextBeats = findPrevNextBeats(frame);
    if (!prevNextBeats.first) {
        // If both values are invalid, we correctly return kInvalidFramePos.
        return prevNextBeats.second->framePosition();
    } else if (!prevNextBeats.second) {
        return prevNextBeats.first->framePosition();
    }
    return (prevNextBeats.second->framePosition() - frame >
                   frame - prevNextBeats.first->framePosition())
            ? prevNextBeats.first->framePosition()
            : prevNextBeats.second->framePosition();
}

std::optional<Beat> BeatsInternal::findNextBeat(FramePos frame) const {
    return findNthBeat(frame, 1);
}

std::optional<Beat> BeatsInternal::findPrevBeat(FramePos frame) const {
    return findNthBeat(frame, -1);
}

Bpm BeatsInternal::getBpmAtPosition(FramePos curFrame) const {
    if (!isValid()) {
        return Bpm();
    }
    if (curFrame < getFirstBeatFrame()) {
        return m_beats.first().bpm();
    }
    return findPrevBeat(curFrame)->bpm();
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

void BeatsInternal::translateBySeconds(double seconds) {
    if (!isValid()) {
        return;
    }

    setFirstBeatFrame(getFirstBeatFrame() +
            durationToFrames(
                    m_streamInfo.getSignalInfo().getSampleRate(), seconds));
    generateBeatsFromMarkers();
}

void BeatsInternal::setBpm(Bpm bpm, int beatIndex) {
    if (!isValid()) {
        return;
    }
    track::io::BpmMarker markerToInsert;
    markerToInsert.set_beat_index(beatIndex);
    markerToInsert.set_bpm(bpm.getValue());
    QVector<track::io::BpmMarker> bpmMarkersMutableCopy;
    copy(m_beatsProto.bpm_markers().cbegin(),
            m_beatsProto.bpm_markers().cend(),
            std::back_inserter(bpmMarkersMutableCopy));

    track::io::BpmMarker searchBeforeMarker;
    searchBeforeMarker.set_beat_index(beatIndex);
    auto prevBpmMarker =
            std::lower_bound(bpmMarkersMutableCopy.begin(),
                    bpmMarkersMutableCopy.end(),
                    searchBeforeMarker,
                    BpmMarkerEarlier);
    if (prevBpmMarker->beat_index() == beatIndex) {
        prevBpmMarker->CopyFrom(markerToInsert);
    } else {
        bpmMarkersMutableCopy.insert(
                prevBpmMarker, markerToInsert);
    }
    m_beatsProto.clear_bpm_markers();
    for (const auto& bpmMarker : bpmMarkersMutableCopy) {
        m_beatsProto.add_bpm_markers()->CopyFrom(
                bpmMarker);
    }
    generateBeatsFromMarkers();
}

int BeatsInternal::size() const {
    return m_beats.size();
}

FramePos BeatsInternal::getFirstBeatPosition() const {
    return m_beats.empty() ? kInvalidFramePos
                           : m_beats.front().framePosition();
}

FramePos BeatsInternal::getLastBeatPosition() const {
    return m_beats.empty() ? kInvalidFramePos
                           : m_beats.back().framePosition();
}

void BeatsInternal::generateBeatsFromMarkers() {
    m_beats.clear();
    if (getSampleRate() <= 0 || getDurationSeconds() <= 0) {
        return;
    }
    // Absence of BPM markers is irrecoverable.
    if (m_beatsProto.bpm_markers_size() == 0) {
        return;
    }
    // If the protobuf does not have any time signature markers, we add the default
    // time signature marker.
    if (m_beatsProto.time_signature_markers_size() == 0) {
        // This marker will get the default values from the protobuf definitions,
        // beatIndex = 0 and timeSignature = 4/4.
        track::io::TimeSignatureMarker generatedTimeSignatureMarker;
        m_beatsProto.add_time_signature_markers()->CopyFrom(
                generatedTimeSignatureMarker);
    }

    consolidateMarkers();

    // We keep generating beats until this frame.
    const FramePos trackLastFrame(getSampleRate() * getDurationSeconds());
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
    BeatMarkers markers;
    while (true) {
        markers = BeatMarker::None;
        Bpm bpmBeforeThisBeat = Bpm(m_beatsProto.bpm_markers().Get(bpmMarkerIndex).bpm());
        if (bpmMarkerIndex < m_beatsProto.bpm_markers_size() - 1 &&
                m_beatsProto.bpm_markers()
                                .Get(bpmMarkerIndex + 1)
                                .beat_index() == m_beats.size()) {
            bpmMarkerIndex++;
        }

        auto currentBpmMarker = m_beatsProto.bpm_markers(bpmMarkerIndex);

        if (currentBpmMarker.beat_index() == m_beats.size()) {
            markers |= BeatMarker::Bpm;
        }
        auto currentTimeSignatureMarker =
                m_beatsProto.time_signature_markers().Get(
                        timeSignatureMarkerIndex);
        TimeSignature currentTimeSignature =
                TimeSignature(currentTimeSignatureMarker.time_signature());
        FrameDiff_t beatLength = getBeatLengthFrames(
                bpmBeforeThisBeat, getSampleRate(), currentTimeSignature);

        if (barRelativeBeatIndex % currentTimeSignature.getBeatsPerBar() == 0) {
            barIndex++;
            if (m_beatsProto.first_downbeat_index() == m_beats.size()) {
                markers |= BeatMarker::TimeSignature;
            }
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
                markers |= BeatMarker::TimeSignature;
            }
        }

        BeatType beatType =
                barRelativeBeatIndex % currentTimeSignature.getBeatsPerBar() ==
                        0
                ? BeatType::Downbeat
                : BeatType::Beat;

        FramePos beatFramePosition = m_beats.empty()
                ? FramePos(getFirstBeatFrame())
                : (m_beats.last().framePosition() + beatLength);
        Beat addedBeat = Beat(beatFramePosition,
                beatType,
                currentTimeSignature,
                Bpm(currentBpmMarker.bpm()),
                m_beats.size(),
                barIndex,
                barRelativeBeatIndex,
                markers);
        barRelativeBeatIndex = (barRelativeBeatIndex + 1) %
                currentTimeSignature.getBeatsPerBar();
        if (addedBeat.framePosition() <= trackLastFrame) {
            m_beats.append(addedBeat);
        } else {
            break;
        }
    }
    updateGlobalBpm();
}

void BeatsInternal::consolidateMarkers() {
    // The initial downbeat offset should always be less than the number
    // of beats in the first measure.
    if (m_beatsProto.first_downbeat_index() >=
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

    QList<track::io::BpmMarker> minimalBpmMarkers;
    for (const auto& bpmMarker :
            m_beatsProto.bpm_markers()) {
        if (minimalBpmMarkers.empty() ||
                !qFuzzyCompare(minimalBpmMarkers.constLast().bpm(),
                        bpmMarker.bpm())) {
            minimalBpmMarkers.append(bpmMarker);
        }
    }
    m_beatsProto.clear_bpm_markers();
    for (const auto& minimalBpmMarker : minimalBpmMarkers) {
        m_beatsProto.add_bpm_markers()->CopyFrom(
                minimalBpmMarker);
    }

    // Very small variations in BPM values due to floating point precision
    // should be cleared out.
    QMap<double, bool> visitedBpm;
    for (int i = 0; i < m_beatsProto.bpm_markers_size(); i++) {
        const double referenceBpm = m_beatsProto.bpm_markers(i).bpm();
        if (!visitedBpm[referenceBpm]) {
            for (int j = i + 1; j < m_beatsProto.bpm_markers_size(); j++) {
                const double currentBpm = m_beatsProto.bpm_markers(j).bpm();
                if (qFuzzyCompare(currentBpm, referenceBpm)) {
                    m_beatsProto.mutable_bpm_markers(j)->set_bpm(referenceBpm);
                }
            }
            visitedBpm[referenceBpm] = true;
        }
    }
}

void BeatsInternal::clear() {
    m_beatsProto.clear_first_beat_time_seconds();
    m_beatsProto.clear_first_downbeat_index();
    m_beatsProto.clear_bpm_markers();
    m_beatsProto.clear_time_signature_markers();
    m_beats.clear();
}

void BeatsInternal::setAsDownbeat(int beatIndex) {
    auto beat = getBeatAtIndex(beatIndex);
    m_beatsProto.set_first_downbeat_index(m_beatsProto.first_downbeat_index() +
            beat->beatInBarIndex());
    generateBeatsFromMarkers();
}

SINT BeatsInternal::getSampleRate() const {
    return m_streamInfo.getSignalInfo().getSampleRate();
}

double BeatsInternal::getDurationSeconds() const {
    return m_streamInfo.getDuration().toDoubleSeconds();
}

void BeatsInternal::setFirstBeatFrame(FramePos framePos) {
    m_beatsProto.set_first_beat_time_seconds(framePos.getValue() / getSampleRate());
}

FramePos BeatsInternal::getFirstBeatFrame() const {
    return FramePos(m_beatsProto.first_beat_time_seconds() * getSampleRate());
}

} // namespace mixxx
