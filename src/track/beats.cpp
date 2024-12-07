#include "track/beats.h"

#include <cmath>
#include <iterator>
#include <optional>
#include <unordered_map>
#include <vector>

#include "audio/frame.h"
#include "proto/beats.pb.h"
#include "track/beats.h"
#include "track/beatutils.h"
#include "track/bpm.h"
#include "util/assert.h"

namespace {

struct BeatGridV1Data {
    double bpm;
    double firstBeat;
};

constexpr double kEpsilon = 0.01;
constexpr mixxx::audio::BeatsPerBar kDefaultBeatsPerBar = mixxx::audio::BeatsPerBar(4);

// The amount that `Beats::tryAdjustTempo()` changes the last marker's BPM by.
constexpr double kBpmAdjustStep = 0.01;
constexpr double kBpmLargeAdjustModifier = 10;

int calculateBeatsTillNextMarker(const mixxx::Beats::ConstIterator& it,
        mixxx::audio::FramePos nextMarkerPosition) {
    const double numBeatsDouble = std::fabs(nextMarkerPosition - *it) / it.beatLengthFrames();
    const int numBeatsInt = static_cast<int>(std::round(numBeatsDouble));

    return numBeatsInt;
}

bool isBeatMarkerLessThanFramePos(const mixxx::BeatMarker& marker,
        const mixxx::audio::FramePos& position) {
    return marker.position() < position;
}

bool isFramePosLessThanBeatMarker(const mixxx::audio::FramePos& position,
        const mixxx::BeatMarker& marker) {
    return position < marker.position();
}

} // namespace

namespace mixxx {

int Beats::ConstIterator::beatsPerBar() const {
    if (m_it == m_beats->m_markers.cend()) {
        return m_beats->lastBeatsPerBar();
    }

    return m_it->beatsPerBar();
}

mixxx::audio::FrameDiff_t Beats::ConstIterator::beatLengthFrames() const {
    if (m_it == m_beats->m_markers.cend()) {
        return m_beats->lastBeatLengthFrames();
    }
    return m_it->beatsLength();
}

Beats::ConstIterator Beats::ConstIterator::operator+=(Beats::ConstIterator::difference_type n) {
    if (n == 0) {
        return *this;
    }

    if (n < 0) {
        static_assert(-1 -
                        std::numeric_limits<
                                Beats::ConstIterator::difference_type>::max() ==
                std::numeric_limits<
                        Beats::ConstIterator::difference_type>::min());
        // Super edge case: -INT_MIN == -(-INT_MIN) because it would be larger than INT_MAX
        if (n == std::numeric_limits<Beats::ConstIterator::difference_type>::min()) {
            *this -= std::numeric_limits<Beats::ConstIterator::difference_type>::max();
            *this -= 1;
        } else {
            *this -= (-n);
        }
        return *this;
    }

    DEBUG_ASSERT(n > 0);
#ifdef MIXXX_DEBUG_ASSERTIONS_ENABLED
    const auto origValue = m_value;
#endif

    // Detect integer overflow in `m_beatOffset + n`
    const int maxBeatOffset = std::numeric_limits<Beats::ConstIterator::difference_type>::max();
    if (m_beatOffset > maxBeatOffset - n) {
        qDebug() << "Beats: Iterator" << m_beatOffset << "+" << n
                 << "would go out of possible range, capping at latest possible position.";
        m_it = m_beats->m_markers.cend();
        m_beatOffset = maxBeatOffset;
        updateValue();
        DEBUG_ASSERT(m_value >= origValue);
        return *this;
    }

    m_beatOffset += n;
    while (m_it != m_beats->m_markers.cend()) {
        const auto nextMarker = std::next(m_it);
        const mixxx::audio::FramePos nextMarkerPosition =
                (nextMarker != m_beats->m_markers.cend())
                ? nextMarker->position()
                : m_beats->m_lastMarkerPosition;
        const auto beatsTillNextMarker = m_it->upperBoundBeatsTillNextMarker(nextMarkerPosition);
        if (m_beatOffset < beatsTillNextMarker) {
            break;
        }
        m_beatOffset -= beatsTillNextMarker;
        m_it++;
    }
    updateValue();
    DEBUG_ASSERT(m_value > origValue);
    return *this;
}

Beats::ConstIterator Beats::ConstIterator::operator-=(Beats::ConstIterator::difference_type n) {
    if (n == 0) {
        return *this;
    }

    if (n < 0) {
        static_assert(-1 -
                        std::numeric_limits<
                                Beats::ConstIterator::difference_type>::max() ==
                std::numeric_limits<
                        Beats::ConstIterator::difference_type>::min());
        // Super edge case: -INT_MIN == -(-INT_MIN) because it would be larger than INT_MAX
        if (n == std::numeric_limits<Beats::ConstIterator::difference_type>::min()) {
            *this += std::numeric_limits<Beats::ConstIterator::difference_type>::max();
            *this += 1;
        } else {
            *this += (-n);
        }
        return *this;
    }

    DEBUG_ASSERT(n > 0);
#ifdef MIXXX_DEBUG_ASSERTIONS_ENABLED
    const auto origValue = m_value;
#endif

    // Detect integer overflow
    const int minBeatOffset = std::numeric_limits<Beats::ConstIterator::difference_type>::lowest();
    if (m_beatOffset < minBeatOffset + n) {
        qDebug() << "Beats Iterator" << m_beatOffset << "-" << n
                 << "would go out of possible range, capping at earliest possible position.";
        m_it = m_beats->m_markers.cbegin();
        m_beatOffset = minBeatOffset;
        updateValue();
        DEBUG_ASSERT(m_value <= origValue);
        return *this;
    }

    m_beatOffset -= n;
    while (m_it != m_beats->m_markers.cbegin() && m_beatOffset < 0) {
        const mixxx::audio::FramePos currentMarkerPosition =
                (m_it != m_beats->m_markers.cend())
                ? m_it->position()
                : m_beats->m_lastMarkerPosition;
        m_it--;
        m_beatOffset += m_it->upperBoundBeatsTillNextMarker(currentMarkerPosition);
    }
    updateValue();
    DEBUG_ASSERT(m_value < origValue);
    return *this;
}

Beats::ConstIterator::difference_type Beats::ConstIterator::operator-(
        const Beats::ConstIterator& other) const {
    const int vectorItDiff = m_it - other.m_it;
    if (vectorItDiff == 0) {
        return m_beatOffset - other.m_beatOffset;
    }

    if (vectorItDiff > 0) {
        return -(other - *this);
    }

    auto it = m_it;
    int result = -m_beatOffset + other.m_beatOffset;
    while (it != m_beats->m_markers.cend() && it != other.m_it) {
        const auto nextMarker = std::next(it);
        const mixxx::audio::FramePos nextMarkerPosition =
                (nextMarker != m_beats->m_markers.cend())
                ? nextMarker->position()
                : m_beats->m_lastMarkerPosition;
        result += it->upperBoundBeatsTillNextMarker(nextMarkerPosition);
        it++;
    }
    DEBUG_ASSERT(it == other.m_it);
    return -result;
}

void Beats::ConstIterator::updateValue() {
    m_value = (m_it != m_beats->m_markers.cend())
            ? m_it->position() + m_beatOffset * m_it->beatsLength()
            : m_beats->m_lastMarkerPosition + m_beatOffset * m_beats->lastBeatLengthFrames();
}

// static
mixxx::BeatsPointer Beats::fromConstTempo(
        mixxx::audio::SampleRate sampleRate,
        mixxx::audio::FramePos lastMarkerPosition,
        mixxx::Bpm lastMarkerBpm,
        const QString& subVersion) {
    VERIFY_OR_DEBUG_ASSERT(sampleRate.isValid() &&
            lastMarkerPosition.isValid() && lastMarkerBpm.isValid()) {
        return nullptr;
    }
    return BeatsPointer(new Beats({},
            lastMarkerPosition,
            lastMarkerBpm,
            kDefaultBeatsPerBar,
            sampleRate,
            subVersion));
}

bool isDoubleBeat(const QVector<audio::FramePos>& beatPositions, int beatIdx) {
    if (beatIdx <= 0 || beatIdx > beatPositions.size()) {
        return false; // failed bounds check
    }
    return beatPositions[beatIdx - 1] == beatPositions[beatIdx];
}

double effectiveHundredthBpm(double beatLength, mixxx::audio::SampleRate sampleRate) {
    return std::round(60 * sampleRate / beatLength * 100);
}

// static
mixxx::BeatsPointer Beats::fromBeatPositions(
        mixxx::audio::SampleRate sampleRate,
        const QVector<audio::FramePos>& beatPositions,
        const QString& subVersion) {
    // The beat map contains the list of beats contains between the first and
    // the last beat with custom property (offset, BPM bar length) For example,
    // you may have the following as input:
    //
    // Track: |--------------------------------------------------------|
    //        |   |   |   |   |   ||  |  |  ||  |  |  ||   |   |   |   ||
    // This is the result in markers:
    // Track: |--------------------------------------------------------|
    //                            |                   |
    //                        Marker with            Marker with
    //                       faster BPM and         same BPM than track
    //                       bar count of 3      start and bar count of 4
    VERIFY_OR_DEBUG_ASSERT(sampleRate.isValid() && beatPositions.size() >= 2) {
        return nullptr;
    }

    std::vector<BeatMarker> markers;
    auto markerPosition = beatPositions.front();
    VERIFY_OR_DEBUG_ASSERT(markerPosition.isValid()) {
        return nullptr;
    }

    auto previousPosition = markerPosition;
    audio::FrameDiff_t previousBeatLengthFrames = 0;
    mixxx::audio::BeatsPerBar beatsPerBar = kDefaultBeatsPerBar;
    bool isBeatCounting = isDoubleBeat(beatPositions, 1);
    for (int i = isBeatCounting ? 2 : 1; i < beatPositions.size(); i++) {
        VERIFY_OR_DEBUG_ASSERT(beatPositions[i].isValid() &&
                beatPositions[i] >= beatPositions[i - 1]) {
            return nullptr;
        }
        auto position = beatPositions[i];
        // If the current beat is a "closing" double beat
        if (isDoubleBeat(beatPositions, i) && isBeatCounting) {
            isBeatCounting = false;
            beatsPerBar = static_cast<mixxx::audio::BeatsPerBar>(
                    std::lround((std::floor(position - markerPosition) /
                            previousBeatLengthFrames)));
            if (!beatsPerBar.isValid()) {
                qWarning() << "Invalid bar definition found in the BeatMap:"
                           << beatsPerBar << "- Ignoring";
                beatsPerBar = kDefaultBeatsPerBar;
            }
            continue;
        }
        audio::FrameDiff_t beatLengthFrames = position - beatPositions[i - 1];

        if (previousBeatLengthFrames == 0) {
            // If we are just starting over a new beat grid, we need to infer
            // the beat length from then
            DEBUG_ASSERT(beatLengthFrames);
            previousBeatLengthFrames = beatLengthFrames;
        }
        if (
                // If the BPM difference is less than 0.01 BPM...
                std::fabs(effectiveHundredthBpm(
                                  previousBeatLengthFrames, sampleRate) -
                        effectiveHundredthBpm(beatLengthFrames, sampleRate)) <
                        1.0 &&
                // ...and we are not at an "opening" double beat
                (isBeatCounting || !isDoubleBeat(beatPositions, i))) {
            //.. we assume the current beat is part of the same marker
            previousPosition = position;
            continue;
        }

        DEBUG_ASSERT(markerPosition.isValid());
        DEBUG_ASSERT(previousBeatLengthFrames > 0);
        markers.push_back(BeatMarker(markerPosition.toLowerFrameBoundary(),
                previousBeatLengthFrames,
                beatsPerBar));
        isBeatCounting = isDoubleBeat(beatPositions, i);
        markerPosition = previousPosition;
        previousPosition = position;
        if (i < beatPositions.size() - 1) {
            previousBeatLengthFrames = 0;
        }
    }

    // Special case: Because we want to be able to serialize to the old beatmap
    // format, we need to save the last beat position separately. Otherwise, we
    // can not reconstruct the last beats in the beatmap because we don't know
    // the length of the track.
    //
    // This only applies if the beat positions cannot be represented by a
    // (constant) beatgrid and require a beatmap instead. In that case, we have
    // to insert an additional marker.
    //
    // If the last beat is a double beat, this means that extra beats have been
    // generated in the beatmap to help storing the bar length of the last
    // marker and thus we don't need to store this position as this will
    // automatically be generated at serialisation, and the opening double beat,
    // where the relevant marker is has already been store in the main loop
    // above Here is an illustration of the that usecase
    //
    //                             track end
    //    ...--------------------------|
    //        |   |   |   |   |   ||   |   |   ||
    //       beat               Marker      Double beat
    // from marker n-1        with dbeat     end marker
    //                            |
    //     last relevant marker --|
    if (!markers.empty() && !isDoubleBeat(beatPositions, beatPositions.size() - 1)) {
        markers.push_back(BeatMarker(markerPosition.toLowerFrameBoundary(),
                previousBeatLengthFrames,
                beatsPerBar));
        markerPosition = beatPositions.back();
    }

    auto bpm = Bpm(60.0 * sampleRate / previousBeatLengthFrames);
    DEBUG_ASSERT(markerPosition.isValid());
    DEBUG_ASSERT(bpm.isValid());
    return BeatsPointer(new Beats(std::move(markers),
            markerPosition.toLowerFrameBoundary(),
            bpm,
            beatsPerBar,
            sampleRate,
            subVersion));
}

// static
mixxx::BeatsPointer Beats::fromBeatMarkers(
        audio::SampleRate sampleRate,
        const std::vector<BeatMarker>& markers,
        const audio::FramePos lastMarkerPosition,
        int lastMarkerBeatsPerBar,
        const Bpm lastMarkerBpm,
        const QString& subVersion) {
    return BeatsPointer(new Beats(markers,
            lastMarkerPosition,
            lastMarkerBpm,
            lastMarkerBeatsPerBar,
            sampleRate,
            subVersion));
}

// static
mixxx::BeatsPointer Beats::fromByteArray(
        mixxx::audio::SampleRate sampleRate,
        const QString& beatsVersion,
        const QString& beatsSubVersion,
        const QByteArray& byteArray) {
    VERIFY_OR_DEBUG_ASSERT(!beatsVersion.isEmpty()) {
        return nullptr;
    }
    mixxx::BeatsPointer pBeats = nullptr;
    if (beatsVersion == BEAT_GRID_1_VERSION || beatsVersion == BEAT_GRID_2_VERSION) {
        pBeats = fromBeatGridByteArray(sampleRate, beatsSubVersion, byteArray);
    } else if (beatsVersion == BEAT_MAP_VERSION) {
        pBeats = fromBeatMapByteArray(sampleRate, beatsSubVersion, byteArray);
    } else {
        qWarning().nospace() << "Failed to deserialize Beats (" << beatsVersion
                             << "): Invalid beats version";
        return nullptr;
    }

    if (!pBeats) {
        qWarning().nospace() << "Failed to deserialize Beats (" << beatsVersion
                             << "): Parsing failed";
        return nullptr;
    }

    qDebug().nospace() << "Successfully deserialized Beats (" << beatsVersion << ")";
    return pBeats;
}

// static
mixxx::BeatsPointer Beats::fromBeatGridByteArray(
        mixxx::audio::SampleRate sampleRate,
        const QString& subVersion,
        const QByteArray& byteArray) {
    VERIFY_OR_DEBUG_ASSERT(sampleRate.isValid()) {
        return nullptr;
    }

    track::io::BeatGrid grid;
    audio::FramePos position;
    Bpm bpm;
    if (grid.ParseFromArray(byteArray.constData(), byteArray.size())) {
        position = audio::FramePos(grid.first_beat().frame_position());
        bpm = Bpm(grid.bpm().bpm());
    } else if (byteArray.size() == sizeof(BeatGridV1Data)) {
        // Legacy fallback for BeatGrid-1.0
        const auto* pBlob = reinterpret_cast<const BeatGridV1Data*>(byteArray.constData());
        position = mixxx::audio::FramePos(pBlob->firstBeat);
        bpm = mixxx::Bpm(pBlob->bpm);
    }

    if (position.isValid() && bpm.isValid()) {
        return fromConstTempo(sampleRate, position, bpm, subVersion);
    }

    // Failed to parse the beatgrid.
    return nullptr;
}

// static
BeatsPointer Beats::fromBeatMapByteArray(
        audio::SampleRate sampleRate,
        const QString& subVersion,
        const QByteArray& byteArray) {
    VERIFY_OR_DEBUG_ASSERT(sampleRate.isValid()) {
        return nullptr;
    }

    track::io::BeatMap map;
    if (!map.ParseFromArray(byteArray.constData(), byteArray.size())) {
        return nullptr;
    }

    QVector<audio::FramePos> beatPositions;
    beatPositions.reserve(map.beat_size());
    for (int i = 0; i < map.beat_size(); i++) {
        beatPositions.append(audio::FramePos(map.beat(i).frame_position()));
    }

    if (beatPositions.size() < 2) {
        qWarning() << "Failed to deserialize Beats: BeatMap contains only"
                   << beatPositions.size() << "beat(s), at least 2 are needed";
        return nullptr;
    }

    return fromBeatPositions(sampleRate, beatPositions, subVersion);
}

QByteArray Beats::toByteArray() const {
    if (hasConstantTempo()) {
        return toBeatGridByteArray();
    }

    return toBeatMapByteArray();
};

QByteArray Beats::toBeatGridByteArray() const {
    DEBUG_ASSERT(hasConstantTempo());

    mixxx::track::io::BeatGrid grid;
    grid.mutable_first_beat()->set_frame_position(
            static_cast<google::protobuf::int32>(
                    m_lastMarkerPosition.toLowerFrameBoundary().value()));
    grid.mutable_bpm()->set_bpm(m_lastMarkerBpm.value());

    std::string output;
    grid.SerializeToString(&output);
    return QByteArray(output.data(), static_cast<int>(output.length()));
};

QByteArray Beats::toBeatMapByteArray() const {
    mixxx::track::io::BeatMap map;
    mixxx::audio::FramePos closingDoubleBeat;
    int currentBeatsPerBar = clastmarker().beatsPerBar();
    mixxx::Beats::ConstIterator it = cfirstmarker();
    for (; it < clastmarker() + 1 || closingDoubleBeat.isValid(); it++) {
        const auto position = (*it).toLowerFrameBoundary();
        // qDebug() << "Beat at" << position << "with bar of" << it.beatsPerBar();
        track::io::Beat beat;
        beat.set_frame_position(static_cast<google::protobuf::int32>(position.value()));
        map.add_beat()->CopyFrom(beat);
        if (it.isMarker() && it.beatsPerBar() != currentBeatsPerBar) {
            // This beat is a marker and the beat per bar is changing, add a double beat to mark it
            currentBeatsPerBar = it.beatsPerBar();
            closingDoubleBeat =
                    (position + (it.beatLengthFrames() * currentBeatsPerBar))
                            .toLowerFrameBoundary();
        } else if (closingDoubleBeat.isValid() && position >= closingDoubleBeat) {
            // Closing double beat should be strictly equal to the current beat
            // position, otherwise something is going wrong.
            qDebug() << "Closing double beat for bar of size"
                     << currentBeatsPerBar << "at" << closingDoubleBeat;
            DEBUG_ASSERT(position == closingDoubleBeat);
            closingDoubleBeat = {};
        } else {
            // Otherwise, we are going, no need to duplicate the current beat
            continue;
        }
        map.add_beat()->CopyFrom(beat);
    }
    std::string output;
    map.SerializeToString(&output);
    return QByteArray(output.data(), static_cast<int>(output.length()));
};

QString Beats::getVersion() const {
    if (hasConstantTempo()) {
        return BEAT_GRID_2_VERSION;
    } else {
        return BEAT_MAP_VERSION;
    }
};

bool Beats::findPrevNextBeats(audio::FramePos position,
        audio::FramePos* prevBeatPosition,
        audio::FramePos* nextBeatPosition,
        bool snapToNearBeats) const {
    auto it = iteratorFrom(position);
    if (it == cend()) {
        *prevBeatPosition = *it;
        *nextBeatPosition = audio::kInvalidFramePos;
        return false;
    } else if (it == cbegin()) {
        *prevBeatPosition = audio::kInvalidFramePos;
        *nextBeatPosition = *it;
        return false;
    }

    if (*it == position) {
        *prevBeatPosition = *it;
        it++;
        *nextBeatPosition = *it;
        return true;
    }

    DEBUG_ASSERT(*it > position);
    if (snapToNearBeats) {
        const audio::FrameDiff_t beatLengthFrames = *it - *std::prev(it);
        const double beatFraction = (*it - position) / beatLengthFrames;
        DEBUG_ASSERT(beatFraction > 0 && beatFraction < 1);
        if (beatFraction < kEpsilon) {
            *prevBeatPosition = *it;
            it++;
            *nextBeatPosition = *it;
            return true;
        }
    }

    *nextBeatPosition = *it;
    it--;
    *prevBeatPosition = *it;
    return true;
}

// Find the next beat at or after the position
Beats::ConstIterator Beats::iteratorFrom(audio::FramePos position) const {
    DEBUG_ASSERT(isValid());
    auto it = cfirstmarker();

    auto previousIfNeeded = [](Beats::ConstIterator it, audio::FramePos position) {
        // In positive direction the minimum step width of a double increases.
        // This may lead to tiny floating point errors that make `std::ceil`
        // round up and makes us end up one beat too late.
        //
        // Likewise, in negative direction, we can also end up one beat to
        // late if position is very close to a beat.
        //
        // This works around that issue by going back to the previous
        // beat if necessary.
        auto previousBeatIt = it - 1;
        return (*previousBeatIt >= position) ? previousBeatIt : it;
    };

    audio::FrameDiff_t diff = position - m_lastMarkerPosition;
    if (diff > 0) {
        DEBUG_ASSERT(*clastmarker() == m_lastMarkerPosition);
        // Lookup position is after the last marker position
        const double n = std::ceil(diff / lastBeatLengthFrames());
        if (n >= static_cast<double>(std::numeric_limits<int>::max())) {
            return cend();
        }
        it = clastmarker() + static_cast<int>(n);
        it = previousIfNeeded(it, position);
    } else if (position < *it) {
        // Lookup position is before the first marker position
        const double n = std::floor((*it - position) / firstBeatLengthFrames());
        if (n > static_cast<double>(std::numeric_limits<int>::max())) {
            return cbegin();
        }
        it -= static_cast<int>(n);
        it = previousIfNeeded(it, position);
    } else {
        it = std::lower_bound(cfirstmarker(), clastmarker() + 1, position);
    }
    // Due to precision error, it happens that `it` and `position` are equal,
    // but `it` is slightly behind `position` so we use epsilon to prevent false alarm
    DEBUG_ASSERT(it == cbegin() || it == cend() || *it > position ||
            std::fabs(*it - position) < kEpsilon);
    DEBUG_ASSERT(it == cbegin() || it == cend() || *it > *std::prev(it));
    return it;
}

Beats::ConstIterator Beats::iteratorClosestTo(audio::FramePos position) const {
    auto it = iteratorFrom(position);
    if (it == cbegin()) {
        return it;
    }

    const auto deltaFrames = *it - position;
    it--;
    if ((position - *it) < deltaFrames) {
        return it;
    }

    it++;
    return it;
}

audio::FramePos Beats::findNthBeat(audio::FramePos position, int n) const {
    if (n == 0) {
        return audio::kInvalidFramePos;
    }

    auto it = iteratorFrom(position);
    const bool searchForward = n > 0;
    if (searchForward) {
        n--;
    } else if (*it == position) {
        n++;
    }

    if (n != 0) {
        it += n;
    }

    return *it;
}

audio::FramePos Beats::findNBeatsFromPosition(audio::FramePos position, double beats) const {
    if (beats == 0) {
        return position;
    }

    auto it = iteratorFrom(position);
    if (*it != position) {
        DEBUG_ASSERT(*it > position);
        const auto prevBeat = std::prev(it);
        beats -= (*it - position) / prevBeat.beatLengthFrames();
    }

    double fullBeats;
    const double fractionBeats = std::modf(beats, &fullBeats);

    const auto n = static_cast<int>(fullBeats);
    DEBUG_ASSERT(n == fullBeats);
    std::advance(it, n);

    const audio::FramePos basePosition = *it;

    // If we go backwards, we need to add the fraction of the beat
    if (fractionBeats < 0) {
        it--;
    }

    return basePosition + it.beatLengthFrames() * fractionBeats;
}

bool Beats::hasBeatInRange(audio::FramePos startPosition, audio::FramePos endPosition) const {
    const audio::FramePos nextBeatPosition = findNextBeat(startPosition);
    return (nextBeatPosition <= endPosition);
}

mixxx::Bpm Beats::getBpmInRange(audio::FramePos startPosition, audio::FramePos endPosition) const {
    // TODO: Make this a debug assertion and investigate why this is triggered.
    if (!startPosition.isValid() || !endPosition.isValid() || startPosition >= endPosition) {
        return {};
    }

    if (m_markers.empty() || startPosition >= m_lastMarkerPosition) {
        return m_lastMarkerBpm;
    }

    std::unordered_map<int, audio::FrameDiff_t> map;

    auto markerIt = m_markers.crbegin();
    auto nextMarkerPosition = m_lastMarkerPosition;

    if (endPosition > m_lastMarkerPosition) {
        DEBUG_ASSERT(startPosition < m_lastMarkerPosition);
        const auto key = static_cast<int>(std::round(m_lastMarkerBpm.value() * 100));
        map.emplace(key, endPosition - m_lastMarkerPosition);
    }

    while (markerIt != m_markers.crend() && nextMarkerPosition > startPosition) {
        if (endPosition <= markerIt->position()) {
            nextMarkerPosition = markerIt->position();
            markerIt++;
            continue;
        }

        audio::FrameDiff_t sectionLengthFrames = nextMarkerPosition - markerIt->position();
        // const audio::FrameDiff_t beatLengthFrames =
        //         sectionLengthFrames * markerIt->beatsLength();

        // To mitigate rounding issues, we save (100 * bpm) as integer, so
        // that we can later return the BPM with two digits after the decimal
        // point. This suffices for our use case.
        const auto key = static_cast<int>(std::round(
                100 * 60.0 * m_sampleRate / markerIt->beatsLength()));

        if (endPosition > markerIt->position() && endPosition < nextMarkerPosition) {
            sectionLengthFrames -= nextMarkerPosition - endPosition;
        }
        if (startPosition > markerIt->position() && startPosition < nextMarkerPosition) {
            sectionLengthFrames -= startPosition - markerIt->position();
        }

        const auto found = map.find(key);
        const auto value = (found != map.cend()) ? (*found).second : 0;
        nextMarkerPosition = markerIt->position();
        markerIt++;

        if (markerIt == m_markers.crend() && nextMarkerPosition > startPosition) {
            sectionLengthFrames += nextMarkerPosition - startPosition;
        }

        map.insert_or_assign(key, value + sectionLengthFrames);
    }

    // Find the BPM that has the longest duration in our map.
    const auto maxBpmDuration = std::max_element(map.cbegin(),
            map.cend(),
            [](const std::pair<int, audio::FrameDiff_t>& lhs,
                    const std::pair<int, audio::FrameDiff_t>& rhs) {
                return (lhs.second < rhs.second);
            });
    return Bpm(static_cast<double>(maxBpmDuration->first) / 100);
}

mixxx::Bpm Beats::getBpmAroundPosition(audio::FramePos position, int n) const {
    if (m_markers.empty()) {
        return m_lastMarkerBpm;
    }

    auto it = iteratorFrom(position);

    // To make sure we are always counting n beats, iterate backward to the
    // lower bound, then iterate forward from there to the upper bound.
    it -= n;
    audio::FramePos startPosition = *it;
    it += 2 * n;
    audio::FramePos endPosition = *it;

    // If we hit the end of the beat map, recalculate the lower bound.
    if (it == cend()) {
        it -= 2 * n;
        startPosition = *it;
    }

    return BeatUtils::calculateAverageBpm(2 * n, m_sampleRate, startPosition, endPosition);
}

std::optional<BeatsPointer> Beats::tryTranslate(
        audio::FrameDiff_t offsetFrames, audio::FramePos currentFrame) const {
    std::vector<BeatMarker> markers;
    auto lastMarkerPosition = m_lastMarkerPosition;

    // If there is no valid current frame, it means we are attempting to
    // translate the whole grid, including all markers on the track
    if (!currentFrame.isValid()) {
        std::transform(m_markers.cbegin(),
                m_markers.cend(),
                std::back_inserter(markers),
                [offsetFrames](const BeatMarker& marker) -> BeatMarker {
                    const auto translatedPosition = marker.position() + offsetFrames;
                    return BeatMarker(translatedPosition.toLowerFrameBoundary(),
                            marker.beatsLength(),
                            marker.beatsPerBar());
                });
        lastMarkerPosition += offsetFrames;
    } else if (m_markers.empty() || currentFrame > lastMarkerPosition) {
        lastMarkerPosition += offsetFrames;
    } else {
        // Otherwise, this is a localise translation and we need to only
        // translate the marker directly before the current frame...
        markers = m_markers;
        auto markerIt = std::lower_bound(markers.begin(),
                markers.end(),
                currentFrame,
                isBeatMarkerLessThanFramePos);

        if (markerIt != markers.begin()) {
            // except if the current frame is before the current frame. In this
            // case, since the grid position and BPM is inferred from that first
            // marker, we translate this one, otherwise we need to rewind to the
            // previous marker as markerIt points to the marker directly after
            // our position
            markerIt--;
        }
        const auto marker = *markerIt;
        const auto translatedPosition = marker.position() + offsetFrames;
        markerIt = markers.erase(markerIt);
        markerIt = markers.emplace(markerIt,
                translatedPosition.toLowerFrameBoundary(),
                marker.beatsLength(),
                marker.beatsPerBar());
    }

    return BeatsPointer(new Beats(markers,
            lastMarkerPosition.toLowerFrameBoundary(),
            m_lastMarkerBpm,
            m_lastBeatsPerBar,
            m_sampleRate,
            m_subVersion));
}

std::optional<BeatsPointer> Beats::tryScale(BpmScale scale) const {
    double scaleFactor = 1.0;
    switch (scale) {
    case BpmScale::Double:
        scaleFactor = 2.0;
        break;
    case BpmScale::Halve:
        scaleFactor = 0.5;
        break;
    case BpmScale::TwoThirds:
        scaleFactor *= 2.0 / 3;
        break;
    case BpmScale::ThreeFourths:
        scaleFactor *= 3.0 / 4;
        break;
    case BpmScale::FourThirds:
        scaleFactor *= 4.0 / 3;
        break;
    case BpmScale::ThreeHalves:
        scaleFactor *= 3.0 / 2;
        break;
    default:
        DEBUG_ASSERT(!"scale value invalid");
        return nullptr;
    }

    std::vector<BeatMarker> markers;
    markers.reserve(m_markers.size());
    for (const auto& marker : std::as_const(m_markers)) {
        const double beatsLength = marker.beatsLength() / scaleFactor;
        if (beatsLength <= 0) {
            qWarning() << "Marker with a beat length of" << marker.beatsLength()
                       << "cannot be scaled by"
                       << scaleFactor;
            return std::nullopt;
        }

        markers.push_back({marker.position(), beatsLength, marker.beatsPerBar()});
    }

    Bpm lastMarkerBpm = m_lastMarkerBpm * scaleFactor;

    return BeatsPointer(new Beats(markers,
            m_lastMarkerPosition,
            lastMarkerBpm,
            m_lastBeatsPerBar,
            m_sampleRate,
            m_subVersion));
}

std::optional<BeatsPointer> Beats::tryAdjustTempo(
        audio::FramePos position, TempoAdjustment adjustment) const {
    auto markers = m_markers;

    // Retrieve the Beat Marker before the current position.
    //
    // Hence, we first get the marker *after* the position, then decrement the
    // iterator.
    // If there is no marker before the current position, the marker after the
    // position can be used because the beats before the first marker are
    // interpolated using the same beat length as the beats after the first
    // marker.
    auto markerIt = std::upper_bound(markers.begin(),
            markers.end(),
            position,
            isFramePosLessThanBeatMarker);
    if (markerIt != markers.begin() && position < m_lastMarkerPosition) {
        markerIt--;
    }

    // At this point, `markerIt` points to the beat marker that we need to
    // modify (or `cend()`, which means that we need to modify the BPM of the
    // last marker).
    Bpm lastMarkerBpm = m_lastMarkerBpm;
    double adjustmentFactor = static_cast<int>(adjustment) & 0b100 ? kBpmLargeAdjustModifier : 1;
    adjustment = static_cast<TempoAdjustment>(static_cast<int>(adjustment) & 0b11);
    if (markerIt == markers.end()) {
        lastMarkerBpm +=
                ((adjustment == TempoAdjustment::Faster) ? kBpmAdjustStep
                                                         : -kBpmAdjustStep) *
                adjustmentFactor;
        if (!lastMarkerBpm.isValid()) {
            qWarning() << "Beats: Tempo adjustment would result in invalid Bpm!";
            return std::nullopt;
        }
    } else {
        const auto marker = *markerIt;
        double currentMarkerBpm = (60.0 * m_sampleRate) / marker.beatsLength();
        currentMarkerBpm += ((adjustment == TempoAdjustment::Faster)
                                            ? kBpmAdjustStep
                                            : -kBpmAdjustStep) *
                adjustmentFactor;
        const double adjustedBeatLength = (60.0 * m_sampleRate) / (currentMarkerBpm);

        if (adjustedBeatLength <= 0) {
            qWarning() << "Beats: Tempo adjustment would result in a marker "
                          "with less than the minimum beats in one bar!";
            return std::nullopt;
        }

        markerIt = markers.erase(markerIt);
        markerIt = markers.emplace(markerIt,
                marker.position(),
                adjustedBeatLength,
                marker.beatsPerBar());
    }

    return fromBeatMarkers(m_sampleRate,
            std::move(markers),
            m_lastMarkerPosition,
            m_lastBeatsPerBar,
            lastMarkerBpm);
}

std::optional<BeatsPointer> Beats::trySetMarker(audio::FramePos position) const {
    auto markers = m_markers;
    auto markerIt = std::lower_bound(markers.begin(),
            markers.end(),
            position,
            isBeatMarkerLessThanFramePos);

    DEBUG_ASSERT(!m_lastMarkerPosition.isFractional());
    auto lastMarkerPosition = m_lastMarkerPosition;
    auto lastMarkerBeatsPerBar = m_lastBeatsPerBar;
    if (markerIt == markers.end()) {
        const int numBeats = calculateBeatsTillNextMarker(
                iteratorClosestTo(position), lastMarkerPosition);
        if (numBeats == 0) {
            // The new marker would be too close to the existing marker, so we
            // just move the existing marker instead.
            lastMarkerPosition = position.toLowerFrameBoundary();
        } else if (lastMarkerPosition < position) {
            // We are behind the last marker. We move the current last
            // marker into the list of marker and use the current position
            // as the new last marker.
            markers.push_back(mixxx::BeatMarker(lastMarkerPosition,
                    lastBeatLengthFrames(),
                    lastMarkerBeatsPerBar));
            lastMarkerPosition = position.toLowerFrameBoundary();
        } else {
            // We are between the last beat marker of additional list and the single last marker,
            // so we can just append a new beat marker to the additional list.
            markers.push_back(mixxx::BeatMarker(position.toLowerFrameBoundary(),
                    lastBeatLengthFrames(),
                    lastMarkerBeatsPerBar));
        }
    } else if (markerIt == markers.begin()) {
        // We are in front of the first regular beat marker, so we can just
        // prepend a new beat marker using the same BPM (beat length).
        const auto marker = *markerIt;
        markers.emplace(markerIt,
                position.toLowerFrameBoundary(),
                marker.beatsLength(),
                marker.beatsPerBar());
    } else {
        // We found the beat marker right after the position we want to create a
        // marker, so rewind the iterator to find the previous marker and use
        // the same BPM (beat length)
        if (markerIt->position() == position) {
            // TODO if `[BPM],stretch_on_marker_duplicate`, call tryStretchBeatGrid
            return std::nullopt;
        }
        markerIt--;
        const auto previousMarker = *markerIt;
        markers.emplace(markerIt + 1,
                position.toLowerFrameBoundary(),
                previousMarker.beatsLength(),
                previousMarker.beatsPerBar());
    }

    return fromBeatMarkers(m_sampleRate,
            std::move(markers),
            lastMarkerPosition,
            m_lastBeatsPerBar,
            m_lastMarkerBpm);
}

std::optional<BeatsPointer> Beats::tryAdjustBeatsPerBar(
        audio::FramePos position, std::int32_t adjustment) const {
    auto markers = m_markers;

    auto markerIt = std::upper_bound(markers.begin(),
            markers.end(),
            position,
            isFramePosLessThanBeatMarker);

    if (markerIt != markers.begin() && position < m_lastMarkerPosition) {
        markerIt--;
    }

    int lastBeatsPerBar = m_lastBeatsPerBar;

    if (markerIt == markers.end()) {
        lastBeatsPerBar += adjustment;

        if (lastBeatsPerBar < 2 || lastBeatsPerBar > 32) {
            qWarning() << "Beats: beat per bar cannot be less than 2 and more than 32!";
            return std::nullopt;
        }
    } else {
        const auto marker = *markerIt;
        const auto adjustedBeatsPerBar = marker.beatsPerBar() + adjustment;

        if (!adjustedBeatsPerBar.isValid()) {
            qWarning() << "Beats: beat per bar cannot be less than 2 and more than 32!";
            return std::nullopt;
        }

        markerIt = markers.erase(markerIt);
        markerIt = markers.emplace(markerIt,
                marker.position(),
                marker.beatsLength(),
                adjustedBeatsPerBar);
    }

    return fromBeatMarkers(m_sampleRate,
            std::move(markers),
            m_lastMarkerPosition,
            lastBeatsPerBar,
            m_lastMarkerBpm);
}

std::optional<BeatsPointer> Beats::tryStretchBeatGrid(
        audio::FramePos position) const {
    Q_UNUSED(position);
    // TODO implement beat stretching
    return std::nullopt;
}

std::optional<BeatsPointer> Beats::tryRemoveMarker(audio::FramePos position) const {
    if (m_markers.empty()) {
        // There are no markers beside the mandatory last marker, so there is
        // nothing to remove.
        return std::nullopt;
    }

    auto it = iteratorClosestTo(position);
    if (!it.isMarker()) {
        // The position is not near a marker, don't remove anything.
        return std::nullopt;
    }
    DEBUG_ASSERT(it != cbegin());

    auto lastMarkerBpm = m_lastMarkerBpm;
    auto lastMarkerPosition = m_lastMarkerPosition;
    auto lastMarkerBeatsPerBar = m_lastBeatsPerBar;
    auto markers = m_markers;
    if (it == clastmarker()) {
        // We are near the last marker. It's not possible to actually remove
        // it, so we remove the marker *before* the last marker instead, and
        // use it's BPM and position values for the last marker. The end result
        // is the same.
        it--;
        lastMarkerBpm = mixxx::Bpm(60.0 * m_sampleRate / it.beatLengthFrames());
        lastMarkerPosition = markers.back().position();
        lastMarkerBeatsPerBar = markers.back().beatsPerBar();
        markers.pop_back();
    } else {
        auto markerIt = std::lower_bound(markers.begin(),
                markers.end(),
                findClosestBeat(position),
                isBeatMarkerLessThanFramePos);
        // At this point we already checked that the search position is on a
        // marker and that it is not at the last marker. This means that
        // `markerIt` should point to a marker in any case.
        VERIFY_OR_DEBUG_ASSERT(markerIt != markers.end()) {
            return std::nullopt;
        }

        markerIt = markers.erase(markerIt);
    }

    return fromBeatMarkers(m_sampleRate,
            std::move(markers),
            lastMarkerPosition,
            lastMarkerBeatsPerBar,
            lastMarkerBpm);
}

std::optional<BeatsPointer> Beats::trySetBpm(mixxx::Bpm bpm) const {
    const auto it = cfirstmarker();
    return BeatsPointer(new Beats({}, *it, bpm, m_lastBeatsPerBar, m_sampleRate, m_subVersion));
}

bool Beats::isValid() const {
    if (!m_lastMarkerPosition.isValid() || !m_lastMarkerBpm.isValid()) {
        return false;
    }

    for (const BeatMarker& marker : m_markers) {
        if (!marker.position().isValid() || marker.beatsLength() <= 0) {
            return false;
        }
    }

    return true;
}

mixxx::audio::FrameDiff_t Beats::firstBeatLengthFrames() const {
    const auto it = cfirstmarker();
    return it.beatLengthFrames();
}

mixxx::audio::FrameDiff_t Beats::lastBeatLengthFrames() const {
    return 60.0 * m_sampleRate / m_lastMarkerBpm.value();
}

audio::FramePos Beats::snapPosToNearBeat(audio::FramePos position) const {
    audio::FramePos prevBeatPosition;
    audio::FramePos nextBeatPosition;
    findPrevNextBeats(position, &prevBeatPosition, &nextBeatPosition, true);
    if (prevBeatPosition.isValid() && position < prevBeatPosition) {
        // Snap to beat
        return prevBeatPosition;
    } else if (nextBeatPosition.isValid() && position > nextBeatPosition) {
        // Snap to beat
        return nextBeatPosition;
    }
    return position;
}

int Beats::numBeatsInRange(audio::FramePos startPosition, audio::FramePos endPosition) const {
    startPosition = snapPosToNearBeat(startPosition);
    audio::FramePos lastPosition = audio::kStartFramePos;
    int i = 1;
    while (lastPosition < endPosition) {
        lastPosition = findNthBeat(startPosition, i);
        if (!lastPosition.isValid()) {
            break;
        }
        i++;
    }
    return i - 2;
};

audio::FramePos Beats::findNextBeat(audio::FramePos position) const {
    return findNthBeat(position, 1);
}

audio::FramePos Beats::findPrevBeat(audio::FramePos position) const {
    return findNthBeat(position, -1);
}

audio::FramePos Beats::findClosestBeat(audio::FramePos position) const {
    if (!isValid()) {
        return audio::kInvalidFramePos;
    }
    audio::FramePos prevBeatPosition;
    audio::FramePos nextBeatPosition;
    findPrevNextBeats(position, &prevBeatPosition, &nextBeatPosition, false);
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

} // namespace mixxx
