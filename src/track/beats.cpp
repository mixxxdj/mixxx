#include "track/beats.h"

#include <iterator>
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

} // namespace

namespace mixxx {

bool BeatIterator::hasNext() const {
    return *m_it <= m_endPosition;
}

audio::FramePos BeatIterator::next() {
    VERIFY_OR_DEBUG_ASSERT(hasNext()) {
        return audio::kInvalidFramePos;
    }
    const audio::FramePos position = *m_it;
    m_it++;
    return position;
}

mixxx::audio::FrameDiff_t Beats::ConstIterator::beatLengthFrames() const {
    if (m_it == m_beats->m_markers.cend()) {
        return m_beats->endBeatLengthFrames();
    }

    const auto nextMarker = std::next(m_it);
    const mixxx::audio::FramePos nextMarkerPosition =
            (nextMarker != m_beats->m_markers.cend())
            ? nextMarker->position()
            : m_beats->m_endMarkerPosition;
    return (nextMarkerPosition - m_it->position()) / m_it->beatsTillNextMarker();
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
    const int beatOffset = m_beatOffset + n;

    // Detect integer overflow
    if (beatOffset < m_beatOffset) {
        qWarning() << "Beats: Iterator would go out of possible range, capping "
                      "at latest possible position.";
        m_it = m_beats->m_markers.cend();
        m_beatOffset = std::numeric_limits<Beats::ConstIterator::difference_type>::max();
        updateValue();
        return *this;
    }

    m_beatOffset = beatOffset;
    while (m_it != m_beats->m_markers.cend() && m_beatOffset >= m_it->beatsTillNextMarker()) {
        m_beatOffset -= m_it->beatsTillNextMarker();
        m_it++;
    }
    updateValue();
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
    const int beatOffset = m_beatOffset - n;

    // Detect integer overflow
    if (beatOffset > m_beatOffset) {
        qWarning() << "Beats: Iterator would go out of possible range, capping "
                      "at earliest possible position.";
        m_it = m_beats->m_markers.cbegin();
        m_beatOffset = std::numeric_limits<Beats::ConstIterator::difference_type>::lowest();
        updateValue();
        return *this;
    }

    m_beatOffset = beatOffset;
    while (m_it != m_beats->m_markers.cbegin() && m_beatOffset < 0) {
        m_it--;
        m_beatOffset += m_it->beatsTillNextMarker();
    }
    updateValue();
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
        result += it->beatsTillNextMarker();
        it++;
    }
    DEBUG_ASSERT(it == other.m_it);
    return -result;
}

void Beats::ConstIterator::updateValue() {
    const auto position = (m_it != m_beats->m_markers.cend())
            ? m_it->position()
            : m_beats->m_endMarkerPosition;
    m_value = position + m_beatOffset * beatLengthFrames();
}

// static
mixxx::BeatsPointer Beats::fromConstTempo(
        mixxx::audio::SampleRate sampleRate,
        mixxx::audio::FramePos endMarkerPosition,
        mixxx::Bpm endMarkerBpm,
        const QString& subVersion) {
    VERIFY_OR_DEBUG_ASSERT(sampleRate.isValid() &&
            endMarkerPosition.isValid() && endMarkerBpm.isValid()) {
        return nullptr;
    }
    return BeatsPointer(new Beats({}, endMarkerPosition, endMarkerBpm, sampleRate, subVersion));
}

// static
mixxx::BeatsPointer Beats::fromBeatPositions(
        mixxx::audio::SampleRate sampleRate,
        const QVector<audio::FramePos>& beatPositions,
        const QString& subVersion) {
    VERIFY_OR_DEBUG_ASSERT(sampleRate.isValid() && beatPositions.size() >= 2) {
        return nullptr;
    }

    std::vector<BeatMarker> markers;
    auto markerPosition = beatPositions.front();
    VERIFY_OR_DEBUG_ASSERT(markerPosition.isValid()) {
        return nullptr;
    }
    int beatsTillNextMarker = 0;

    auto previousPosition = markerPosition;
    audio::FrameDiff_t previousBeatLengthFrames = beatPositions[1] - previousPosition;
    for (int i = 1; i < beatPositions.size(); i++) {
        VERIFY_OR_DEBUG_ASSERT(beatPositions[i].isValid() &&
                beatPositions[i] > beatPositions[i - 1]) {
            return nullptr;
        }
        auto position = beatPositions[i];
        audio::FrameDiff_t beatLengthFrames = position - previousPosition;

        if (std::fabs(previousBeatLengthFrames - beatLengthFrames) < 1.0) {
            previousPosition = position;
            beatsTillNextMarker++;
            continue;
        }

        DEBUG_ASSERT(markerPosition.isValid());
        DEBUG_ASSERT(beatsTillNextMarker > 0);
        markers.push_back(BeatMarker(markerPosition.toLowerFrameBoundary(), beatsTillNextMarker));
        markerPosition = previousPosition;
        previousBeatLengthFrames = beatLengthFrames;
        beatsTillNextMarker = 1;
        previousPosition = position;
    }

    // Special case: Because we want to be able to serialize to the old beatmap
    // format, we need to save the last beat position separately. Otherwise, we
    // can not reconstruct the last beats in the beatmap because we don't know
    // the length of the track.
    //
    // Hence, we insert an additional marker here.
    markers.push_back(BeatMarker(markerPosition.toLowerFrameBoundary(), beatsTillNextMarker));
    markerPosition = beatPositions.back();

    auto bpm = Bpm(60.0 * sampleRate / previousBeatLengthFrames);
    DEBUG_ASSERT(markerPosition.isValid());
    DEBUG_ASSERT(bpm.isValid());
    return BeatsPointer(new Beats(std::move(markers),
            markerPosition.toLowerFrameBoundary(),
            bpm,
            sampleRate,
            subVersion));
}

// static
mixxx::BeatsPointer Beats::fromByteArray(
        mixxx::audio::SampleRate sampleRate,
        const QString& beatsVersion,
        const QString& beatsSubVersion,
        const QByteArray& byteArray) {
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
        const auto blob = reinterpret_cast<const BeatGridV1Data*>(byteArray.constData());
        position = mixxx::audio::FramePos(blob->firstBeat);
        bpm = mixxx::Bpm(blob->bpm);
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
                    m_endMarkerPosition.toLowerFrameBoundary().value()));
    grid.mutable_bpm()->set_bpm(m_endMarkerBpm.value());

    std::string output;
    grid.SerializeToString(&output);
    return QByteArray(output.data(), static_cast<int>(output.length()));
};

QByteArray Beats::toBeatMapByteArray() const {
    mixxx::track::io::BeatMap map;
    for (auto it = cbegin(); it != cend(); it++) {
        const auto position = (*it).toLowerFrameBoundary();
        qWarning() << position;
        track::io::Beat beat;
        beat.set_frame_position(static_cast<google::protobuf::int32>(position.value()));
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
    if (it == clatest()) {
        *prevBeatPosition = *it;
        *nextBeatPosition = audio::kInvalidFramePos;
        return false;
    } else if (it == cearliest()) {
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

Beats::ConstIterator Beats::iteratorFrom(audio::FramePos position) const {
    DEBUG_ASSERT(isValid());
    auto it = cbegin();
    if (position > m_endMarkerPosition) {
        DEBUG_ASSERT(*(cend() - 1) == m_endMarkerPosition);
        // Lookup position is after the last marker position
        const double n = std::ceil((position - m_endMarkerPosition) / endBeatLengthFrames());
        if (n >= static_cast<double>(std::numeric_limits<int>::max())) {
            return clatest();
        }
        it = (cend() - 1) + static_cast<int>(n);

        // In some rare cases there may be extremely tiny floating point errors
        // that make `std::ceil` round up and makes us end up one beat too
        // late. This works around that issue by going back to the previous
        // beat if necessary.
        auto previousBeatIt = it - 1;
        if (*previousBeatIt >= position) {
            it = previousBeatIt;
        }
    } else if (position < *it) {
        // Lookup position is before the first marker position
        const double n = std::floor((*it - position) / firstBeatLengthFrames());
        if (n > static_cast<double>(std::numeric_limits<int>::max())) {
            return cearliest();
        }
        it -= static_cast<int>(n);
    } else {
        it = std::lower_bound(cbegin(), cend(), position);
    }
    DEBUG_ASSERT(it == cearliest() || it == clatest() || *it >= position);
    DEBUG_ASSERT(it == cearliest() || it == clatest() ||
            *it - position < std::prev(it).beatLengthFrames());
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

std::unique_ptr<BeatIterator> Beats::findBeats(
        audio::FramePos startPosition,
        audio::FramePos endPosition) const {
    auto it = iteratorFrom(startPosition);
    return std::make_unique<BeatIterator>(std::move(it), endPosition);
}

bool Beats::hasBeatInRange(audio::FramePos startPosition, audio::FramePos endPosition) const {
    const audio::FramePos nextBeatPosition = findNextBeat(startPosition);
    return (nextBeatPosition <= endPosition);
}

mixxx::Bpm Beats::getBpmInRange(audio::FramePos startPosition, audio::FramePos endPosition) const {
    if (m_markers.empty()) {
        return m_endMarkerBpm;
    }

    const auto start = iteratorFrom(startPosition);
    const auto end = iteratorFrom(endPosition);

    const int numBeats = std::distance(start, end);
    if (numBeats < 2) {
        return {};
    }

    return BeatUtils::calculateAverageBpm(numBeats, m_sampleRate, *start, *end);
}

mixxx::Bpm Beats::getBpmAroundPosition(audio::FramePos position, int n) const {
    if (m_markers.empty()) {
        return m_endMarkerBpm;
    }

    auto it = iteratorFrom(position);

    // To make sure we are always counting n beats, iterate backward to the
    // lower bound, then iterate forward from there to the upper bound.
    it -= n;
    audio::FramePos startPosition = *it;
    it += 2 * n;
    audio::FramePos endPosition = *it;

    // If we hit the end of the beat map, recalculate the lower bound.
    if (it == clatest()) {
        it -= 2 * n;
        startPosition = *it;
    }

    return BeatUtils::calculateAverageBpm(2 * n, m_sampleRate, startPosition, endPosition);
}

std::optional<BeatsPointer> Beats::tryTranslate(audio::FrameDiff_t offsetFrames) const {
    std::vector<BeatMarker> markers;
    std::transform(m_markers.cbegin(),
            m_markers.cend(),
            std::back_inserter(markers),
            [offsetFrames](const BeatMarker& marker) -> BeatMarker {
                return BeatMarker(marker.position() + offsetFrames,
                        marker.beatsTillNextMarker());
            });

    return BeatsPointer(new Beats(markers,
            m_endMarkerPosition + offsetFrames,
            m_endMarkerBpm,
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
        const double beatsTillNextMarkerFractional = marker.beatsTillNextMarker() * scaleFactor;
        const int beatsTillNextMarker = static_cast<int>(std::trunc(beatsTillNextMarkerFractional));
        if (beatsTillNextMarkerFractional != beatsTillNextMarker) {
            qWarning() << "Marker with" << marker.beatsTillNextMarker()
                       << "beats till next marker cannot be scaled by"
                       << scaleFactor;
            return std::nullopt;
        }

        markers.push_back({marker.position(), beatsTillNextMarker});
    }

    Bpm endMarkerBpm = m_endMarkerBpm * scaleFactor;

    return BeatsPointer(new Beats(markers,
            m_endMarkerPosition,
            endMarkerBpm,
            m_sampleRate,
            m_subVersion));
}

std::optional<BeatsPointer> Beats::trySetBpm(mixxx::Bpm bpm) const {
    const auto it = cbegin();
    return BeatsPointer(new Beats({}, *it, bpm, m_sampleRate, m_subVersion));
}

bool Beats::isValid() const {
    if (!m_endMarkerPosition.isValid() || !m_endMarkerBpm.isValid()) {
        return false;
    }

    for (const BeatMarker& marker : m_markers) {
        if (!marker.position().isValid() || marker.beatsTillNextMarker() <= 0) {
            return false;
        }
    }

    return true;
}

mixxx::audio::FrameDiff_t Beats::firstBeatLengthFrames() const {
    const auto it = cbegin();
    return it.beatLengthFrames();
}

mixxx::audio::FrameDiff_t Beats::endBeatLengthFrames() const {
    return 60.0 * m_sampleRate / m_endMarkerBpm.value();
}

int Beats::numBeatsInRange(audio::FramePos startPosition, audio::FramePos endPosition) const {
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

audio::FramePos Beats::findNBeatsFromPosition(audio::FramePos position, double beats) const {
    audio::FramePos prevBeatPosition;
    audio::FramePos nextBeatPosition;

    if (!findPrevNextBeats(position, &prevBeatPosition, &nextBeatPosition, true)) {
        return position;
    }
    const audio::FrameDiff_t fromFractionBeats = (position - prevBeatPosition) /
            (nextBeatPosition - prevBeatPosition);
    const audio::FrameDiff_t beatsFromPrevBeat = fromFractionBeats + beats;

    const int fullBeats = static_cast<int>(beatsFromPrevBeat);
    const audio::FrameDiff_t fractionBeats = beatsFromPrevBeat - fullBeats;

    // Add the length between this beat and the fullbeats'th beat
    // to the end position
    audio::FramePos nthBeatPosition;
    if (fullBeats > 0) {
        nthBeatPosition = findNthBeat(nextBeatPosition, fullBeats);
    } else {
        nthBeatPosition = findNthBeat(prevBeatPosition, fullBeats - 1);
    }

    if (!nthBeatPosition.isValid()) {
        return position;
    }

    // Add the fraction of the beat
    if (fractionBeats != 0) {
        nextBeatPosition = findNthBeat(nthBeatPosition, 2);
        if (!nextBeatPosition.isValid()) {
            return position;
        }
        nthBeatPosition += (nextBeatPosition - nthBeatPosition) * fractionBeats;
    }

    return nthBeatPosition;
};

} // namespace mixxx
